#ifndef __STATIC_USER_MANAGER_BLE_NOTIFIER_H__
#define __STATIC_USER_MANAGER_BLE_NOTIFIER_H__

#include "pmsis.h"

int initHandler();
uint8_t handleDetections(char* name,int size);
void closeHandler();

#endif
