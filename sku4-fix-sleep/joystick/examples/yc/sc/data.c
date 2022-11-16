
/*********************************************

shadowCreate controler .
v0.1. create        20191111           zk.xu
v0.2. add k11       2020           zk.xu


********************************************/


#include "nrf_esb.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "boards.h"
#include "bsp.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_error.h"
#include "gbltype.h"
#include "user_config.h"

#include "data.h"


//uint8_t gs_data[MAX_DSIZE];

uint8_t *gs_data;



u32 get_data(u8 st, u8 ll)
{
    u32 result = 0;

    u8 i;
    u8 ss = st >> 3;
    u8 ee = (st + ll - 1) >> 3;
    u8 mod = st % 8 ;
    u32 m = ((1 << ll) - 1);

    i = ee - ss;

    switch (i)
    {
    case 0:
        result = (gs_data[ss] >> mod)&m;
        break;

    case 1:
        result = (gs_data[ss] >> mod);
        result |= (gs_data[ee] << (8 - mod));

        result = result & m;

        break;

    case 2:
        result = (gs_data[ss] >> mod);
        result |= (gs_data[ss + 1] << (8 - mod));
        result |= (gs_data[ee] << (16 - mod));

        result = result & m;


        break;


    case 3:
        result = (gs_data[ss] >> mod);
        result |= (gs_data[ss + 1] << (8 - mod));
        result |= (gs_data[ss + 2] << (16 - mod));
        result |= (gs_data[ee] << (24 - mod));

        result = result & m;

        break;

    case 4:
        result = (gs_data[ss] >> mod);
        result |= (gs_data[ss + 1] << (8 - mod));
        result |= (gs_data[ss + 2] << (16 - mod));
        result |= (gs_data[ss + 3] << (24 - mod));

        result |= (gs_data[ee] << (32 - mod));

        result = result & m;

        break;
    }

    //NRF_LOG_INFO("[%d] ",result);
    return result;

}

// 0  8
void set_data(uint8_t st, uint8_t ll, uint32_t data)
{
    u8 i;
    u8 ss = st >> 3; //0
    u8 ee = (st + ll - 1) >> 3; // 0
    u8 mod = st % 8 ; //0

    data &= ((1 << ll) - 1);

    i = ee - ss; //0


    switch (i)
    {
    case 0:
        gs_data[ss] |= data << mod;
        break;

    case 1:
        gs_data[ss] |= data << mod;
        gs_data[ee] |= data >> (8 - mod);

        break;

    case 2:
        gs_data[ss] |= data << mod;
        gs_data[ss + 1] |= data >> (8 - mod);
        gs_data[ee] |= data >> (16 - mod);

        break;


    case 3:
        gs_data[ss] |= data << mod;
        gs_data[ss + 1] |= data >> (8 - mod);
        gs_data[ss + 2] |= data >> (16 - mod);
        gs_data[ee] |= data >> (24 - mod);
        break;

    case 4:
        gs_data[ss] |= data << mod;
        gs_data[ss + 1] |= data >> (8 - mod);
        gs_data[ss + 2] |= data >> (16 - mod);
        gs_data[ss + 3] |= data >> (24 - mod);
        gs_data[ee] |= data >> (32 - mod);


        break;
    }




}


#ifdef USE_LA_RCOV
    void set_imu_hc(uint8_t tm, imu_packet_t *pimu, int16_t *pla)
#else
    void set_imu_hc(uint8_t tm, imu_packet_t *pimu)
