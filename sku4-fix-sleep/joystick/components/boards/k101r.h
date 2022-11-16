/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef K101_H
#define K101_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LEDs definitions for k02
#define LEDS_NUMBER    1

#define LED_START      25
#define LED_1          25
#define LED_STOP       25

#define LEDS_ACTIVE_STATE 1

#define LEDS_INV_MASK  LEDS_MASK

#define LEDS_LIST { LED_1 }

#define BSP_LED_0      LED_1



#define  X_B   23
#define  Y_B   22
#define  FUNC_B  27
//h
//#define  FRONT_B  28
//#define  SIDE_B  29
#define  MID_B   24

////
#define  UP_B   
#define  DOWN_B 
#define  LEFT_B 
#define  RIGHT_B

#define  SENS1_B // on the stick
#define  SENS2_B // front 


#define BUTTONS_NUMBER 4


#define BUTTON_1       X_B
#define BUTTON_2       Y_B
#define BUTTON_3       FUNC_B
#define BUTTON_4       MID_B // 

//#define BUTTON_5       FRONT_B
//#define BUTTON_6       SIDE_B


#define BUTTON_START   BUTTON_1
#define BUTTON_STOP    BUTTON_4



#define BUTTONS_ACTIVE_STATE 1

#define BUTTONS_LIST { BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4 }

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_2
#define BSP_BUTTON_2   BUTTON_3
#define BSP_BUTTON_3   BUTTON_4//
//#define BSP_BUTTON_4   BUTTON_5
//#define BSP_BUTTON_5   BUTTON_6

#define BUTTON_PULL    NRF_GPIO_PIN_NOPULL


#define BUTTON_PULL0    NRF_GPIO_PIN_PULLDOWN
#define BUTTON_PULL1    NRF_GPIO_PIN_PULLDOWN
#define BUTTON_PULL2    NRF_GPIO_PIN_PULLDOWN
#define BUTTON_PULL3    NRF_GPIO_PIN_PULLUP
//#define BUTTON_PULL4    NRF_GPIO_PIN_NOPULL
//#define BUTTON_PULL5    NRF_GPIO_PIN_NOPULL


#define  BSP_BUTTON_0_AC  APP_BUTTON_ACTIVE_HIGH
#define  BSP_BUTTON_1_AC  APP_BUTTON_ACTIVE_HIGH

#define  BSP_BUTTON_2_AC  APP_BUTTON_ACTIVE_HIGH
#define  BSP_BUTTON_3_AC  APP_BUTTON_ACTIVE_LOW

//#define  BSP_BUTTON_4_AC  APP_BUTTON_ACTIVE_LOW
//#define  BSP_BUTTON_5_AC  APP_BUTTON_ACTIVE_LOW


#define HWFC           true

#ifdef __cplusplus
}
#endif

#endif // K02_H
