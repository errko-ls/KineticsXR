
/*********************************************

shadowCreate controler .
v0.1. create        20191111           zk.xu


********************************************/

#ifndef __RF0__
#define __RF0__
#include "user_config.h"


#define BOND_MAGIC_TAG 0xb5a6c7d8
#define BOND_CMD_TAG      0x36
#define BOND_RES_TAG2     0x26
#define BOND_RES_TAG      0x24



#define BOND_CMD_LEN      (7)

#define BOND_CMD_LEFT     0x47
#define BOND_CMD_RIGHT    0x58

#define REQ_LEFT   0x01
#define REQ_RIGHT  0x02
#define REQ_LASTL  0x04
#define REQ_LASTR  0x08

// 2bit vib
#define REQ_VIBR_SHIFT   6 // 4
#define REQ_VIBL_SHIFT   4 // 6

#define REQ_VIBL_MASK   0x30
#define REQ_VIBR_MASK   0xc0

//#define REQ_VIBR   0x10 // 0x20  //0x30 // 0x00
//#define REQ_VIBL   0x40 // 0x80  //0xc0  //0x00


#define REQ_LED   0x10

#define REQ_TAG    0x69
#define REQ_CMD_LEN     (5)

#define DATA_TAG_LEFT    0x75
#define DATA_TAG_RIGHT   0x7f

#define MAX_SEQ   255


#define  CUST_RF_TIMER_CNT_DATA 900


#define  CUST_RF_TIMER_CNT_DATA2 335//335//330

#define  CUST_RF_TIMER_CNT_DATA_HOST 900


#define  CUST_RF_TIMER_CNT_DATA_CTL2 700 //



#endif


