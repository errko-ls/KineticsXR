
/*********************************************

shadowCreate controler .
v0.1. create        20191111           zk.xu


********************************************/


#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_drv_timer.h"
#include "sys.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "user_config.h"


#include "nrf_rng.h"
//#include "hal_rng.h"
//#include "sys_utils.h"



const  nrf_drv_timer_t  gTIMER_TICK = NRF_DRV_TIMER_INSTANCE(4);


#define TIME_CC_CNT    TIME_CNT

#define TICK_TIMER_CONFIG                                                    \
{                                                                                    \
    .frequency          = (nrf_timer_frequency_t)NRF_TIMER_FREQ_1MHz,\
    .mode               = (nrf_timer_mode_t)NRF_TIMER_MODE_TIMER,          \
    .bit_width          = (nrf_timer_bit_width_t)NRF_TIMER_BIT_WIDTH_32,\
    .interrupt_priority = 1,                    \
    .p_context          = NULL                                                       \
}

static sys_tick_cb gtickcb = NULL;

uint32_t gSysTick = 0;



static IRQn_Type thisirq;

// maybe this will more accurate ---test
void sys_clocks_switch(void)
{

    // start external crystal : K02 : 32M ,and  hFCLK :64M .
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    {
        // Do nothing while waiting for the clock to start
    }

}

void sys_tick_irq(nrf_timer_event_t event_type, void *p_context)
{
    if (event_type == NRF_TIMER_EVENT_COMPARE0)
    {

        gSysTick++;

        if (gtickcb)gtickcb(gSysTick);

    }
}

void sys_tick_init(void)
{
    uint32_t err_code = NRF_SUCCESS;

    nrf_drv_timer_config_t timer_cfg = TICK_TIMER_CONFIG;
    err_code = nrf_drv_timer_init(&gTIMER_TICK, &timer_cfg, sys_tick_irq);
    APP_ERROR_CHECK(err_code);
    nrf_drv_timer_extended_compare(&gTIMER_TICK, NRF_TIMER_CC_CHANNEL0, TIME_CC_CNT, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    thisirq = nrfx_get_irq_number(gTIMER_TICK.p_reg);
    gSysTick = 0;


}



void  sys_tick_irqdisable(void)
{
    NRFX_IRQ_DISABLE(thisirq);
}

void  sys_tick_irqenable(void)
{
    NRFX_IRQ_ENABLE(thisirq);
}


void sys_tick_get_unlock(uint32_t *tick)
{
    *tick = gSysTick;
}



void sys_tick_start(void)
{
    gSysTick = 0;
    nrf_drv_timer_enable(&gTIMER_TICK);

    NRF_LOG_WARNING("systick start");
}

void sys_tick_stop(void)
{
    nrf_drv_timer_disable(&gTIMER_TICK);
}

void sys_tick_unint(void)
{
    nrf_drv_timer_uninit(&gTIMER_TICK);
    
    NRF_LOG_INFO("sys_tick_unint");
}


void sys_tick_reg_cb(sys_tick_cb cb)
{

    gtickcb = cb;

}

uint32_t sys_tick_get_dur(void)
{
    return TIME_CC_CNT;
}

static void internal_error_process(void)
{
    NRF_LOG_FINAL_FLUSH();

    nrf_delay_ms(100);

    NVIC_SystemReset();
}


void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    NRF_LOG_ERROR("app_error_handler err_code:%d %s:%d", error_code, p_file_name, line_num);
    
    internal_error_process();	
}


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_ERROR("app_error_fault_handler id: 0x%08x, pc: 0x%08x, info: 0x%08x", id, pc, info);

    internal_error_process();	
}


void app_error_handler_bare(uint32_t error_code)
{
    NRF_LOG_ERROR("app_error_handler_bare: 0x%08x!", error_code);

    internal_error_process();	
}
																 
			

#if 1
// 0~255 -->   0~6
uint8_t sys_rand_get(void)
{
    uint8_t rand;

    nrf_rng_task_trigger(NRF_RNG_TASK_START);
    while (!nrf_rng_event_get(NRF_RNG_EVENT_VALRDY));
    nrf_rng_task_trigger(NRF_RNG_TASK_STOP);
    nrf_rng_event_clear(NRF_RNG_EVENT_VALRDY);

    rand = nrf_rng_random_value_get();

    rand = rand / 37;

    if (rand > 6)rand = 6;

    return rand;
}
#endif
