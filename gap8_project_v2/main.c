/*
 * Copyright 2020 GreenWaves Technologies, SAS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include "BleNotifier.h"
#include "Gap.h"
#include "lynredKernels.h"
#include "pmsis.h"
#include "bsp/bsp.h"
#include "bsp/gapoc_b_v2.h"
#include "bsp/camera.h"


#include "bsp/fs.h"
#include "bsp/fs/readfs.h"


#if defined(QSPI)
#define FLASH_NAME "QSPI"
#include "bsp/flash/spiflash.h"
#else
#define FLASH_NAME "HYPER"
#include "bsp/flash/hyperflash.h"
#endif

#include "bsp/camera/thermeye.h"

#include "shutterless/PreFiltering.h"
#include "shutterless/PreFilteringCluster.h"
#include "ImageDraw.h"
#include "setup.h"

#define MOUNT           1
#define UNMOUNT         0
#define CID             0

#define GPIO_USER_LED 0


#if defined(QSPI)
struct pi_device QspiRam;
AT_QSPIFLASH_FS_EXT_ADDR_TYPE lynred_L3_Flash;
#else
struct pi_device HyperRam;
AT_HYPERFLASH_FS_EXT_ADDR_TYPE lynred_L3_Flash;
#endif


#define FIX2FP(Val, Precision)    ((float) (Val) / (float) (1<<(Precision)))

L2_MEM bbox_t *out_boxes;


PI_L2 int16_t* img_offset;
PI_L2 int16_t* ImageIn;
PI_L2 unsigned char *ImageInChar;

#define BUFFER_SIZE 1024
static struct pi_device cam;
static struct pi_device cluster_dev;


extern L1_CL_MEM AT_L1_POINTER lynred_L1_Memory;


void open_flash_filesystem(struct pi_device *flash, struct pi_device *fs)
{
    struct pi_readfs_conf fsconf;
    
    /* Init & open flash. */
    #if defined(QSPI)
    struct pi_spiflash_conf flash_conf;
    pi_spiflash_conf_init(&flash_conf);
    #else
    struct pi_hyperflash_conf flash_conf;
    pi_hyperflash_conf_init(&flash_conf);
    #endif
    pi_open_from_conf(flash, &flash_conf);
    if (pi_flash_open(flash))
    {
        printf("Error flash open !\n");
        pmsis_exit(-1);
    }

    pi_readfs_conf_init(&fsconf);

    fsconf.fs.flash = flash;
    
    pi_open_from_conf(fs, &fsconf);
    
    if (pi_fs_mount(fs)){
        printf("Error FS mounting !\n");
        pmsis_exit(-2);
    }
}

void close_flash_filesystem(struct pi_device *flash, struct pi_device *fs)
{
    pi_fs_unmount(fs);
    pi_flash_close(flash);
}

#if !defined(INPUT_RAW_FILE) && !defined(INPUT_FILE)

static int32_t open_camera_thermeye(struct pi_device *device)
{
    struct pi_thermeye_conf cam_conf;
    pi_thermeye_conf_init(&cam_conf);
    pi_open_from_conf(device, &cam_conf);
    if (pi_camera_open(device))
    {
        return -1;
    }
    return 0;
}
#endif

static int initNN(){

    #ifndef INPUT_FILE
    PRINTF("Loading Offset Image from Flash...\n");
    pi_fs_file_t *file = NULL;
    char *name = "Calibration.bin";
    int32_t size = 0;
    uint32_t size_total = 0;

    img_offset  = (unsigned short int *) pmsis_l2_malloc(80 * 80 * sizeof(short int));
    char * buff =  img_offset;

    if (img_offset==NULL )
    {
        PRINTF("Failed to allocate Memory for image Offset\n");
        pmsis_exit(-4);
    }

    struct pi_device flash;
    struct pi_device fs;

    open_flash_filesystem(&flash, &fs);

    file = pi_fs_open(&fs, name, 0);
    if (file == NULL)
    {
        printf("File %s open failed !\n", name);
        pmsis_exit(-4);
    }

    do
    {
        //Read from filesystem(on flash) to a buffer in L2 memory.
        size = pi_fs_read(file, buff+size_total, BUFFER_SIZE);
        size_total += size;
    } while (size_total < file->size);

    pi_fs_close(file);

    close_flash_filesystem(&flash,&fs);

    #endif

    return 0;

}

