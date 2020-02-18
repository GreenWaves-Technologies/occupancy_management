#ifndef __SSDKERNEL_H__
#define __SSDKERNEL_H__

#include "AutoTilerLibTypes.h"
#include "SSDBasicKernels.h"
#define _SSDKernels_L1_Memory_SIZE 37992
#define _SSDKernels_L2_Memory_SIZE 37992
extern char *SSDKernels_L1_Memory; /* Size given for generation: 38000 bytes, used: 37992 bytes */
extern char *SSDKernels_L2_Memory; /* Size used for generation: 37992 bytes */
extern void SDD3Dto2DSoftmax_40_40_16(
		short * In,
		short * Out,
		unsigned char Q,
		unsigned short n_classes);
extern void SDD3Dto2D_40_40_32(
		short * In,
		short * Out,
		unsigned char Q,
		unsigned short n_classes);
extern void Predecoder40_40(
		short * Classes,
		short * Boxes,
		void  * Ancor_layer,
		void  * BoundingBoxes,
		int Boxes_Q);
extern void SDD3Dto2DSoftmax_20_20_16(
		short * In,
		short * Out,
		unsigned char Q,
		unsigned short n_classes);
extern void SDD3Dto2D_20_20_32(
		short * In,
		short * Out,
		unsigned char Q,
		unsigned short n_classes);
extern void Predecoder20_20(
		short * Classes,
		short * Boxes,
		void  * Ancor_layer,
		void  * BoundingBoxes,
		int Boxes_Q);
extern void SDD3Dto2DSoftmax_10_10_16(
		short * In,
		short * Out,
		unsigned char Q,
		unsigned short n_classes);
extern void SDD3Dto2D_10_10_32(
		short * In,
		short * Out,
		unsigned char Q,
		unsigned short n_classes);
extern void Predecoder10_10(
		short * Classes,
		short * Boxes,
		void  * Ancor_layer,
		void  * BoundingBoxes,
		int Boxes_Q);
extern void SDD3Dto2DSoftmax_5_5_16(
		short * In,
		short * Out,
		unsigned char Q,
		unsigned short n_classes);
extern void SDD3Dto2D_5_5_32(
		short * In,
		short * Out,
		unsigned char Q,
		unsigned short n_classes);
extern void Predecoder5_5(
		short * Classes,
		short * Boxes,
		void  * Ancor_layer,
		void  * BoundingBoxes,
		int Boxes_Q);
#endif
