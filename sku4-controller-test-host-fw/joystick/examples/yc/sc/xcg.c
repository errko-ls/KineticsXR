/*******************************
shadowCreate controler .
v0.1. create        20191201     zk.xu

******************************/
// main program  to xchange data with kernel 

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_delay.h"
#include <stdbool.h>
#include "nrf.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "boards.h"
#include "nrf_drv_spis.h"
#include "app_timer.h"

#include "xcg.h"
#include "sys.h"
#include "d_packet.h"

#include "m_rf.h"
#include "persist.h"
#include "rf0.h"
#include "rled_ctl.h"
#include "user_config.h"

#define SPI_M2
//#define  STORE_PIN  11
//#define  STOREIR_PIN  12

#define  HINT_PIN  7
#define APP_SPIS_CS_PIN 5
#define APP_SPIS_MISO_PIN 2
#define APP_SPIS_MOSI_PIN 3
#define APP_SPIS_SCK_PIN 4

// TODO:

static const nrf_drv_spis_t spis = NRF_DRV_SPIS_INSTANCE(0);
static volatile bool spis_xfer_done;
static volatile bool spis_transfer_done; 
static volatile bool host_flag;

volatile bool predictset=false;

static uint32_t glastSpicomletetick;

APP_TIMER_DEF(xcg_t_id);
APP_TIMER_DEF(spi_transfer_id);

static int xcg_time_ms = 300;
static uint32_t xcg_time_tick = APP_TIMER_TICKS(300);
static uint32_t spi_transfer_time_tick = APP_TIMER_TICKS(300); //for spi transfer

static uint8_t enterDfuMark = 0;

#define FW_VERSION (0x0102)

#define CMD_DATA  0xA6
#define CMD_CLR_BOND  0xA7
#define CMD_REQUEST_TAG   (0xA8)
#define CMD_EXTDATA_RLEDTAG  0xB6

#define PACKET_SIZE 30
#define MAX_SND_SIZE 6
#define SEND_FIX_SIZE ((PACKET_SIZE*MAX_SND_SIZE)+10)


volatile uint8_t  gleden = 0;
volatile uint32_t gvib = 0;

unsigned char  bond_info;

static uint8_t gxcgbuffer[255];
static uint8_t gxcgbufferout[255];
static uint8_t ackHost = 0; //Notify the host by controlling the interrupt pin to get the reply
volatile uint32_t auto_sleep_mode = 0;
volatile uint32_t send_data_mode = 0;


void needAckHost(uint8_t enable)
{
	if (enable)
		ackHost = 1;
	else
		ackHost = 0;
}

void xcg_handler(void *p_context);

void spis_event_handler(nrf_drv_spis_event_t event)
{
    if (event.evt_type == NRF_DRV_SPIS_XFER_DONE)
    {
        //  if(host_flag){
        //nrf_gpio_pin_clear(HINT_PIN);
        //  }
        nrf_gpio_pin_clear(HINT_PIN);
        spis_xfer_done = true;
        //NRF_LOG_INFO(" Transfer completed");
        //NRF_LOG_FLUSH();
    }
    else if (event.evt_type == NRF_DRV_SPIS_BUFFERS_SET_DONE)
    {
        nrf_gpio_pin_set(HINT_PIN);
    }

}

void xcg_init(void)
{
    ret_code_t err_code;
    nrf_drv_spis_config_t spis_config = NRF_DRV_SPIS_DEFAULT_CONFIG;

    //nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    //err_code = nrf_drv_gpiote_out_init(HINT_PIN, &out_config);
    //APP_ERROR_CHECK(err_code);
    nrf_gpio_cfg_output(HINT_PIN);



    nrf_gpio_pin_clear(HINT_PIN);

    // TODO:   TOBE CONFIG
    spis_config.csn_pin               = APP_SPIS_CS_PIN;
    spis_config.miso_pin              = APP_SPIS_MISO_PIN;
    spis_config.mosi_pin              = APP_SPIS_MOSI_PIN;
    spis_config.sck_pin               = APP_SPIS_SCK_PIN;
    spis_config.mode                  = NRF_SPIS_MODE_0;
    spis_config.bit_order             = NRF_SPIS_BIT_ORDER_MSB_FIRST;
    spis_config.csn_pullup            = NRFX_SPIS_DEFAULT_CSN_PULLUP;

    APP_ERROR_CHECK(nrf_drv_spis_init(&spis, &spis_config, spis_event_handler));
    NRF_LOG_WARNING("xcg_init");

    NRF_LOG_FLUSH();


    // nrf_gpio_cfg_input(STOREIR_PIN,GPIO_PIN_CNF_PULL_Disabled);
    //  nrf_gpio_cfg_input(STORE_PIN,GPIO_PIN_CNF_PULL_Disabled);



    err_code = app_timer_create(&xcg_t_id, APP_TIMER_MODE_SINGLE_SHOT, xcg_handler);
    APP_ERROR_CHECK(err_code);



}


