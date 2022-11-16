/*******************************
shadowCreate controler .
v0.1. create                    20191201     zk.xu
v0.2. add special K11           20200730     zk.xu

******************************/


#ifndef __USR_H__
#define __USR_H__

//#define TEST_MODE

#define USE16BIT
//#define USE_8G
#define USE_16G

//#define USE_LSM_8G 
//#define USE_ICM_8G 

//#define RL_CTL

//todo:
#define CHANGE_TEST_2
//#define USE_MORE_CFG_BY_RF


//#define CTL_RESEND //error

//#ifndef USE_K11

//#if defined(K11_P2)||defined(K102_FROM_K101)||defined(USE_K101_P2) 
    #define USE_GYRO_4000
//#endif


#ifndef USE_K11
    //#define USE_LA_RCOV
#endif

#ifdef USE_K11
    //#define USE_MAG
#define USE_K11_RM_MOS_EN
#endif

#define USE_RLED_1DM //dl1ms      
#define USE_RLED_PATTEN_10
#define USE_RLED_PATTEN_10_METHOD2

#ifdef USE_HOST
    #define HOST_DIR_AP
    #define HOST_ADD_BOOT
#else

#endif

#define SWITCH_CHANGE

#ifdef CHANGE_TEST_2

#if 1
#define TIME_CNT    50
#define DIV_CNT_SHIFT    1


#define EXP_NEGDELY  4000

#define RLDCFG_TRSMIT   (3000)
#define RLDCFG   (RLDCFG_TRSMIT/TIME_CNT)

#define AP_DELAY_FIXED  200
#define AP_DELAY_VAR    50//450
#define HOST_DELAY_VAR   0

#define DELAY_VAR    (HOST_DELAY_VAR+AP_DELAY_VAR)
#define DELAY_AND_SHIFT  (AP_DELAY_FIXED+DELAY_VAR)
#define BONCE_TIME       (8000/TIME_CNT)

#define DELAY_VAR_TEST       (0)//test to be remove

#define RLED_TIME_TRIP        ((EXP_NEGDELY-RLDCFG_TRSMIT-DELAY_AND_SHIFT-DELAY_VAR_TEST)/TIME_CNT)  //133340

#if 0//def K102_FROM_K101
#define DELAY_VAR2    50

#else 

#define DELAY_VAR2    100//650//test to be remove
#endif

//#define RLED_DRUT_CNT ((DELAY_VAR+DELAY_VAR2+TIME_CNT)/TIME_CNT)
#define RLED_DRUT_CNT ((0+DELAY_VAR2+TIME_CNT)/TIME_CNT)

#else
    #define TIME_CNT    50
    #define DIV_CNT_SHIFT    1


    #define EXP_NEGDELY  4000

    #define RLDCFG_TRSMIT   (3000)
    #define RLDCFG   (RLDCFG_TRSMIT/TIME_CNT)

    #define AP_DELAY_FIXED  200
    #define AP_DELAY_VAR    500//450
    #define HOST_DELAY_VAR   50

    #define DELAY_VAR    (HOST_DELAY_VAR+AP_DELAY_VAR)
    #define DELAY_AND_SHIFT  (AP_DELAY_FIXED+DELAY_VAR)
    #define BONCE_TIME       (8000/TIME_CNT)

    #define DELAY_VAR_TEST       (0)//test to be remove

    #define RLED_TIME_TRIP        ((EXP_NEGDELY-RLDCFG_TRSMIT-DELAY_AND_SHIFT-DELAY_VAR_TEST)/TIME_CNT)  //133340

    #define DELAY_VAR2    400//650//test to be remove

    #define RLED_DRUT_CNT ((DELAY_VAR+DELAY_VAR2+TIME_CNT)/TIME_CNT)
#endif

#else
    #define TIME_CNT    100

    #define EXP_NEGDELY  4000

    #define RLDCFG_TRSMIT   (3000)
    #define RLDCFG   (RLDCFG_TRSMIT/TIME_CNT)

    #define AP_DELAY_FIXED  300
    #define AP_DELAY_VAR    500
    #define HOST_DELAY_VAR   100

    #define DELAY_VAR    (HOST_DELAY_VAR+AP_DELAY_VAR)
    #define DELAY_AND_SHIFT  (AP_DELAY_FIXED+DELAY_VAR)
    #define BONCE_TIME       (8000/TIME_CNT)

    #define DELAY_VAR_TEST       (0)//test to be remove

    #define RLED_TIME_TRIP        ((EXP_NEGDELY-RLDCFG_TRSMIT-DELAY_AND_SHIFT-DELAY_VAR_TEST)/TIME_CNT)  //133340

    #define DELAY_VAR2    700//test to be remove

    #define RLED_DRUT_CNT ((DELAY_VAR+DELAY_VAR2+TIME_CNT)/TIME_CNT)



#endif




typedef struct
{
    uint8_t rts;
    uint8_t seq;
    uint8_t touchx;
    uint8_t touchy; //5
    uint8_t d[3];
    uint16_t a[3];
    uint16_t L[3];
} __attribute__((packed)) usr_packet_t;

typedef struct
{
    int16_t d[3];
    int16_t a[3];
    int16_t g[3];
//#ifdef USE_MAG
    int16_t m[3];
//#endif
    uint8_t s;
} imu_packet_t;

#define OFFSET_A_OF_IMUPKT 6
#define OFFSET_A_OF_USRPKT 11


#define MAG_CALIB_TEST_MORE

#define RGB_USR_ONE

//#define CALIB_GYRO_ONLY 


#define ADD_CTL_TYPE_MISC

#endif

