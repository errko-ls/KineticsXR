/*******************************
shadowCreate controler .
v0.1.   simple sm for imu use it , todo : modify  for mult instance .
v0.2.  add PKT_USER_SIZE
******************************/


#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_drv_timer.h"
#include "sys.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_delay.h"


#include "nrf_log_default_backends.h"
#include "user_config.h"

#include "d_packet.h"

#ifdef PKT_USER_SIZE

    #ifdef HEADER_SIZE
        #undef HEADER_SIZE
    #endif

    #define HEADER_SIZE  (sizeof(uint32_t)+sizeof(uint8_t))
#endif


cp_buffer_t u_packet;


void d_packet_init(void)
{
    u_packet.c_head = -1;
    u_packet.p_head = -1;

}

void d_packet_add(d_packet_t *pcur)
{
    int8_t p_head;
    d_packet_t *pdata;
    p_head = (u_packet.p_head + 1) % MAX_PACK_SIZE;
    pdata = &u_packet.data[p_head];
#ifdef PKT_USER_SIZE
    memcpy((void *)pdata, (void *)pcur, pcur->size + HEADER_SIZE);
#else
    memcpy((void *)pdata, (void *)pcur, sizeof(d_packet_t));
#endif
    u_packet.p_head = p_head;
}

void d_packet_init_sync(void)
{
    int8_t p_head;
    d_packet_init();

    p_head = u_packet.p_head;

    while (p_head < 0)
    {
        nrf_delay_us(100);
        p_head = u_packet.p_head;
    }
}

int d_packet_get_next(d_packet_t *pcur)
{
    int8_t p_head;
    d_packet_t *pdata;
    int8_t cidx;


    p_head = u_packet.p_head;
    if (p_head < 0)return -1;

    if (u_packet.c_head == u_packet.p_head)return -2;


    cidx = (u_packet.c_head + 1) % MAX_PACK_SIZE;
    pdata = &u_packet.data[cidx];

#ifdef PKT_USER_SIZE
    memcpy((void *)pcur, (void *)pdata, pdata->size + HEADER_SIZE);
#else
    memcpy((void *)pcur, (void *)pdata, sizeof(d_packet_t));
#endif
    u_packet.c_head = cidx;
    return 0;
}

#ifdef PKT_USER_SIZE


int d_packet_get_next_usr(void *udata, uint32_t *pts)
{
    int8_t p_head;
    d_packet_t *pdata;
    int8_t cidx;


    p_head = u_packet.p_head;

    if (p_head < 0)return -1;

    if (u_packet.c_head == u_packet.p_head)return -2;


    cidx = (u_packet.c_head + 1) % MAX_PACK_SIZE;
    pdata = &u_packet.data[cidx];
    *pts = pdata->ts;
    memcpy((void *)udata, (char *)pdata + HEADER_SIZE, pdata->size);
    u_packet.c_head = cidx;
    //return 0;
    return pdata->size;
}

#endif

#ifdef USE_LA_RCOV

// zzzzkkkkkkkkkkk  always check size befor call this fun

void d_packet_get_nexta(int16_t *bufa)
{
    d_packet_t *pdata;
    int8_t cidx;

    cidx = (u_packet.c_head + 1) % MAX_PACK_SIZE;
    pdata = &u_packet.data[cidx];

    //d_packet_t *pcur= (d_packet_t *)pdata->data;

    memcpy((void *)bufa, (char *)pdata + OFFSET_A_OF_USRPKT, 6);

    u_packet.c_head = cidx;

}

#endif



void  d_packet_skip_next(void)
{
#if 0  // zzzzkkkkkkkkkkk  always check size befor call this fun
    int8_t p_head;
    p_head = u_packet.p_head;

    if (p_head < 0)return -1;
    if (u_packet.c_head == u_packet.p_head)return -2;
#endif

    u_packet.c_head = (u_packet.c_head + 1) % MAX_PACK_SIZE;

    //   return 0;
}



int d_packet_get_latest(d_packet_t *pcur)
{
    int8_t p_head;
    int8_t c_head;
    d_packet_t *pdata;
    int8_t cidx;

    c_head = u_packet.c_head;
    p_head = u_packet.p_head;

    if (p_head < 0)return -1;

    if (c_head == p_head)return -2;

    cidx = u_packet.p_head;

    pdata = &u_packet.data[cidx];

#ifdef PKT_USER_SIZE
    memcpy((void *)pcur, (void *)pdata, pdata->size + HEADER_SIZE);
#else
    memcpy((void *)pcur, (void *)pdata, sizeof(d_packet_t));
#endif
    u_packet.c_head = cidx;
    return 0;

}

int d_packet_get_size(void)
{
    int8_t p_head = u_packet.p_head;
    int8_t c_head = u_packet.c_head;

    if (c_head == p_head)return 0;

    if (p_head < 0)return 0;

    if (p_head > c_head)
    {
        return p_head - c_head;
    }
    else if (p_head < c_head)
    {
        return (MAX_PACK_SIZE - c_head + p_head);

    }
    return 0;
}