void xcg_ap_time_calibration(void)
{


}




void xcg_deinit(void)
{

    app_timer_stop(xcg_t_id);

    nrf_drv_spis_uninit(&spis);
}

#if 0
static void   tk_cb(uint32_t ts)
{
      if(predictset){
           predictset=false;
           nrf_gpio_pin_set(HINT_PIN);
       }
                
}
#endif

void xcg_start(void)
{
    ret_code_t err_code = 0;
    err_code = app_timer_start(xcg_t_id, xcg_time_tick, NULL);    
   // sys_tick_reg_cb(tk_cb);
    APP_ERROR_CHECK(err_code);

}

extern uint32_t gAploss;

static int cdtick = 0;



void xcg_handler(void *p_context)
{
    int size;
    //uint8_t logsize;
    int cnt = 0;

    uint8_t *pbuff;
    uint32_t pts;



    size = d_packet_get_size();

    if ((xcg_time_ms == XCG_T1) && (size == 0))
    {
        //app_timer_stop(xcg_t_id);

        cdtick++;
        if (cdtick == 300)
        {
            cdtick = 0;
        }
        else
        {
            app_timer_start(xcg_t_id, 6, NULL); //5/33
            goto __end;
        }
    }

    if (size) cdtick = 0;


    xcg_start();

    while (size > 10)
    {
        d_packet_skip_next();
        size = d_packet_get_size();
        gAploss++;
        //NRF_LOG_WARNING("discard");
    }

    //logsize = size;

    //gxcgbuffer[0] = FW_VERSION;
    gxcgbuffer[1] = rf_get_bond_info();
    gxcgbuffer[3] = PACKET_SIZE;

    pbuff = &gxcgbuffer[8];

    while (size > 0)
    {
        d_packet_get_next_usr((void *)(pbuff + 4), &pts);

#ifdef DIV_CNT_SHIFT
        pts = pts >> DIV_CNT_SHIFT;
#endif
        memcpy(pbuff, (void *)&pts, 4);
        pbuff += PACKET_SIZE;
        size--;
        cnt++;
        if (cnt == MAX_SND_SIZE)
            break;
    }
    gxcgbuffer[2] = cnt;
    sys_tick_get_unlock(&glastSpicomletetick);
#ifdef DIV_CNT_SHIFT
    glastSpicomletetick = glastSpicomletetick >> DIV_CNT_SHIFT;
#endif
    memcpy(&gxcgbuffer[4], (void *)&glastSpicomletetick, 4);
    spis_xfer_done = false;
    //nrf_gpio_pin_set(HINT_PIN);

    //predictset=true;
    nrf_drv_spis_buffers_set(&spis, gxcgbuffer, SEND_FIX_SIZE, gxcgbufferout, SEND_FIX_SIZE);
    while (!spis_xfer_done)
    {
        __WFE();
    }

    if (gxcgbufferout[0] == CMD_DATA)
    {
        //uint8_t led = (gxcgbufferout[1] >> 4) & 0x0f;
       
        gvib = ((gxcgbufferout[3] << 8) | gxcgbufferout[2]);
        //NRF_LOG_WARNING("gvib:%x",gvib);
        
    }
    else if (gxcgbufferout[0] == CMD_CLR_BOND)
    {
        rf_clear_bond_info();
    }
    nrf_delay_us(1);

    //NRF_LOG_WARNING("stir=%d",nrf_gpio_pin_read(STORE_PIN));

__end:
    //xcg_start();

    //NRF_LOG_WARNING(" snd=%d left=%d bond=%d,dt=%d",cnt,logsize-cnt,rf_get_bond_info(), glastSpicomletetick);
    //NRF_LOG_FLUSH();
    return;
}


void spis_bondEvent_handler(nrf_drv_spis_event_t event)
{
    if (event.evt_type == NRF_DRV_SPIS_XFER_DONE)
    {
    	if(ackHost)
    	{
        	nrf_gpio_pin_clear(HINT_PIN);
    	}
		spis_transfer_done = true;
    }
    else if (event.evt_type == NRF_DRV_SPIS_BUFFERS_SET_DONE)
    {
    	if (ackHost) 
    	{
	    	nrf_gpio_pin_set(HINT_PIN);
    	}
    }
}

