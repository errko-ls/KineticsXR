
/*********************************************

shadowCreate controler .
v0.1. create        20191111           zk.xu


********************************************/

#ifndef __MRF_H__
#define __MRF_H__

#include "sdk_common.h"
#include "nrf.h"


typedef void (* rf_data_cb)(uint8_t *pdata, uint8_t size, uint8_t idx);
typedef void (* rf_data_start_cb)(void);
typedef void (* rf_data_stop_cb)(void);
typedef void (* rf_req_cb)(uint8_t *pdata, uint8_t size, uint32_t ts);

typedef struct
{
    void        *p_context;
    //uint8_t      gRepair;
    rf_data_cb   data_cb;
    rf_req_cb   req_cb;

    rf_data_start_cb data_start;
    rf_data_stop_cb data_stop;
    uint8_t      randidx;
    uint8_t      rldcfg;

} rf_config;

typedef struct rf_info_t
{

    uint8_t   left_ok;
    uint8_t  right_ok;
    //uint8_t  idx;
    uint32_t left_type;
    uint32_t right_type;

    uint32_t check_bond;

} rf_info_t;

typedef rf_config rf_config_t;
void rf_module_stop(void);
void rf_module_init_and_start(rf_config_t *cfg);
void rf_debug_print(void);
void rf_clear_bond_info(void);
uint8_t rf_get_bond_info(void);

void rf_set_extdata1(uint8_t d);
void rf_set_extdata2(uint8_t d);
void rf_set_extdata3(uint32_t d);
uint32_t bondSpecifiedJoyStick(uint8_t joyStickID);
void clear_SpecifiedJoyStick_bondInfo(uint8_t joyStickID);
void rf_stop_data(void);
#endif
