/*******************************
shadowCreate controler .

v0.1. create        20191111     zk.xu

******************************/

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

#include "app_timer.h"

#include "rled_ctl.h"
#include "nrf_drv_gpiote.h"
#include "user_config.h"
#include "sys.h"

volatile uint32_t  gledval = 0;
extern uint8_t  gleden;

#ifdef USE_V02A
    #define STORBE_IR  12
    #define STORBE_L 9
    #define STORBE_R 6

#else
#ifdef USE_A11B
    #define STORBE_IR  9
    #define STORBE_L 16
    #define STORBE_R 17
    
#else
    #define STORBE_SLAM 11
    #define STORBE_IR  12
    #define STORBE_L 16
    #define STORBE_R 17
#endif
#endif

uint32_t rled_getraw(void)
{
    return gledval;
}

void rled_setraw(uint32_t  v)
{
    gledval = v;

}


void rled_start(void)
{

}
void rled_stop(void)
{

}


static uint32_t gtick;
#ifdef  USE_RLED_PATTEN_10

void rled_set_state(uint8_t state)
{
    //glastState=!glastState;
}

#endif

#ifdef  USE_V02A

void rled_event(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{

    if (pin == STORBE_IR)
    {
        sys_tick_get_unlock(&gtick);
        // rf_set_extdata3(gtick);

#ifdef  USE_RLED_PATTEN_10
        gledval = gtick;
        //NRF_LOG_WARNING("gledval=%d",gledval);
        // NRF_LOG_FLUSH();

        if (nrf_gpio_pin_read(STORBE_L))
            gleden = gleden & 0x02;
        else
            gleden = gleden | 0x01;


        if (nrf_gpio_pin_read(STORBE_R))
           gleden = gleden & 0x01;
        else
           gleden = gleden | 0x02;

        
       //NRF_LOG_WARNING("gledval=%d",gleden);
       
      // NRF_LOG_FLUSH();
#else

    #ifdef USE_RLED_PATTEN_10_METHOD2
        if (nrf_gpio_pin_read(STORBE_SLAM))
            glastState = true;
        else
            glastState = false;

        // NRF_LOG_WARNING("rled state=%d",glastState);
    #else
        glastState = !glastState;
    #endif

        if (glastState)
        {
            gledval = gtick;
        }

        
#endif


    }
}

#else
void rled_event(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{

    if (pin == STORBE_IR)
    {
        sys_tick_get_unlock(&gtick);
        // rf_set_extdata3(gtick);

#ifdef  USE_RLED_PATTEN_10
#ifdef USE_A11B
        gledval = gtick;
        //NRF_LOG_WARNING("gledval=%d",gledval);
        // NRF_LOG_FLUSH();
#else

#ifdef USE_RLED_PATTEN_10_METHOD2
        if (nrf_gpio_pin_read(STORBE_SLAM))
            glastState = true;
        else
            glastState = false;

        // NRF_LOG_WARNING("rled state=%d",glastState);
#else
        glastState = !glastState;
#endif

        if (glastState)
        {
            gledval = gtick;
        }
#endif
#else
        gledval = gtick;
#endif

        //NRF_LOG_WARNING("rled gtick=%d",gtick);
       // NRF_LOG_FLUSH();

#ifdef RL_CTL
er
        if (nrf_gpio_pin_read(STORBE_L))
            gleden = gleden | 0x01;
        else
            gleden = gleden & 0x02;


        if (nrf_gpio_pin_read(STORBE_R))
            gleden = gleden | 0x02;
        else
            gleden = gleden & 0x01;
#else
        gleden =  0x03;
#endif


    }
}

#endif

void rled_init(void)
{
    ret_code_t err_code = 0;
    
#ifdef USE_V02A
    
#elif defined(USE_A11B)

#else
    nrf_gpio_cfg_input(STORBE_SLAM, NRF_GPIO_PIN_NOPULL);
#endif
    nrf_gpio_cfg_input(STORBE_L, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(STORBE_R, NRF_GPIO_PIN_NOPULL);

    nrf_gpio_cfg_input(STORBE_IR, NRF_GPIO_PIN_NOPULL);


    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_config.pull = NRF_GPIO_PIN_NOPULL; //NRF_GPIO_PIN_PULLDOWN;// NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(STORBE_IR, &in_config, rled_event);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(STORBE_IR, true);




}

void rled_deinit(void)
{

    nrf_drv_gpiote_in_event_enable(STORBE_IR, false);


}


