/*
 * Copyright (C) 2020 GreenWaves Technologies
 * All rights reserved.
 *
 *
 */

//Fixed point versions
int32_t preFiltering_fixed(uint16_t* img_input_fp16,uint16_t* img_offset_fp16,uint8_t q_output);

//This is the new filtering integrating bad pixels handling:
int32_t shutterless_fixed(uint16_t* img_input_fp16,uint16_t* img_offset_fp16,uint8_t q_output);

//Floating point versions
int32_t preFiltering_float(uint16_t* img_input_fp16,uint16_t* img_offset_fp16,uint8_t q_output);
int32_t shutterless_float(unsigned short* img_input_fp16,unsigned short* img_offset_fp16,uint8_t q_output, float gamma);