int initL3Buffers(){

    /* Init & open ram. */
#if defined(QSPI)
    struct pi_device *ram= &QspiRam;
    static struct pi_spiram_conf conf;
    pi_spiram_conf_init(&conf);
#else
    struct pi_device *ram=&HyperRam;
    static struct pi_hyperram_conf conf;
    pi_hyperram_conf_init(&conf);
#endif
    pi_open_from_conf(ram, &conf);
    if (pi_ram_open(ram))
    {
        printf("Error ram open !\n");
        return -1;
    }


    //Allocating output bounding boxes
    out_boxes = (bbox_t *) pmsis_l2_malloc(MAX_BB*sizeof(bbox_t));
    if(out_boxes==NULL){
      printf("Error Allocating CNN output buffers");
      return 1;
    }
    
    return 0;
}


void drawBboxes(bbox_t *boundbxs, uint8_t *img, int img_w,int img_h){

     for (int counter=0;counter< MAX_BB;counter++){
        if(out_boxes[counter].alive && (out_boxes[counter].score > SCORE_THR)){
            //printf("bb %d\n",counter);
            int box_x = out_boxes[counter].x;
            int box_y = out_boxes[counter].y;
            int box_w = out_boxes[counter].w;
            int box_h = out_boxes[counter].h;
            DrawRectangle(img, 80, 80, box_x, box_y, box_w, box_h, 255);
        }
    }
}
L2_MEM short int output_1[40];
L2_MEM signed char output_2[10];
L2_MEM signed char output_3[10];


static void RunNN()
{
    unsigned int ti,ti_nn,ti_ssd;
        for(int i=0;i<10;i++){
            output_1[i]=0;
            output_2[i]=0;
        }

    gap_cl_starttimer();
    gap_cl_resethwtimer();
    ti = gap_cl_readhwtimer();

    for(int i=0;i<MAX_BB;i++) out_boxes[i].alive=0;

    lynredCNN(ImageInChar, output_1,output_2,output_3);

    ti_nn = gap_cl_readhwtimer()-ti;

    for(int i=0;i<10;i++)
    {

        if(output_2[i]==1 && output_3[i]!=0)
        {
            out_boxes[i].alive=1;
            out_boxes[i].class=output_2[i];
            out_boxes[i].score=output_3[i];
            out_boxes[i].x=(output_1[4*i+1]*lynred_Output_1_OUT_SCALE)*80;
            out_boxes[i].y=(output_1[4*i]*lynred_Output_1_OUT_SCALE)*80;
            out_boxes[i].w=((output_1[4*i+3]-output_1[4*i+1])*lynred_Output_1_OUT_SCALE)*80;
            out_boxes[i].h=((output_1[4*i+2]-output_1[4*i])*lynred_Output_1_OUT_SCALE)*80;

            if(out_boxes[i].alive && (out_boxes[i].score > SCORE_THR))
            {
                int box_x = out_boxes[i].x;
                int box_y = out_boxes[i].y;
                int box_w = out_boxes[i].w;
                int box_h = out_boxes[i].h;
                PRINTF("BBOX (x, y, w, h): (%d, %d, %d, %d) SCORE: %f\n", box_x, box_y, box_w, box_h, FIX2FP(out_boxes[i].score,7));
            }
        }
    }

    PRINTF("Cycles NN : %10d\n",ti_nn);
    
}


char bleDetString[200];
char tmpString[200];
int dt;
int old_dt;
float thres;
void sendResultsToBle(bbox_t *boundbxs){

    int stringLenght = 0;
    int AliveBBs=0;


    for (int counter=0;counter< MAX_BB;counter++){
         if(boundbxs[counter].alive && (boundbxs[counter].score > SCORE_THR)){
            AliveBBs++;
        }
    }
    if(AliveBBs>MAX_OUT_BB) AliveBBs=MAX_OUT_BB;

    stringLenght+=sprintf(bleDetString,"%d;",AliveBBs);
    AliveBBs=0;
    for (int counter=0;counter< MAX_BB;counter++){
         if(boundbxs[counter].alive && (boundbxs[counter].score > SCORE_THR) && AliveBBs<MAX_OUT_BB){

            int box_x = boundbxs[counter].x;
            int box_y = boundbxs[counter].y;
            int box_w = boundbxs[counter].w;
            int box_h = boundbxs[counter].h;
            //Ble application only uses x and y coordinate set to center of Bounding Box
            int x = box_x + (box_w/2);
            int y = box_y + (box_h/2);
            stringLenght+=sprintf(tmpString,"%dx%d;",x, y);
            strcat(bleDetString,tmpString);
            AliveBBs++;
        }
    }

    //stringLenght+=sprintf(tmpString,"Gap8 Power Consuption %f mW/FPS",((float)(1/(50000000.f/12000000)) * 16.800));
    stringLenght+=sprintf(tmpString,"A Project from GreenWaves and Lynred");
    strcat(bleDetString,tmpString);

    //printf("%s",bleDetString);
    //printf("\n");
    //printf("String Size: %d\n",stringLenght);

    dt = handleDetections(bleDetString,stringLenght);
/*    if(dt<10)dt=10;
    if(dt!=old_dt){
        old_dt=dt;
        thres = ((float)old_dt)/100;
        anchor_layer_2->confidence_thr = FP2FIX(thres,15);
        anchor_layer_3->confidence_thr = FP2FIX(thres,15);
        anchor_layer_4->confidence_thr = FP2FIX(thres,15);
        anchor_layer_5->confidence_thr = FP2FIX(thres,15);

    }*/
}

