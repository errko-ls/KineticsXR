
#ifndef __SYS_H__
#define __SYS_H__
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_delay.h"


typedef enum ctl_state
{
    CTL_NORMAL_MODE,
    CTL_TEST_MODE,
    CTL_BONDING_MODE,
    CTL_CALIB_MODE,
    CTL_NO_BONDING_CHARGING_MODE,
} ctl_state_t;


typedef void (*sys_tick_cb)(uint32_t ts);

void sys_tick_init(void);
void sys_tick_start(void);
void sys_tick_stop(void); 
void sys_tick_reg_cb(sys_tick_cb cb);

void sys_tick_unint(void);
void sys_clocks_switch(void);
uint32_t sys_tick_get_dur(void);

uint8_t sys_rand_get(void);
void sys_tick_get_unlock(uint32_t *tick);
void  sys_tick_irqdisable(void);
void  sys_tick_irqenable(void);

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info);
void app_error_handler_bare(uint32_t error_code);
#endif
