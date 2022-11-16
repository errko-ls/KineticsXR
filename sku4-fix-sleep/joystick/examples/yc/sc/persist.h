/*********************************************

shadowCreate controler .
v0.1. create        20191111           zk.xu


********************************************/


#ifndef __PERSIST_H__
#define __PERSIST_H__

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "fds.h"

#define GYRO_FILE_ID     0x0001
#define GYRO_MAGIC_KEY   0x0001
#define GYRO_REC_KEY     0x0002

#define ACC_FILE_ID     0x0002
#define ACC_MAGIC_KEY   0x0001
#define ACC_REC_KEY     0x0002

#define MAG_FILE_ID     0x0003
#define MAG_MAGIC_KEY   0x0001
#define MAG_REC_KEY     0x0002

#define BOND_FILE_ID    0x0004
#define BOND_L_KEY      0x0001
#define BOND_R_KEY      0x0002
#define BOND_KEY        0x0003
#define BOND_MAGIC      0x0004
#define GROUP_IDX       0x0005
#define JOYSTICK_L_PRODUCT_ID (0x0006)
#define JOYSTICK_R_PRODUCT_ID (0x0007)
#define JOYSTICK_L_VERSION (0x000b) 
#define JOYSTICK_R_VERSION (0x000c) 



void persist_init(void);
ret_code_t persist_read(uint32_t *readData, uint32_t fileId, uint32_t recKey);
ret_code_t persist_write(uint32_t *writeData, uint32_t fileId, uint32_t recKey, uint32_t size, uint32_t sync);
ret_code_t persist_delete(uint32_t fileId, uint32_t recKey, uint32_t sync);



#endif