int read_raw_image(char* filename, int16_t* buffer,int w,int h){
    struct pi_fs_conf conf;
    static struct pi_device fs;
    static pi_fs_file_t *file;
    unsigned int ReadSize=0;

    pi_fs_conf_init(&conf);
    conf.type = PI_FS_HOST;
    pi_open_from_conf(&fs, &conf);

    if (pi_fs_mount(&fs))
        return -2;

    file = pi_fs_open(&fs, filename, PI_FS_FLAGS_READ);
    if (file == NULL) return -3;


    {
        char *TargetImg = buffer;
        unsigned int RemainSize = w*h*sizeof(int16_t);

        while (RemainSize > 0)
        {
            unsigned int Chunk = Min(4096, RemainSize);
            unsigned R = pi_fs_read(file,TargetImg, Chunk);
            ReadSize+=R;
            if (R!=Chunk) break;
            TargetImg += Chunk; RemainSize -= Chunk;
        }
    }

    pi_fs_close(file);
    pi_fs_unmount(&fs);

    printf("Image %s, [W: %d, H: %d], Gray, Size: %d bytes, Loaded sucessfully\n", filename, w, h, ReadSize);

    return 0;

}

/* This SLEEP only works in pulpos for now
 * TODO: need to be support in freeRTOS when new api available
 */

#ifdef SLEEP
#define RTC_TIME 5
void go_to_sleep(){

    rt_rtc_conf_t rtc_conf;
    rt_rtc_t *rtc;

    rtc_conf.clkDivider = 0xF000;
    rt_rtc_cntDwn_t cntDwn = { RTC_TIME, 0 };

    rtc = rt_rtc_open(&rtc_conf, NULL);
    if (rtc == NULL) return -1;

    rt_pm_wakeup_clear_all();

    rt_rtc_control(rtc, RTC_CNTDOWN_SET, (void *)&cntDwn, NULL);
    rt_rtc_control(rtc, RTC_CNTDOWN_START, NULL, NULL);

    if(rt_pm_state_switch(RT_PM_STATE_DEEP_SLEEP, RT_PM_STATE_FAST)==-1)
        printf("Error....\n");

}
#endif


int32_t fixed_shutterless(int16_t* img_input_fp16,int16_t* img_offset_fp16,int w, int h, uint8_t q_output){
    
    int16_t min,max;
    int16_t out_min = 0;
    int32_t out_max = 255;
    int32_t out_space = (out_max-out_min);
    uint8_t *img_input_fp8=img_input_fp16;
   
    //Optmized shutterless running on cluster (cluster must be open ahead and have enough free memory)
    int error = shutterless_fixed_cl(&cluster_dev,img_input_fp16,img_offset_fp16,40,&min,&max);
    //Calling shutterless running on fabric controller
    //int error = shutterless_fixed_fc(img_input_fp16,img_offset_fp16,40,&min,&max);

    float div = 1./(max-min);
    int32_t div_fix = FP2FIX(div ,15);

    //Normalizing to 8 bit and changing fixed point format for NN.
    for(int i=0;i<w*h;i++){
        img_input_fp8[i]= (uint8_t)(((out_space)* ((((((int32_t)img_input_fp16[i])-(int32_t)min))*div_fix)))>>(15-q_output+8));
    }

    return error;
}

int32_t float_shutterless(int16_t* img_input_fp16,int16_t* img_offset_fp16,int w, int h, uint8_t q_output, float gamma){
    int min,max;
    int32_t out_min = 0;
    int32_t out_max = 255;
    uint8_t *img_input_fp8=img_input_fp16;
    
    int error = shutterless_float(img_input_fp16,img_offset_fp16,40,&min,&max);    
        
    for(int i=0;i<w*h;i++){
        img_input_fp16[i]= (int16_t)((out_max-out_min)* (pow(((float)img_input_fp16[i]-min)/(max-min),gamma) + out_min)) ;
        img_input_fp8[i]= img_input_fp16[i] << (q_output-8);
    }
    return error;
}



