/*******************************
shadowCreate controler .
v0.1. create         20191111     zk.xu
v0.2. add k11       2020           zk.xu

******************************/

#ifndef __D_H__
#define __D_H__
#include "user_config.h"

#define BITS_PER_LONG 32
#define BIT(nr)            (1UL << (nr))
#define BIT_MASK(nr)        (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)        ((nr) / BITS_PER_LONG)


enum DATA_TYPE
{
    TIME_STAMP,
    PACKET_ID,
    MAGX, MAGY, MAGZ,
    ACCX, ACCY, ACCZ,
    GYROX, GYROY, GYROZ,
    REV1, REV2, REV3,
    REV4, REV5, REV6,

    TOUCHX, TOUCHY,
    KEY_S_TG,
    KEY_S_MOV,
    KEY_S_BACK,
    KEY_S_POWER,
    KEY_ENTER,
    KEY_EARSE_BONDS,
    KEY_VOL,
};


#ifdef  USE_K11

    #define TM_BW  8

    #define PI_BW  8

    #define MX_BW  16
    #define MY_BW  16
    #define MZ_BW  16

    //64

#ifdef USE16BIT

    #define AX_BW  16
    #define AY_BW  16
    #define AZ_BW  16

    #define GX_BW  16
    #define GY_BW  16
    #define GZ_BW  16

    #define R1_BW  7
    #define R2_BW  7
    #define R3_BW  7

#else
    #define AX_BW  13
    #define AY_BW  13
    #define AZ_BW  13

    #define GX_BW  13
    #define GY_BW  13
    #define GZ_BW  13

    #define R1_BW  13
    #define R2_BW  13
    #define R3_BW  13
    //117
#endif

    #define TX_BW  4
    #define TY_BW  4
    #define COMB_TXY_BW  8

    #define BUTTON_BW  8

    #define COMB_TXYB_BW  16

    #define BAT_BW  8

    #define COMB_TX2END_BW   24
    //24
    //205/8=26


    #define TM_M  ((1<<TM_BW)-1)
    #define PI_M  ((1<<PI_BW)-1)

    #define MX_M  ((1<<MX_BW)-1)
    #define MY_M  ((1<<MY_BW)-1)
    #define MZ_M  ((1<<MZ_BW)-1)
    #define AX_M  ((1<<AX_BW)-1)
    #define AY_M  ((1<<AY_BW)-1)
    #define AZ_M  ((1<<AZ_BW)-1)
    #define GX_M  ((1<<GX_BW)-1)
    #define GY_M  ((1<<GY_BW)-1)
    #define GZ_M  ((1<<GZ_BW)-1)

    #define R1_M  ((1<<R1_BW)-1)
    #define R2_M  ((1<<R2_BW)-1)
    #define R3_M  ((1<<R3_BW)-1)

    #define TX_M  ((1<<TX_BW)-1)
    #define TY_M  ((1<<TY_BW)-1)
    #define SS_M  ((1<<SENSE_BW)-1)
    #define BT_M  ((1<<BUTTON_BW)-1)
    #define H1_M  ((1<<H1_BW)-1)
    #define H2_M  ((1<<H2_BW)-1)
    #define BAT_M  ((1<<BAT_BW)-1)



    #define TM_OF   0
    #define PI_OF  (TM_OF+TM_BW)


    #define MX_OF  (PI_OF+PI_BW)
    #define MY_OF  (MX_OF+MX_BW)
    #define MZ_OF  (MY_OF+MY_BW)

    #define AX_OF  (MZ_OF+MZ_BW)
    #define AY_OF  (AX_OF+AX_BW)
    #define AZ_OF  (AY_OF+AY_BW)
    #define GX_OF  (AZ_OF+AZ_BW)
    #define GY_OF  (GX_OF+GX_BW)
    #define GZ_OF  (GY_OF+GY_BW)
    #define R1_OF  (GZ_OF+GZ_BW)
    #define R2_OF  (R1_OF+R1_BW)
    #define R3_OF  (R2_OF+R2_BW)

    #define TX_OF  (R3_OF+R3_BW)
    #define TY_OF  (TX_OF+TX_BW)
    #define BT_OF  (TY_OF+TY_BW)
    #define BAT_OF (BT_OF+BUTTON_BW)

    #define MAX_DSIZE (BAT_OF+BAT_BW+7)/8



#else
    #define TM_BW  8

    #define PI_BW  8

    #define MX_BW  16
    #define MY_BW  16
    #define MZ_BW  16

    //64