#endif
{

    gs_data[0] = tm;
    gs_data[1] = pimu->s;

    gs_data[2] = pimu->d[0] & 0xff;
    gs_data[3] = pimu->d[0] >> 8;

    gs_data[4] = pimu->d[1] & 0xff;
    gs_data[5] = pimu->d[1] >> 8;

    gs_data[6] = pimu->d[2] & 0xff;
    gs_data[7] = pimu->d[2] >> 8;

#ifdef USE16BIT

    gs_data[8] = pimu->a[0]& 0xff;
    gs_data[9] = pimu->a[0]>> 8;
    
    gs_data[10] = pimu->a[1]& 0xff;
    gs_data[11] = pimu->a[1]>> 8;
    
    gs_data[12] = pimu->a[2]& 0xff;
    gs_data[13] = pimu->a[2]>> 8;

    gs_data[14] = pimu->g[0]& 0xff;
    gs_data[15] = pimu->g[0]>> 8;
    
    gs_data[16] = pimu->g[1]& 0xff;
    gs_data[17] = pimu->g[1]>> 8;
    
    gs_data[18] = pimu->g[2]& 0xff;
    gs_data[19] = pimu->g[2]>> 8;
 
#else
#if 0
    set_data(AX_OF, AX_BW, pimu->a[0]);
    set_data(AY_OF, AY_BW, pimu->a[1]);
    set_data(AZ_OF, AZ_BW, pimu->a[2]);
    
    set_data(GX_OF, GX_BW, pimu->g[0]);
    set_data(GY_OF, GY_BW, pimu->g[1]);
    set_data(GZ_OF, GZ_BW, pimu->g[2]);

    //set_data(R1_OF, R1_BW, 0);
#else
    set_data(AX_OF, AX_BW + AY_BW, ((pimu->a[1]&0x1fff) << AX_BW) | (pimu->a[0]&0x1fff));

    set_data(AZ_OF, AZ_BW + GX_BW, ((pimu->g[0]&0x1fff) << AZ_BW) | (pimu->a[2]&0x1fff));

    set_data(GY_OF, GY_BW + GZ_BW, ((pimu->g[2]&0x1fff) << GY_BW) | (pimu->g[1]&0x1fff));
    
#endif
#endif

//
#ifdef USE_LA_RCOV
    set_data(R1_OF, R1_BW + R2_BW, (pla[1] << R1_BW) | pla[0]);
    set_data(R3_OF, R3_BW, pla[2]);
#else
    #ifdef USE_MAG
    set_data(R1_OF, R1_BW + R2_BW, (pimu->m[1] << R1_BW) | pimu->m[0]);
    set_data(R3_OF, R3_BW, pimu->m[2]);
    #endif    
#endif


#if 0//dd
    {
    if(!logflag){

    #ifdef USE_GYRO_4000
    #define  FACTOR_GYRO_RAW2U 0.002130606158f
    #define  FACTOR_GYRO_U2RAW 469.35f
    #else

    #define  FACTOR_GYRO_RAW2U 0.001065303079f
    #define  FACTOR_GYRO_U2RAW 938.7f
    #endif
    
    #define OFFSET_INT32_13 19
    #define OFFSET_INT32_16 16

        //int32_t gz = ((int32_t)get_data(GZ_OF, GZ_BW) << OFFSET_INT32_13) >> OFFSET_INT32_13;
         
    u32 gz = (u32)get_data(GZ_OF, GZ_BW);
         
        // float g2 = (float) (gz << 4) * FACTOR_GYRO_RAW2U;
         
        // int32_t gz = ((int32_t)get_data(GZ_OF, GZ_BW) << OFFSET_INT32_13) >> OFFSET_INT32_16;
        
        // NRF_LOG_INFO("gyro:z %x :%x", (int)(imudata[2]*1000.0f),(int)(g2*1000.0f));

         NRF_LOG_INFO("gyro_1 0x%x ,ori  0x%x", gz&0x1fff,pimu->g[2]&0x1fff);
         NRF_LOG_INFO("gyro_1 0x%x ,ori  0x%x", gz&0x1fff,pimu->g[2]&0x1fff);

        
         //  NRF_LOG_INFO("gyro_tr 0x%x", pimu->g[2]);
      }
      logflag++;
    }
#endif

}


void get_imu_hc(uint8_t *tm, imu_packet_t *pimu)
{



}



uint32_t get_entry(uint8_t **data)
{
    *data = gs_data;
    return MAX_DSIZE;
}



uint32_t set_entry(uint8_t *data)
{
    gs_data = data;

    return MAX_DSIZE;

}