void sc_spiSlave_init(void)
{
	nrf_drv_spis_config_t spis_config = NRF_DRV_SPIS_DEFAULT_CONFIG;

	nrf_gpio_cfg_output(HINT_PIN);
    nrf_gpio_pin_clear(HINT_PIN);

    // TODO:   TOBE CONFIG
    spis_config.csn_pin               = APP_SPIS_CS_PIN;
    spis_config.miso_pin              = APP_SPIS_MISO_PIN;
    spis_config.mosi_pin              = APP_SPIS_MOSI_PIN;
    spis_config.sck_pin               = APP_SPIS_SCK_PIN;
    spis_config.mode                  = NRF_SPIS_MODE_0;
    spis_config.bit_order             = NRF_SPIS_BIT_ORDER_MSB_FIRST;
    spis_config.csn_pullup            = NRFX_SPIS_DEFAULT_CSN_PULLUP;

    APP_ERROR_CHECK(nrf_drv_spis_init(&spis, &spis_config, spis_bondEvent_handler));
}

static int sc_spi_transfer(void)
{
	int ret;
	uint8_t txBuffer[SEND_FIX_SIZE] = {0};
	uint8_t PackagingInputBuffer[SEND_FIX_SIZE] = {0};
	uint8_t rxBuffer[SEND_FIX_SIZE] = {0};
	request_t current_request = {0};
	static request_t last_request = {0};
	static acknowledge_t requestAck = {0};
	uint8_t joyStickID = 0;

	memset(PackagingInputBuffer, 0, sizeof(PackagingInputBuffer));
	if(last_request.requestHead.needAck) //The first four bytes of input buffer
	{
		memcpy(PackagingInputBuffer, (uint8_t *)&requestAck, sizeof(requestAck));
		memset(&last_request, 0, sizeof(last_request));
		memset(&requestAck, 0, sizeof(requestAck));
		ackHost = 1;
	}
	else
	{
		ackHost = 0;
	}

	if(gInfo.left_ok || gInfo.right_ok)
	{
		int size;
	    int cnt = 0;

	    uint8_t *pbuff;
	    uint32_t pts;

	    size = d_packet_get_size();	 
	    while (size > 10)
	    {
	        d_packet_skip_next();
	        size = d_packet_get_size();
	        gAploss++;
	        //NRF_LOG_WARNING("discard");
	    }

	    PackagingInputBuffer[4] = PACKET_SIZE;
	    pbuff = &PackagingInputBuffer[10];
		ackHost = 1;
	
	    while (size > 0)
	    {
	        d_packet_get_next_usr((void *)(pbuff + 4), &pts);
#ifdef DIV_CNT_SHIFT
	        pts = pts >> DIV_CNT_SHIFT;
#endif
	        memcpy(pbuff, (void *)&pts, 4);
	        pbuff += PACKET_SIZE;
	        size--;
	        cnt++;
	        if (cnt == MAX_SND_SIZE)
	            break;
	    }
		
	    PackagingInputBuffer[5] = cnt;

	    sys_tick_get_unlock(&glastSpicomletetick);
#ifdef DIV_CNT_SHIFT
	    glastSpicomletetick = glastSpicomletetick >> DIV_CNT_SHIFT;
#endif
	    memcpy(&PackagingInputBuffer[6], (void *)&glastSpicomletetick, 4);			
	}
	spis_transfer_done = false;
	memcpy(txBuffer, PackagingInputBuffer, sizeof(PackagingInputBuffer));
	ret = nrf_drv_spis_buffers_set(&spis, txBuffer, SEND_FIX_SIZE, rxBuffer, SEND_FIX_SIZE);
    while (!spis_transfer_done)
    {
        __WFE();
    }

	if(enterDfuMark == 1) {
		NRF_POWER->GPREGRET = 0xB1;
		nrf_delay_ms(50);
		NVIC_SystemReset();
		NRF_LOG_WARNING("enterDfuMark:%d\n", enterDfuMark);
	}


	memset(&current_request, 0, sizeof(request_t));
	if (rxBuffer[0] == CMD_REQUEST_TAG)
	{
		current_request.requestHead.needAck = ((rxBuffer[1]&0x80)>>7);
		current_request.requestHead.requestType = (rxBuffer[1]&0x7f);
		current_request.requestData[0] = rxBuffer[2];
		current_request.requestData[1] = rxBuffer[3];
		current_request.requestData[2] = rxBuffer[4];

		NRF_LOG_WARNING("requestType:%d\n", current_request.requestHead.requestType);
		switch(current_request.requestHead.requestType)
		{
			case getMasterNordicVersionRequest:
				requestAck.acknowledgeData[0] = (FW_VERSION&0xff);
				requestAck.acknowledgeData[1] = ((FW_VERSION&0xff00) >> 8);
			NRF_LOG_WARNING("FW_VERSION:0x%x\n", FW_VERSION);
				break;
			case setVibStateRequest:
				gvib = ((current_request.requestData[1] << 8) | current_request.requestData[0]);
				//NRF_LOG_WARNING("setVibStateRequest gvib:0x%x\n", gvib);
				break;
			case bondJoyStickRequest:	
				joyStickID = (current_request.requestData[0]&0x01);
				if ((!gInfo.left_ok) || (!gInfo.right_ok))
					bondSpecifiedJoyStick(joyStickID);
				break;
			case disconnectJoyStickRequest:
				joyStickID = (current_request.requestData[0]&0x01);
				if (gInfo.left_ok || gInfo.right_ok)
				{
					ackHost = 0;
					rf_stop_data();
					clear_SpecifiedJoyStick_bondInfo(joyStickID);
				}
				break;
			case getLeftJoyStickProductNameRequest:
				if (gInfo.left_ok)
				{
					requestAck.acknowledgeData[0] = gLeftjoyStick_productNameID;
				}
				break;
			case getRightJoyStickProductNameRequest:
				if (gInfo.right_ok)
				{
					requestAck.acknowledgeData[0] = gRightjoyStick_productNameID;
				}
				break;
			case getLeftJoyStickFwVersionRequest:
				if (gInfo.left_ok)
				{
					requestAck.acknowledgeData[0] = (gLeftjoyStick_fwVersion&0xff);
					requestAck.acknowledgeData[1] = ((gLeftjoyStick_fwVersion&0xff00) >> 8);
				}
				break;
			case getRightJoyStickFwVersionRequest:
				if (gInfo.right_ok)
				{
					requestAck.acknowledgeData[0] = (gRightjoyStick_fwVersion&0xff);
					requestAck.acknowledgeData[1] = ((gRightjoyStick_fwVersion&0xff00) >> 8);
				}
				break;
			case getJoyStickBondStateRequest:
				//NRF_LOG_WARNING("getJoyStickBondStateRequest:%d", gInfo.left_ok|(gInfo.right_ok << 1));
				requestAck.acknowledgeData[0] = (gInfo.left_ok | (gInfo.right_ok << 1));
				break;
			case hostEnterDfuStateRequest:
				requestAck.acknowledgeData[0] = 6;
				enterDfuMark = 1;
				NRF_LOG_INFO("hostEnterDfuStateRequest\r\n");
				break;
			case setControllerSleepMode:
				auto_sleep_mode = current_request.requestData[0];
				send_data_mode  = current_request.requestData[1];
				NRF_LOG_INFO("nordic audo_sleep_mode 0x%x send_data_mode 0x%x\r\n",auto_sleep_mode,send_data_mode);
				break;
		}
		if (current_request.requestHead.needAck == 1)
		{
			if ((current_request.requestHead.requestType >= invalidRequest) || (current_request.requestHead.requestType == 0))
				requestAck.acknowledgeHead.ack = 0;
			else
				requestAck.acknowledgeHead.ack = 1;			
		}
		else
		{
			requestAck.acknowledgeHead.ack = 0;
		}
		requestAck.acknowledgeHead.requestType = current_request.requestHead.requestType; 
		memcpy((uint8_t *)&last_request, (uint8_t *)&current_request, sizeof(request_t));
		//NRF_LOG_FLUSH();
	}
	
	return ret;
}

static void spi_transfer_handler(void *p_context)
{	
	sc_spi_transfer_timer_start();

	sc_spi_transfer();
}

int sc_spi_transfer_timer_init(void)
{	
	ret_code_t err_code;
	
	err_code = app_timer_create(&spi_transfer_id, APP_TIMER_MODE_SINGLE_SHOT, spi_transfer_handler);
    APP_ERROR_CHECK(err_code);

	return err_code;
}

int sc_spi_transfer_timer_start(void)
{
    ret_code_t err_code = 0;
	
    err_code = app_timer_start(spi_transfer_id, spi_transfer_time_tick, NULL);    
    APP_ERROR_CHECK(err_code);

	return err_code;
}

void xcg_set_freq(int i)
{
    xcg_time_ms = i;
    xcg_time_tick = APP_TIMER_TICKS(xcg_time_ms)- 8;//8
	spi_transfer_time_tick = APP_TIMER_TICKS(xcg_time_ms)- 8;//8
    //NRF_LOG_WARNING("spi_transfer_time_tick=%d", spi_transfer_time_tick);
}


