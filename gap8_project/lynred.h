
#ifndef __LYNRED_H__
#define __LYNRED_H__

#ifdef __EMUL__
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>
#include <string.h>
#include "helpers/helpers.h"
#endif

extern AT_HYPERFLASH_FS_EXT_ADDR_TYPE lynred_L3_Flash;

#define INPUT1_Q 15

#define OUTPUT1_Q 12
#define OUTPUT2_Q 12
#define OUTPUT3_Q 12
#define OUTPUT4_Q 12
#define OUTPUT5_Q 12
#define OUTPUT6_Q 12
#define OUTPUT7_Q 11
#define OUTPUT8_Q 12

#endif //__LYNRED_H__