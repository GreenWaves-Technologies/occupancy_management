/* PMSIS includes. */
#include "pmsis.h"
#include "bsp/bsp.h"
#include "bsp/camera.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "pmsis.h"
#include "bsp/fs.h"
#include "BleNotifier.h"


//#define ASYNC
//#define BLE 

#define IMG_WIDTH  ( 80 )
#define IMG_HEIGHT ( 80 )
#define IMG_SIZE   ( IMG_HEIGHT * IMG_WIDTH )


static struct pi_device cam;

static uint16_t calib_buffer[IMG_SIZE];
static uint16_t img_buffer[IMG_SIZE];
static int16_t ref_buffer[IMG_SIZE];
static int16_t dif_buff[IMG_SIZE];
static uint8_t scaled_buff[IMG_SIZE];

#define GPIO_USER_LED 0

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

void test_therm_eye()
{
    printf("Entering main controller\n");
    
    pi_gpio_pin_write(NULL, GPIO_USER_LED, 1);
    pi_freq_set(PI_FREQ_DOMAIN_FC,250000000);
    
    //Use bluetooth to get same noise aspect
    #ifdef BLE 
    int errors;
    errors = initHandler();
    if(errors)
    {
        printf("User manager init failed!\n");
        pmsis_exit(-5);
    }
    #endif
 

    if (open_camera_thermeye(&cam))
    {
        printf("Thermal Eye camera open failed !\n");
        pmsis_exit(-1);
    }

    pi_task_t cb = {0};
    pi_task_block(&cb);

    printf("Cover sensor until end of calibration(led will switch off).\n");
    /* Calibration step. */
    pi_gpio_pin_write(NULL, GPIO_USER_LED, 0);
    pi_time_wait_us(3 * 1000 * 1000);
    //printf("Buffer %p, size %d\n", calib_buffer, IMG_SIZE * sizeof(uint16_t));
    #if defined(ASYNC)
    pi_task_block(&cb);
    pi_camera_control(&cam, PI_CAMERA_CMD_STOP, 0);
    pi_camera_capture_async(&cam, calib_buffer, IMG_SIZE * sizeof(uint16_t), &cb);
    pi_camera_control(&cam, PI_CAMERA_CMD_START, 0);
    pi_task_wait_on(&cb);
    #else
    pi_camera_control(&cam, PI_CAMERA_CMD_START, 0);
    pi_camera_capture(&cam, calib_buffer, IMG_SIZE * sizeof(uint16_t));
    pi_camera_control(&cam, PI_CAMERA_CMD_STOP, 0);
    #endif  /* ASYNC */

    pi_gpio_pin_write(NULL, GPIO_USER_LED, 1);
    printf("Calibration done!\n");
    pi_camera_close(&cam);
    
    printf("Saving Calibrated Image to PC...\n");
    printf("%d %d %d %d\n",calib_buffer[80*80-1],calib_buffer[5000],calib_buffer[2000],calib_buffer[1000]);
    char string_buffer[50];
    sprintf(string_buffer, "../../../Calibration.bin");
    struct pi_fs_conf conf;
    pi_fs_conf_init(&conf);
    struct pi_device fs;
    conf.type = PI_FS_HOST;
    pi_open_from_conf(&fs, &conf);
    if (pi_fs_mount(&fs)) return ;
    void *File = pi_fs_open(&fs, string_buffer, PI_FS_FLAGS_WRITE);    
    #define CHUNK_SIZE 1024
    unsigned char * OutBuffer = (unsigned char * ) calib_buffer;
    int datasize = IMG_SIZE * sizeof(uint16_t);
    int steps = datasize / CHUNK_SIZE;


    for (int i=0; i<steps; i++)
    {
        pi_fs_write(File,OutBuffer +(CHUNK_SIZE*i), CHUNK_SIZE);
    }
    if (((datasize) % CHUNK_SIZE) != 0)
    {
        pi_fs_write(File,OutBuffer+(CHUNK_SIZE*steps) , ((datasize) % CHUNK_SIZE)*sizeof(unsigned char));
    }

    pi_fs_close(File);
    pi_fs_unmount(&fs);
    printf("Exit ...\n");

    pmsis_exit(0);
}

/* Program Entry. */
int main(void)
{
    printf("\n\n\t *** Therm Eye Calibration Software ***\n\n");
    return pmsis_kickoff((void *) test_therm_eye);
}
