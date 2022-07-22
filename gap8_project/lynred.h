
#ifndef __LYNRED_H__
#define __LYNRED_H__

#include "lynredInfo.h"
#include "Gap.h"

#if defined(QSPI)
extern AT_QSPIFLASH_FS_EXT_ADDR_TYPE lynred_L3_Flash;
#else
extern AT_HYPERFLASH_FS_EXT_ADDR_TYPE lynred_L3_Flash;
#endif
#define INPUT1_Q lynred_Input_1_Q

#define OUTPUT1_Q lynred_Output_1_Q
#define OUTPUT2_Q lynred_Output_2_Q
#define OUTPUT3_Q lynred_Output_3_Q
#define OUTPUT4_Q lynred_Output_4_Q
#define OUTPUT5_Q lynred_Output_5_Q
#define OUTPUT6_Q lynred_Output_6_Q
#define OUTPUT7_Q lynred_Output_7_Q
#define OUTPUT8_Q lynred_Output_8_Q

#endif //__LYNRED_H__