#ifdef USE16BIT
    
    #define AX_BW  16
    #define AY_BW  16
    #define AZ_BW  16
    
    #define GX_BW  16
    #define GY_BW  16
    #define GZ_BW  16
    
    #define R1_BW  7
    #define R2_BW  7
    #define R3_BW  7
    
#else

    #define AX_BW  13
    #define AY_BW  13
    #define AZ_BW  13

    #define GX_BW  13
    #define GY_BW  13
    #define GZ_BW  13

    #define R1_BW  13
    #define R2_BW  13
    #define R3_BW  13
    //117
#endif

    #define TX_BW  4
    #define TY_BW  4

    #define COMB_TXY_BW  8//(TX_BW+TY_BW)


    #define SENSE_BW  4
    #define BUTTON_BW  4
    #define COMB_SB_BW  8//(SENSE_BW+BUTTON_BW)

    #define COMB_TXYS_BW  12//(COMB_TXY_BW+SENSE_BW)
    #define COMB_TXYSB_BW  16// (COMB_TXY_BW+COMB_SB_BW)


    #define H1_BW   4
    #define H2_BW   4
    #define COMB_H_BW   8

    #define COMB_TXYSBH1_BW   20//(COMB_TXYSB_BW+H1_BW)
    #define COMB_TXYSBH_BW   24//(COMB_TXYSB_BW+COMB_H_BW)



    #define BAT_BW  3
    // 27

    #define COMB_TX2END_BW   27



    //208/8=26

    // 208/8= 25.625

    #define TM_M  ((1<<TM_BW)-1)
    #define PI_M  ((1<<PI_BW)-1)

    #define MX_M  ((1<<MX_BW)-1)
    #define MY_M  ((1<<MY_BW)-1)
    #define MZ_M  ((1<<MZ_BW)-1)
    #define AX_M  ((1<<AX_BW)-1)
    #define AY_M  ((1<<AY_BW)-1)
    #define AZ_M  ((1<<AZ_BW)-1)
    #define GX_M  ((1<<GX_BW)-1)
    #define GY_M  ((1<<GY_BW)-1)
    #define GZ_M  ((1<<GZ_BW)-1)

    #define R1_M  ((1<<R1_BW)-1)
    #define R2_M  ((1<<R2_BW)-1)
    #define R3_M  ((1<<R3_BW)-1)

    #define TX_M  ((1<<TX_BW)-1)
    #define TY_M  ((1<<TY_BW)-1)
    #define SS_M  ((1<<SENSE_BW)-1)
    #define BT_M  ((1<<BUTTON_BW)-1)
    #define H1_M  ((1<<H1_BW)-1)
    #define H2_M  ((1<<H2_BW)-1)
    #define BAT_M  ((1<<BAT_BW)-1)



    #define TM_OF   0
    #define PI_OF  (TM_OF+TM_BW)


    #define MX_OF  (PI_OF+PI_BW)
    #define MY_OF  (MX_OF+MX_BW)
    #define MZ_OF  (MY_OF+MY_BW)

    #define AX_OF  (MZ_OF+MZ_BW)
    #define AY_OF  (AX_OF+AX_BW)
    #define AZ_OF  (AY_OF+AY_BW)
    #define GX_OF  (AZ_OF+AZ_BW)
    #define GY_OF  (GX_OF+GX_BW)
    #define GZ_OF  (GY_OF+GY_BW)
    #define R1_OF  (GZ_OF+GZ_BW)
    #define R2_OF  (R1_OF+R1_BW)
    #define R3_OF  (R2_OF+R2_BW)


    #define TX_OF  (R3_OF+R3_BW)
    #define TY_OF  (TX_OF+TX_BW)
    #define SS_OF  (TY_OF+TY_BW)
    #define BT_OF  (SS_OF+SENSE_BW)
    #define H1_OF  (BT_OF+BUTTON_BW)
    #define H2_OF  (H1_OF+H1_BW)
    #define BAT_OF  (H2_OF+H2_BW)


    #define MAX_DSIZE (BAT_OF+BAT_BW+7)/8


#endif

void set_data(uint8_t st, uint8_t ll, uint32_t data);
uint32_t get_entry(uint8_t **data);
uint32_t set_entry(uint8_t *data);
uint32_t get_data(uint8_t st, uint8_t ll);

void get_imu_hc(uint8_t *tm, imu_packet_t *pimu);
#ifdef USE_LA_RCOV
    void set_imu_hc(uint8_t tm, imu_packet_t *pimu, int16_t *pla);
#else
    void set_imu_hc(uint8_t tm, imu_packet_t *pimu);
#endif

#endif