#define USER_GPIO 18

void peopleDetection(void)
{
    char *ImageName = "../../../samples/im2.pgm";
    char *RawImageName = "../../../raw_samples/dump_out_imgs/im_gap_20210125-14_15_04.bin";

    //To configure and use User LED
    //pi_pad_e pad = (GPIO_USER_LED >> PI_GPIO_NUM_SHIFT);
    //uint32_t pin = (GPIO_USER_LED & PI_GPIO_NUM_MASK);
    //pi_pad_set_function(pad, PI_PAD_FUNC0);
    //pi_gpio_pin_configure(NULL, GPIO_USER_LED, PI_GPIO_OUTPUT);
    //pi_gpio_pin_write(NULL, GPIO_USER_LED, 1);

    //To configure and use CONN3 pin8 on Gapoc_b boards
    //That is connected to pad 24 gpio18 of Gap8
    //this can be use as analysis pin (i.e. connect and external osilloscope and check chip activity)
    //pi_pad_set_function(PI_PAD_32_A13_TIMER0_CH1, PI_PAD_32_A13_GPIO_A18_FUNC1);
    //pi_gpio_pin_configure(NULL, USER_GPIO, PI_GPIO_OUTPUT);
    //pi_gpio_pin_write(NULL, USER_GPIO, 1);

    unsigned int Wi, Hi;
    //Input image size
    unsigned int W = 80, H = 80;
    unsigned int save_index=0;
    PRINTF("Entering main controller\n");

    
    ImageInChar = (unsigned char *) pmsis_l2_malloc( W * H* sizeof(short int));
    if (ImageInChar == 0)
    {
        PRINTF("Failed to allocate Memory for Image (%d bytes)\n", W * H * sizeof(int16_t));
        return 1;
    }
    ImageIn = (int16_t *)ImageInChar;

    #ifdef INPUT_FILE
    //Reading Image from Bridge
    PRINTF("Loading Image from File\n");
    if ((ReadImageFromFile(ImageName, &Wi, &Hi, ImageInChar, W * H * sizeof(unsigned char)) == 0) || (Wi != W) || (Hi != H))
    {
        PRINTF("Failed to load image %s or dimension mismatch Expects [%dx%d], Got [%dx%d]\n", ImageName, W, H, Wi, Hi);
        pmsis_exit(-1);
    }

    #elif defined INPUT_RAW_FILE
    //Load 16 bits raw image

    if(read_raw_image(RawImageName, ImageIn,W,H)){
        PRINTF("Failed to load raw image\n");
        pmsis_exit(-1);
    }
    #endif

    /* Configure And open cluster. */
    struct pi_cluster_conf cl_conf;
    cl_conf.id = 0;
    pi_open_from_conf(&cluster_dev, (void *) &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
        PRINTF("Cluster open failed !\n");
        pmsis_exit(-7);
    }

    pi_freq_set(PI_FREQ_DOMAIN_FC,250000000);
    pi_freq_set(PI_FREQ_DOMAIN_CL,175000000);

    PRINTF("Init NN\n");
    if(initNN())
    {
        PRINTF("NN Init exited with an error\n");
        return 1;
    }

    PRINTF("Allocating Buffers in L3\n");
    if (initL3Buffers())
    {
        PRINTF("L3 Allocation exited with an error\n");
        return 1;
    }

    PRINTF("Constructor\n");
    if (lynredCNN_Construct(0))
    {
        PRINTF("Graph constructor exited with an error\n");
        return 1;
    }
    //Deallocating L1 to be used by other cluster calls
    if(lynredCNN_Destruct(1)){
        printf("Error deallocating L1 for cluster...\n");
        pmsis_exit(-1);
    }

    #if !defined(INPUT_RAW_FILE) && !defined(INPUT_FILE)
    PRINTF("Opening camera\n");
    if (open_camera_thermeye(&cam))
    {
        PRINTF("Thermal Eye camera open failed !\n");
        pmsis_exit(-1);
    }

    #ifdef OFFSET_IMAGE_EVERY_BOOT
    //This taking the offset each time we turn on the board
    PRINTF("Shooting offset, cover sensor with a black body!\n");
    pi_gpio_pin_write(NULL, GPIO_USER_LED, 0);
    pi_time_wait_us(2 * 1000 * 1000);
    pi_camera_control(&cam, PI_CAMERA_CMD_START, 0);
    pi_camera_capture(&cam, img_offset, W*H * sizeof(uint16_t));
    pi_camera_control(&cam, PI_CAMERA_CMD_STOP, 0);
    pi_gpio_pin_write(NULL, GPIO_USER_LED, 1);
    PRINTF("Offset image taken!\n");
    #endif
    #endif

    #ifdef USE_BLE
    PRINTF("Init BLE\n");
    int status;
    status = initHandler();

    if(status)
    {
        PRINTF("User manager init failed!\n");
        pmsis_exit(-5);
    }
    #endif

    PRINTF("Running NN\n");
    struct pi_cluster_task *task = pmsis_l2_malloc(sizeof(struct pi_cluster_task));
    if(task==NULL) {
        PRINTF("Alloc Error! \n");
        pmsis_exit(-7);
    }
    memset(task, 0, sizeof(struct pi_cluster_task));
    task->entry = RunNN;
    task->arg = (void *) NULL;
    task->stack_size = (uint32_t) STACK_SIZE;
    task->slave_stack_size = (uint32_t) SLAVE_STACK_SIZE;


    char iterate=1;
    while(iterate){


        #if defined(INPUT_RAW_FILE) || defined(INPUT_FILE)
        iterate=0;
        #else
        PRINTF("Taking Picture!\n");
        pi_gpio_pin_write(NULL, USER_GPIO, 0);
        
        pi_camera_control(&cam, PI_CAMERA_CMD_START, 0);
        pi_camera_capture(&cam, ImageIn, W*H*sizeof(int16_t));
        pi_camera_control(&cam, PI_CAMERA_CMD_STOP, 0);
        pi_gpio_pin_write(NULL, USER_GPIO , 1);
        #endif

        #ifndef INPUT_FILE

        PRINTF("Calling shutterless filtering\n");
        //The shutterless floating point version was done just for reference...very slow on gap.
        //if(float_shutterless(ImageIn, img_offset,W,H,8,1)){

        int tm = pi_time_get_us();
        
        if(fixed_shutterless(ImageIn, img_offset,W,H,8)){
            PRINTF("Error Calling prefiltering, exiting...\n");
            pmsis_exit(-8);
        }

        tm = pi_time_get_us() - tm;
        PRINTF("Shutterless %.02f ms\n", ((float)tm)/1000);
        
        #endif
        PRINTF("Call cluster\n");
        //Explicitly allocating Cluster stack since it could also be used by shutterless
        task->stacks = pmsis_l1_malloc(STACK_SIZE+SLAVE_STACK_SIZE*7);
    
        //Calling warm constructor to allocate only L1
        if(lynredCNN_Construct(1)){
            printf("Error allocating L1 for cluster...\n");
            pmsis_exit(-1);
        }
        
        pi_cluster_send_task_to_cl(&cluster_dev, task);

        lynredCNN_Destruct(1);
        pmsis_l1_malloc_free(task->stacks,STACK_SIZE+SLAVE_STACK_SIZE*7);
        
        #ifdef SAVE_TO_PC
        char string_buffer[50];
        sprintf(string_buffer, "../../../dump_out_imgs/img_%04ld.pgm", save_index);
        unsigned char *img_out_ptr = ImageIn;
        //for(int i=0;i<W*H;i++) img_out_ptr[i] = (uint8_t) (ImageIn[i] >> (INPUT1_Q - 8));
        drawBboxes(out_boxes,img_out_ptr,80,80);
        WriteImageToFile(string_buffer, W, H, img_out_ptr);
        save_index++;
        #endif

        #if defined USE_BLE
        sendResultsToBle(&bbxs);
        #ifndef SAVE_TO_PC
        //pi_time_wait_us(2 * 1000 * 1000);
        #endif
        #endif

        #ifdef SLEEP
        //This is not the optimized deep sleep, when should take care of pad setting in sleep
        //and to properly shutdown all external devices
        go_to_sleep();
        #endif
    }

    lynredCNN_Destruct(0);

    // Close the cluster
    pi_cluster_close(&cluster_dev);

    PRINTF("Ended\n");
    pmsis_exit(0);
}


int main(void)
{
    PRINTF("\n\n\t *** Therm Eye ***\n");
    PRINTF("\n\t   %s Version\n\n", FLASH_NAME);
    return pmsis_kickoff((void *) peopleDetection);
}
