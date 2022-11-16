/*******************************
shadowCreate controler .
v0.1. init . 20191122  zk.xu

******************************/


#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_esb.h"
#include "nrf_error.h"
#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_delay.h"
#include "app_util.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_drv_common.h"
#include "nrf_drv_timer.h"
#include "app_util_platform.h"
#include "nrf_nvmc.h"
#include "nrf_drv_systick.h"

// yc pravate config

#include "user_config.h"


#include "sys.h"
#include "m_rf.h"
#include "persist.h"
#include "rf0.h"

#define ENABLE_DEBUG



typedef enum rf_state_t
{
    STATE_NONE,
    STATE_CONFIG,   // receive
    STATE_TANING,
    STATE_REQ,    //send TIME/LED
    STATE_DATA,   //receive

} rf_state_t;


//typedef void (* rf_bond_timeout_cb_t)(void);

//static rf_bond_timeout_cb_t   bond_timeout_cb=NULL;

uint8_t gvibl = 0;
uint8_t gvibr = 0;
// uint8_t gvib=0;
uint8_t gledtimming = 0xff;
//---------------------var balow ----------------------

uint8_t current_Ldata = 0;
uint8_t current_Rdata = 0;
uint8_t last_Ldata = 0;
uint8_t last_Rdata = 0;
uint8_t status = 1;


#define  CUST_RF_TIMER 1
#define  CUST_RF_TIMER_PRESCAL 4
#define  CUST_RF_TIMER_BIT 3
#define  CUST_RF_TIMER_PRIORITY 5

#define  CUST_RF_TIMER_CNT_BOND 8000//

#define  CUST_BOND_TIMER_S_ONE_CTL   30//60//12
#define  CUST_S_CNT   (CUST_BOND_TIMER_S_ONE_CTL*125)


/*select the chanel, and should not be used for data chanel*/
#define  CUST_CHANNEL_NUM_4_BOND     7
static uint8_t gChanel4bond[CUST_CHANNEL_NUM_4_BOND] = {11, 21, 31, 41, 51, 61, 71};

#define  CUST_CHANNEL_NUM_4_DATA     7


static uint8_t gChanel4data[7][CUST_CHANNEL_NUM_4_DATA] = {{12, 22, 32, 42, 52, 62, 72}, {13, 23, 33, 43, 53, 63, 73}, {14, 24, 34, 44, 54, 64, 74}, {15, 25, 35, 45, 55, 65, 75}, {16, 26, 36, 46, 56, 66, 76}, {17, 27, 37, 47, 57, 67, 77}, {18, 28, 38, 48, 58, 68, 78}};




// 1~79
#define  CUST_DATA_CHANNEL_ALL_NUM   33
#define  CUST_DATA_CHANNEL_USE_NUM   3


uint8_t  chaneltodolist;

//-------------------fix below----------------------
const  nrf_drv_timer_t  gEsbCommunicationTimerInstance = NRF_DRV_TIMER_INSTANCE(1);
const  nrf_drv_timer_t  gBondTimerInstance = NRF_DRV_TIMER_INSTANCE(2);

static uint8_t gRF_ID[4] = {0xff, 0xff, 0xff, 0xff};

/*shadow creator  address all device bond   */
uint8_t g_ID0[4] =   {0x77, 0x66, 0x11, 0x17};
/*shadow creator  address all device data  */
uint8_t g_ID1[4] =   {0x28, 0x29, 0x30, 0x31};

uint8_t gPrefixes[3] = {0x11, 0x22, 0x33};
rf_info_t gInfo;

extern uint32_t gSysTick;

static uint32_t gCurTick;
static uint32_t gReqTick;

//static uint32_t gBondCnt;
uint32_t gLeftjoyStick_fwVersion = 0;
uint32_t gRightjoyStick_fwVersion = 0;
uint32_t gLeftjoyStick_productNameID = 0;
uint32_t gRightjoyStick_productNameID = 0;

static uint32_t gCurTimerCnt = CUST_RF_TIMER_CNT_DATA;
static nrfx_timer_event_handler_t gCurTimerhandler;
static nrf_esb_event_handler_t gCurEventCb;

static uint8_t gCurrentChanelIndex = 0;
static uint8_t gCurrentChanelmax = CUST_CHANNEL_NUM_4_BOND;
static uint8_t *gPCurrentChanel;
static rf_state_t gCurRfstate;
static uint8_t   *gCurID;
static uint8_t grequestBondHandleID = 0;
static rf_data_cb gDataCallback = NULL;
static rf_data_start_cb gDatastartCallback = NULL;
static  rf_req_cb  gReqCallback = NULL;

static uint8_t gDataChannelGroupID; //Randomly switch data transmission channels group

static nrf_esb_payload_t gRxPayload;
static nrf_esb_payload_t gTxPayload;


#define RF_TIMER_CONFIG                                                    \
{                                                                                    \
    .frequency          = (nrf_timer_frequency_t)CUST_RF_TIMER_PRESCAL,\
    .mode               = (nrf_timer_mode_t)NRFX_TIMER_DEFAULT_CONFIG_MODE,          \
    .bit_width          = (nrf_timer_bit_width_t)CUST_RF_TIMER_BIT,\
    .interrupt_priority = CUST_RF_TIMER_PRIORITY,                    \
    .p_context          = NULL                                                       \
}



#define NRF_ESB_BOND_CONFIG {.protocol                  = NRF_ESB_PROTOCOL_ESB_DPL,         \
                                .mode                   = NRF_ESB_MODE_PRX,                 \
                                .event_handler          = 0,                                \
                                .bitrate                = NRF_ESB_BITRATE_2MBPS,            \
                                .crc                    = NRF_ESB_CRC_16BIT,                \
                                .tx_output_power        = NRF_ESB_TX_POWER_4DBM,            \
                                .retransmit_delay       = 250,                              \
                                .retransmit_count       = 0,                                \
                                .tx_mode                = NRF_ESB_TXMODE_AUTO,              \
                                .radio_irq_priority     = 1,                                \
                                .event_irq_priority     = 2,                                \
                                .payload_length         = 32,                               \
                                .selective_auto_ack     = true                             \
}


#define NRF_ESB_DATA_RX_CONFIG {.protocol                  = NRF_ESB_PROTOCOL_ESB_DPL,         \
                                .mode                   = NRF_ESB_MODE_PRX,                 \
                                .event_handler          = 0,                                \
                                .bitrate                = NRF_ESB_BITRATE_2MBPS,            \
                                .crc                    = NRF_ESB_CRC_16BIT,                \
                                .tx_output_power        = NRF_ESB_TX_POWER_4DBM,            \
                                .retransmit_delay       = 250,                              \
                                .retransmit_count       = 0,                                \
                                .tx_mode                = NRF_ESB_TXMODE_AUTO,              \
                                .radio_irq_priority     = 1,                                \
                                .event_irq_priority     = 2,                                \
                                .payload_length         = 32,                               \
                                .selective_auto_ack     = true                             \
}

#define NRF_ESB_DATA_TX_CONFIG {.protocol                  = NRF_ESB_PROTOCOL_ESB_DPL,         \
                                .mode                   = NRF_ESB_MODE_PTX,                 \
                                .event_handler          = 0,                                \
                                .bitrate                = NRF_ESB_BITRATE_2MBPS,            \
                                .crc                    = NRF_ESB_CRC_16BIT,                \
                                .tx_output_power        = NRF_ESB_TX_POWER_4DBM,            \
                                .retransmit_delay       = 250,                              \
                                .retransmit_count       = 0,                                \
                                .tx_mode                = NRF_ESB_TXMODE_AUTO,              \
                                .radio_irq_priority     = 1,                                \
                                .event_irq_priority     = 2,                                \
                                .payload_length         = 32,                               \
                                .selective_auto_ack     = true                             \
}


static nrf_esb_config_t gEsbDataRXconfig = NRF_ESB_DATA_RX_CONFIG;
static nrf_esb_config_t gEsbDataTXconfig = NRF_ESB_DATA_TX_CONFIG;
static nrf_esb_config_t gEsbDataBondconfig = NRF_ESB_BOND_CONFIG;



/*debug use*/
static uint32_t gBondReqRecvCNTL = 0;
static uint32_t gBondReqRecvCNTR = 0;

static uint32_t gDataReqSendCnt = 0;
static uint32_t gDataRecvCntL = 0;
static uint32_t gDataRecvCntR = 0;

static uint8_t gLastsendL = 0;
static uint8_t gLastsendR = 0;

#define rf_is_timer_started  nrf_drv_timer_is_enabled(&gEsbCommunicationTimerInstance)
#define rf_timer_start      nrf_drv_timer_enable(&gEsbCommunicationTimerInstance)
#define rf_timer_stop       nrf_drv_timer_disable(&gEsbCommunicationTimerInstance)
#define rf_timer_resume     nrf_drv_timer_resume(&gEsbCommunicationTimerInstance)
#define rf_timer_clear      nrf_drv_timer_clear(&gEsbCommunicationTimerInstance)
#define rf_timer_pause      nrf_drv_timer_pause(&gEsbCommunicationTimerInstance)

#define rf_timer_set_cpmpare     nrf_drv_timer_extended_compare(&gEsbCommunicationTimerInstance, \
                                 NRF_TIMER_CC_CHANNEL0,gCurTimerCnt,\
                                 NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true)
#define rf_timer_uninit      nrf_drv_timer_uninit(&gEsbCommunicationTimerInstance);


static void rf_data_req(void);
void rf_stop_data(void);
static void rf_start_data(void);
void rf_data_event(nrf_esb_evt_t const *p_event);
void rf_test_event(nrf_esb_evt_t const *p_event);
static uint32_t rf_change_address(uint8_t *add0, uint8_t *add1, uint8_t *prefixes, uint8_t cnt);


ret_code_t rf_save_bond_left(uint32_t joyStickFwVersion, uint32_t productID);
ret_code_t rf_save_bond_right(uint32_t joyStickFwVersion, uint32_t productID);
extern void needAckHost(uint8_t enable);

static void rf_change_ramup(void)
{
    // set fastmode// where to set 。zk.xu ,mark todo
    NRF_RADIO->MODECNF0 |= (RADIO_MODECNF0_RU_Fast << RADIO_MODECNF0_RU_Pos);
}

#if 0
static uint8_t  logperid = 0;
static void rf_timer_bond(nrf_timer_event_t event_type, void *p_context)
{

    if (event_type == NRF_TIMER_EVENT_COMPARE0)
    {
        //zk:  bond will not change config ,always rx mode
        uint32_t err_code = NRF_SUCCESS;

        if ((gInfo.left_ok + gInfo.right_ok) == 1)
        {
            gBondCnt--;
            if (gBondCnt == 0)
            {
                gBondCnt = 10000;
                rf_stop_data();
                rf_start_data();
                return;
            }
        }

        gCurrentChanelIndex = (gCurrentChanelIndex + 1) % gCurrentChanelmax;
        nrf_esb_stop_rx() ;
        err_code = nrf_esb_set_rf_channel(gPCurrentChanel[gCurrentChanelIndex]);
        APP_ERROR_CHECK(err_code);
        nrf_esb_start_rx();

        if (!logperid)
            NRF_LOG_WARNING("bond");
        logperid++;
    }
    else
    {
        NRF_LOG_WARNING("rf_timer_bond other");
    }
}
#endif


static void rf_timer_data(nrf_timer_event_t event_type, void *p_context)
{
    uint32_t err_code = NRF_SUCCESS;

    if (event_type == NRF_TIMER_EVENT_COMPARE0)
    {
        //zk:  switch to tx mode ,for req data by host


        //NRF_LOG_WARNING("rf_timer_data+");
        gCurrentChanelIndex = (gCurrentChanelIndex + 1) % gCurrentChanelmax;
        nrf_esb_stop_rx();
        nrf_esb_disable();
        //NRF_LOG_WARNING(" cha=%d",gPCurrentChanel[gCurrentChanelIndex]);
        err_code = nrf_esb_set_rf_channel(gPCurrentChanel[gCurrentChanelIndex]);
        APP_ERROR_CHECK(err_code);

        rf_change_ramup();

        nrf_esb_init(&gEsbDataTXconfig);

        rf_change_address(gCurID, gRF_ID, gPrefixes, 3);
        nrf_esb_flush_tx();
        rf_data_req();
        //NRF_LOG_WARNING("rf_timer_data-");
    }
    else
    {
        NRF_LOG_WARNING("rf_timer_data other ? ");
    }

    //NRF_LOG_FLUSH();
}


static void rf_timer_init(uint32_t time_val, nrfx_timer_event_handler_t handler)
{
    uint32_t err_code = NRF_SUCCESS;

    nrf_drv_timer_config_t timer_cfg = RF_TIMER_CONFIG;
    err_code = nrf_drv_timer_init(&gEsbCommunicationTimerInstance, &timer_cfg, handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_timer_extended_compare(&gEsbCommunicationTimerInstance, NRF_TIMER_CC_CHANNEL0, time_val, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

}



void rf_init_bond_info(void)
{

    ret_code_t ret;
    uint32_t readData;
    gInfo.left_ok = 0;
    gInfo.right_ok = 0;
    //gInfo.idx=0;

    ret = persist_read(&readData, BOND_FILE_ID, BOND_L_KEY);
    if ((ret == FDS_SUCCESS) && (readData > 0))
    {
        gInfo.left_ok = 1;
        gInfo.left_type = readData;
    }

	ret = persist_read(&readData, BOND_FILE_ID, JOYSTICK_L_VERSION);
    if ((ret == FDS_SUCCESS) && (readData > 0))
    {
        gLeftjoyStick_fwVersion = readData;
    }
	
    ret = persist_read(&readData, BOND_FILE_ID, BOND_R_KEY);
    if ((ret == FDS_SUCCESS) && (readData > 0))
    {
        gInfo.right_ok = 1;
        gInfo.right_type = readData;
    }

	ret = persist_read(&readData, BOND_FILE_ID, JOYSTICK_R_VERSION);
    if ((ret == FDS_SUCCESS) && (readData > 0))
    {
        gRightjoyStick_fwVersion = readData;
    }
}

ret_code_t rf_save_bond_info(uint32_t tp)
{

    uint32_t Data;
    ret_code_t ret = 0;
    NRF_LOG_WARNING("rf_save_bond_info");


    if (gInfo.left_ok == 1)
    {
        Data = tp;
        gInfo.left_type=Data;
        ret = persist_write(&Data, BOND_FILE_ID, BOND_L_KEY, 1, 1);
    }

    if (gInfo.right_ok == 1)
    {
        Data = tp;
        gInfo.right_type=Data;
        ret = persist_write(&Data, BOND_FILE_ID, BOND_R_KEY, 1, 1);
    }


    return ret;
}


ret_code_t rf_save_bond_left(uint32_t joyStickFwVersion, uint32_t productID)
{

    uint32_t Data;
    ret_code_t ret = 0;
    NRF_LOG_WARNING("rf_save_bond_left");


    if (gInfo.left_ok == 1)
    {
        Data = 1;
        gInfo.left_type = Data;
        ret = persist_write(&Data, BOND_FILE_ID, BOND_L_KEY, 1, 1);
		if (ret != NRF_SUCCESS)
			NRF_LOG_WARNING("[%s_%d]persist_write fail:%d\n", __func__, __LINE__, ret)
			
		ret = persist_write(&joyStickFwVersion, BOND_FILE_ID, JOYSTICK_L_VERSION, 1, 1);
		if (ret != NRF_SUCCESS)
			NRF_LOG_WARNING("[%s_%d]persist_write fail:%d\n", __func__, __LINE__, ret)

		ret = persist_write(&productID, BOND_FILE_ID, JOYSTICK_L_PRODUCT_ID, 1, 1);
		if (ret != NRF_SUCCESS)
			NRF_LOG_WARNING("[%s_%d]persist_write fail:%d\n", __func__, __LINE__, ret)
    }
    return ret;
}

ret_code_t rf_save_bond_right(uint32_t joyStickFwVersion, uint32_t productID)
{

    uint32_t Data;
    ret_code_t ret = 0;
    NRF_LOG_WARNING("rf_save_bond_right");


    if (gInfo.right_ok == 1)
    {
        Data = 1;
        gInfo.right_type=Data;
        ret = persist_write(&Data, BOND_FILE_ID, BOND_R_KEY, 1, 1);
		if (ret != NRF_SUCCESS)
			NRF_LOG_WARNING("[%s_%d]persist_write fail:%d\n", __func__, __LINE__, ret)

		ret = persist_write(&joyStickFwVersion, BOND_FILE_ID, JOYSTICK_R_VERSION, 1, 1);
		if (ret != NRF_SUCCESS)
			NRF_LOG_WARNING("[%s_%d]persist_write fail:%d\n", __func__, __LINE__, ret)

		ret = persist_write(&productID, BOND_FILE_ID, JOYSTICK_R_PRODUCT_ID, 1, 1);
		if (ret != NRF_SUCCESS)
			NRF_LOG_WARNING("[%s_%d]persist_write fail:%d\n", __func__, __LINE__, ret)
    }
    return ret;
}

void clear_SpecifiedJoyStick_bondInfo(uint8_t joyStickID)
{
	uint32_t Data = 0;
	ret_code_t ret;
	
	if (joyStickID == 0 && gInfo.left_ok) //left joyStick
	{
		ret = persist_read(&Data, BOND_FILE_ID, BOND_L_KEY);
	    if (ret == FDS_SUCCESS)
	    {
	        NRF_LOG_WARNING("check BOND_L_KEY :%d\n", Data);
	        if (Data )
	        {
	            Data = 0;
	            persist_write(&Data, BOND_FILE_ID, BOND_L_KEY, 1, 1);
	        }
	    }
		gInfo.left_ok = 0;
	}
	else if (joyStickID == 1 && gInfo.right_ok) //right joyStick
	{
		ret = persist_read(&Data, BOND_FILE_ID, BOND_R_KEY);
	    if (ret == FDS_SUCCESS)
	    {
	        NRF_LOG_WARNING("check BOND_R_KEY :%d\n", Data);
	        if (Data)
	        {
	            Data = 0;
	            persist_write(&Data, BOND_FILE_ID, BOND_R_KEY, 1, 1);
	        }
	    }
		gInfo.right_ok = 0;
	}
	gCurRfstate = STATE_NONE;
}

void rf_clear_bond_info(void)
{
    uint32_t Data = 0;
    ret_code_t ret;


    ret = persist_read(&Data, BOND_FILE_ID, BOND_L_KEY);
    if (ret == FDS_SUCCESS)
    {
        if (Data)
        {
            Data = 0;
            persist_write(&Data, BOND_FILE_ID, BOND_L_KEY, 1, 1);
        }
    }


    ret = persist_read(&Data, BOND_FILE_ID, BOND_R_KEY);
    if (ret == FDS_SUCCESS)
    {
        if (Data)
        {
            Data = 0;
            persist_write(&Data, BOND_FILE_ID, BOND_R_KEY, 1, 1);
        }
    }

    ret = persist_read(&Data, BOND_FILE_ID, BOND_L_KEY);
    if (ret == FDS_SUCCESS)
    {
        NRF_LOG_WARNING("after clear, BOND_L_KEY:%d ", Data);
    }
    ret = persist_read(&Data, BOND_FILE_ID, BOND_R_KEY);
    if (ret == FDS_SUCCESS)
    {
        NRF_LOG_WARNING("after clear, BOND_R_KEY:%d ", Data);
    }
    gInfo.left_ok = 0;
    gInfo.right_ok = 0;


#if 0
    NRF_LOG_FLUSH();
    nrf_delay_ms(200);
    NVIC_SystemReset();
#endif

}

// if not pair success ,just use old info at next reset
void rf_app_re_pair()
{
    // TODO flash

}


static void rf_init_id()
{
    gRF_ID[0] = (NRF_FICR->DEVICEID[0] & 0x000000ff);
    gRF_ID[1] = (NRF_FICR->DEVICEID[0] & 0x0000ff00) >> 8;
    gRF_ID[2] = (NRF_FICR->DEVICEID[0] & 0x00ff0000) >> 16;
    gRF_ID[3] = (NRF_FICR->DEVICEID[0] & 0xff000000) >> 24;

    //gPrefixes[1]=(NRF_FICR->DEVICEID[1]& 0x000000ff);
    //gPrefixes[2]=(NRF_FICR->DEVICEID[1]& 0x0000ff00)>>8;

    NRF_LOG_WARNING("rf %x:%x:%x:%x", gRF_ID[0], gRF_ID[1], gRF_ID[2], gRF_ID[3]);

}



void rf_bond_event(nrf_esb_evt_t const *p_event)
{
    switch (p_event->evt_id)
    {
    case NRF_ESB_EVENT_RX_RECEIVED:

        //   NRF_LOG_WARNING("rf_bond_event+");
        if (nrf_esb_read_rx_payload(&gRxPayload) == NRF_SUCCESS)
        {

            if (gRxPayload.data[0] == BOND_CMD_TAG)
            {
                if (gRxPayload.data[1] == BOND_CMD_LEFT)
                {
                    gTxPayload.data[0] = gRF_ID[0];
                    gTxPayload.data[1] = gRF_ID[1];
                    gTxPayload.data[2] = gRF_ID[2];
                    gTxPayload.data[3] = gRF_ID[3];

                    gTxPayload.data[4] = BOND_CMD_TAG;
                    gTxPayload.data[5] = gRxPayload.data[1];
                    gTxPayload.data[6] = gDataChannelGroupID;

                    gTxPayload.length = BOND_CMD_LEN;
                    gTxPayload.pipe = 0;

                    gTxPayload.noack = true; ///////test

                    nrf_esb_flush_rx();
                    nrf_esb_flush_tx();
                    nrf_esb_write_payload(&gTxPayload);


                    gBondReqRecvCNTL++;

                }
                else if (gRxPayload.data[1] == BOND_CMD_RIGHT) // now use same base
                {
                    gTxPayload.data[0] = gRF_ID[0];
                    gTxPayload.data[1] = gRF_ID[1];
                    gTxPayload.data[2] = gRF_ID[2];
                    gTxPayload.data[3] = gRF_ID[3];


                    gTxPayload.data[4] = BOND_CMD_TAG;
                    gTxPayload.data[5] = gRxPayload.data[1];
                    gTxPayload.data[6] = gDataChannelGroupID;

                    gTxPayload.length = BOND_CMD_LEN;
                    gTxPayload.pipe = 0;
                    gTxPayload.noack = true; ///////test

                    nrf_esb_flush_rx();
                    nrf_esb_flush_tx();
                    nrf_esb_write_payload(&gTxPayload);



                    gBondReqRecvCNTR++;

                }
                else
                {
                    NRF_LOG_WARNING("error bond ");
                    return;
                }



            }
            else  if ((gRxPayload.data[0] == BOND_RES_TAG) || (gRxPayload.data[0] == BOND_RES_TAG2))
            {

                if (gRxPayload.data[1] == BOND_CMD_LEFT)
                {
                    gInfo.left_ok = 1;
                    //rf_save_bond_left(rtype);
                    NRF_LOG_WARNING("left ok ");
                }
                else if (gRxPayload.data[1] == BOND_CMD_RIGHT)
                {
                    gInfo.right_ok = 1;
                    //rf_save_bond_right(rtype);
                    NRF_LOG_WARNING("right ok ");
                }
                else
                {
                    NRF_LOG_WARNING("error bond 2");
                    return;
                }


                nrf_esb_flush_rx();
                if (gInfo.left_ok && gInfo.right_ok)
                {
                    // NRF_LOG_WARNING("start data ");
                    rf_stop_data();
                    rf_start_data();
                }



            }

            return;
        }

        //NRF_LOG_WARNING("error bond 3");

    }


}


void rf_data_event(nrf_esb_evt_t const *p_event)
{
    //NRF_LOG_WARNING("evid=%d",p_event->evt_id);
    switch (p_event->evt_id)
    {


    case NRF_ESB_EVENT_TX_FAILED:
        nrf_esb_flush_tx();
    case NRF_ESB_EVENT_TX_SUCCESS:

        //NRF_LOG_WARNING("tx ok+");
        //zk: if  request has send ,then wait for data . noack always success ?
        nrf_esb_disable();
        nrf_esb_init(&gEsbDataRXconfig);
        rf_change_address(gCurID, gRF_ID, gPrefixes, 3);
        nrf_esb_start_rx();
        rf_change_ramup();

        break;

    case NRF_ESB_EVENT_RX_RECEIVED:

        if (nrf_esb_read_rx_payload(&gRxPayload) == NRF_SUCCESS)
        {

            nrf_esb_flush_rx();
            //if(gRxPayload.data[0]==DATA_TAG_LEFT)
            if (gRxPayload.pipe == 1)
            {


                gLastsendL = 0;

                if(gRxPayload.length > 2)
				{
                	if (gDataCallback)
						gDataCallback(&gRxPayload.data[1], gRxPayload.length - 1, 0);
                  	gDataRecvCntL++;
									current_Ldata = gRxPayload.data[2];
            	}


            }
            //else if(gRxPayload.data[0]==DATA_TAG_RIGHT)
            else if (gRxPayload.pipe == 2)
            {
                gLastsendR = 0;
                if(gRxPayload.length > 2)
				{
                	if (gDataCallback)
						gDataCallback(&gRxPayload.data[1], gRxPayload.length - 1, 1);
                	gDataRecvCntR++;
									current_Rdata = gRxPayload.data[2];
              }
            }
            else
            {
                NRF_LOG_WARNING("error bond 2");
                return;
            }
        }
        break;
    }

}



static uint32_t rf_change_address(uint8_t *add0, uint8_t *add1, uint8_t *prefixes, uint8_t cnt)
{
    uint32_t err_code;

    err_code = nrf_esb_set_base_address_0(add0);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_1(add1);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_prefixes(prefixes, cnt);
    VERIFY_SUCCESS(err_code);

    rf_change_ramup();
    return err_code;

}


#if 0
static uint32_t rf_start_bond(void)
{

    uint32_t err_code;
    NRF_LOG_WARNING("rf_start_bond ");

// config
    gCurTimerhandler = rf_timer_bond;
    gCurTimerCnt = CUST_RF_TIMER_CNT_BOND;
    gCurrentChanelIndex = 0;
    gCurrentChanelmax = CUST_CHANNEL_NUM_4_BOND;
    gPCurrentChanel = gChanel4bond;
    gCurRfstate = STATE_CONFIG;
    gCurID = g_ID0;
    gCurEventCb = rf_bond_event;

// use config
    rf_timer_init(gCurTimerCnt, gCurTimerhandler);

    nrf_esb_stop_rx();
    nrf_esb_disable();

    // esb config
    rf_change_ramup();
    //gEsbDataBondconfig.tx_output_power  = RADIO_TXPOWER_TXPOWER_Pos4dBm;
    //gEsbDataBondconfig.tx_mode                  = NRF_ESB_TXMODE_MANUAL;
    gEsbDataBondconfig.event_handler = gCurEventCb;
    err_code = nrf_esb_init(&gEsbDataBondconfig);
    VERIFY_SUCCESS(err_code);

    rf_change_address(gCurID, gRF_ID, gPrefixes, 3);


    err_code = nrf_esb_set_rf_channel(gPCurrentChanel[gCurrentChanelIndex]);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_esb_start_rx();
    APP_ERROR_CHECK(err_code);

    gBondCnt = CUST_S_CNT;
    rf_timer_start;

    gCurRfstate = STATE_CONFIG;
    return err_code;
}
#endif

void rf_stop_data(void)
{
    NRF_LOG_WARNING("rf_stop_bond ++");

    nrf_esb_stop_rx();
    nrf_esb_disable();
	if (rf_is_timer_started)
    	rf_timer_uninit;
    NRF_LOG_WARNING("rf_stop_bond --");
}

static void rf_start_data(void)
{

//  uint32_t err_code;
    int i;
    gCurTimerhandler = rf_timer_data;
    gCurTimerCnt = CUST_RF_TIMER_CNT_DATA;
    gCurrentChanelIndex = 0;
    gCurrentChanelmax = CUST_CHANNEL_NUM_4_DATA;
    gPCurrentChanel = gChanel4data[gDataChannelGroupID];
    gCurRfstate = STATE_DATA;
    gCurID = g_ID1;
    gCurEventCb = rf_data_event;
    NRF_LOG_WARNING("rf_start_data+");
    for (i = 0; i < 4; i++)
    {
        gCurID[i] = gRF_ID[3 - i];
    }
    rf_change_address(gCurID, gRF_ID, gPrefixes, 3);

    //

    NRF_LOG_WARNING("RF:%x:%x:%x:%x", gCurID[0], gCurID[1], gCurID[2], gCurID[3]);

    rf_timer_init(gCurTimerCnt, gCurTimerhandler);
    nrf_esb_stop_rx();
    nrf_esb_disable();

    gEsbDataTXconfig.event_handler = gCurEventCb;
    gEsbDataRXconfig.event_handler = gCurEventCb;

    gDataReqSendCnt = 0;

    rf_timer_start;
    gDatastartCallback();

    NRF_LOG_WARNING("rf_start_data-");
}



/*data include : left and right and chanel
 if not received by controller. will send next time.

*/

uint32_t get_req_tick(void)
{
    return  gReqTick;
}

uint32_t get_req_cnt(void)
{

    return gDataReqSendCnt;

}




static void rf_data_req()
{
    nrf_esb_payload_t  payload;
    //uint8_t dtrt=0;
    uint8_t ptmp[REQ_CMD_LEN] = {0};

    payload.pipe = 0;
    payload.noack = true; 
    payload.length = REQ_CMD_LEN;
    payload.data[0] = REQ_TAG;

    //payload.data[1]=0;

    //gReqTick=gSysTick;
    sys_tick_get_unlock(&gReqTick);
    gReqCallback(ptmp, 4, gReqTick);

    payload.data[1] = ptmp[0];
    payload.data[2] = ptmp[1]; 

	/*
	 * bit[12, 15]: motor amplitude
	 * bit[8, 11]: Motor vibration frequency
	 * bit[4, 7]: Motor vibration time
	 * bit[2, 3]: Motor vibration switch
	 * bit[0, 1]: Handle ID
	 */
	payload.data[3] = ptmp[2];
	payload.data[4] = ptmp[3];
#if 1
    if (gLastsendL == 1)
        payload.data[1] |= REQ_LASTL;

    if (gLastsendR == 1)
        payload.data[1] |= REQ_LASTR;
#endif

    //memcpy(&payload.data[3], (void *)&gReqTick, 4);
    gLastsendL = 1;
    gLastsendR = 1;
    nrf_esb_write_payload(&payload);


    gDataReqSendCnt++;

    //NRF_LOG_WARNING("send");

}



void rf_debug_init(void)
{
    gBondReqRecvCNTL = 0;
    gBondReqRecvCNTR = 0;

    gDataReqSendCnt = 0;
    gDataRecvCntR = 0;
    gDataRecvCntL = 0;
    NRF_LOG_WARNING("rf_debug_init");

}

void rf_debug_print(void)
{

    if (gCurRfstate == STATE_DATA)
    {
        NRF_LOG_WARNING("ReqCnt %d: L %d:  R %d", gDataReqSendCnt, gDataRecvCntL, gDataRecvCntR);

        //NRF_LOG_WARNING("miss  L %d:   R %d",(gDataReqSendCnt-gDataRecvCntL),(gDataReqSendCnt-gDataRecvCntR));
    }
    else
    {
        NRF_LOG_WARNING("gBondRecvCNTL %d : gBondRecvCNTR %d :ReqCnt %d: RecvCntL %d: RecvCntR %d", gBondReqRecvCNTL, gBondReqRecvCNTR, gDataReqSendCnt, gDataRecvCntL, gDataRecvCntR);
    }
    //NRF_LOG_WARNING("addr %x :%x :%x",NRF_RADIO->BASE0,NRF_RADIO->BASE1,NRF_RADIO->PREFIX0);


}







void rf_module_init_and_start(rf_config_t *cfg)
{

    gDataChannelGroupID = cfg->randidx;

    rf_debug_init();
    rf_init_id();
    //rf_clear_bond_info();// debug only
    rf_init_bond_info();

    gDataCallback = cfg->data_cb;
    gReqCallback = cfg->req_cb;
    gDatastartCallback = cfg->data_start;

    sys_tick_get_unlock(&gCurTick);
    NRF_LOG_WARNING("bond %d :%d ", gInfo.left_ok, gInfo.right_ok);

    if (gInfo.left_ok && gInfo.right_ok) //After binding, enable data acquisition of left/right joystick
    {
    	needAckHost(1);
        rf_start_data();
    }
    NRF_LOG_WARNING("[%s] successfully\n", __func__);
}



void rf_module_stop()
{

    NRF_LOG_WARNING("rf_module_stop ++");

    nrf_esb_stop_rx();
    nrf_esb_disable();
    rf_timer_uninit;

    NRF_LOG_WARNING("rf_module_stop --");

}



uint8_t rf_get_bond_info(void)
{

    uint8_t ret = 0;

    if (gInfo.left_ok)
        ret = gInfo.left_type & 0x0f;
    else
        ret = 0;


    if (gInfo.right_ok)
        ret |= (gInfo.right_type &0x0f) << 4;
    else
        ret &= 0x0f;

  //  NRF_LOG_WARNING("left %x, right %x" ,gInfo.left_type, gInfo.right_type);

    return ret;

}

static void bond_timer_handler(nrf_timer_event_t event_type, void *p_context)
{
	uint32_t err_code = NRF_SUCCESS;

    if (event_type == NRF_TIMER_EVENT_COMPARE0)
    {
        gCurrentChanelIndex = (gCurrentChanelIndex + 1) % gCurrentChanelmax;
        nrf_esb_stop_rx() ;
        err_code = nrf_esb_set_rf_channel(gPCurrentChanel[gCurrentChanelIndex]);
        APP_ERROR_CHECK(err_code);
        nrf_esb_start_rx();
    }
}

static void bond_timer_init(uint32_t time_val, nrfx_timer_event_handler_t handler)
{
    uint32_t err_code = NRF_SUCCESS;

    nrf_drv_timer_config_t timer_cfg = RF_TIMER_CONFIG;
    err_code = nrf_drv_timer_init(&gBondTimerInstance, &timer_cfg, handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_timer_extended_compare(&gBondTimerInstance, NRF_TIMER_CC_CHANNEL0, time_val, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
}

void bond_event_handler(nrf_esb_evt_t const *p_event)
{
    switch (p_event->evt_id)
    {
    case NRF_ESB_EVENT_RX_RECEIVED:
        if (nrf_esb_read_rx_payload(&gRxPayload) == NRF_SUCCESS)
        {
            if (gRxPayload.data[0] == BOND_CMD_TAG)
            {
                if ((gRxPayload.data[1] == BOND_CMD_LEFT) && (grequestBondHandleID == 0))
                {
                    gTxPayload.data[0] = gRF_ID[0];
                    gTxPayload.data[1] = gRF_ID[1];
                    gTxPayload.data[2] = gRF_ID[2];
                    gTxPayload.data[3] = gRF_ID[3];
                    gTxPayload.data[4] = BOND_CMD_TAG;
                    gTxPayload.data[5] = gRxPayload.data[1];
                    gTxPayload.data[6] = gDataChannelGroupID;

                    gTxPayload.length = BOND_CMD_LEN;
                    gTxPayload.pipe = 0;
                    gTxPayload.noack = true;

                    nrf_esb_flush_rx();
                    nrf_esb_flush_tx();
                    nrf_esb_write_payload(&gTxPayload);
					NRF_LOG_WARNING("receive left handleID request\n");
                }
                else if ((gRxPayload.data[1] == BOND_CMD_RIGHT) && (grequestBondHandleID == 1))
                {
                    gTxPayload.data[0] = gRF_ID[0];
                    gTxPayload.data[1] = gRF_ID[1];
                    gTxPayload.data[2] = gRF_ID[2];
                    gTxPayload.data[3] = gRF_ID[3];
                    gTxPayload.data[4] = BOND_CMD_TAG;
                    gTxPayload.data[5] = gRxPayload.data[1];
                    gTxPayload.data[6] = gDataChannelGroupID;

                    gTxPayload.length = BOND_CMD_LEN;
                    gTxPayload.pipe = 0;
                    gTxPayload.noack = true;

                    nrf_esb_flush_rx();
                    nrf_esb_flush_tx();
                    nrf_esb_write_payload(&gTxPayload);
					NRF_LOG_WARNING("receive right handleID request\n");

                }
                else
                {
                    NRF_LOG_WARNING("unknown handleID\n");
                    return;
                }



            }
            else  if (gRxPayload.data[0] == BOND_RES_TAG)
            {
				
                if (gRxPayload.data[1] == BOND_CMD_LEFT && (grequestBondHandleID == 0))
                {
                    gInfo.left_ok = 1;
					gLeftjoyStick_fwVersion = (gRxPayload.data[2] | (gRxPayload.data[3] << 8));
					gLeftjoyStick_productNameID = gRxPayload.data[4];
                    rf_save_bond_left(gLeftjoyStick_fwVersion, gLeftjoyStick_productNameID);
                    NRF_LOG_WARNING("left joystick bond success\n");
                }
                else if (gRxPayload.data[1] == BOND_CMD_RIGHT && (grequestBondHandleID == 1))
                {
                    gInfo.right_ok = 1;
					gRightjoyStick_fwVersion = (gRxPayload.data[2] | (gRxPayload.data[3] << 8));
					gRightjoyStick_productNameID = gRxPayload.data[4];
                    rf_save_bond_right(gRightjoyStick_fwVersion, gRightjoyStick_productNameID);
                    NRF_LOG_WARNING("right joystick bond success\n");
                }
                else
                {
                    NRF_LOG_WARNING("handle ack error\n");
                    return;
                }


                nrf_esb_flush_rx();
                if (gInfo.left_ok && gInfo.right_ok)
                {
                    NRF_LOG_WARNING("left and right joystick are bound successfully\n");
					nrf_esb_stop_rx();
    				nrf_esb_disable();
					needAckHost(1);
                    rf_start_data();
                }

            }
			else
			{
				NRF_LOG_WARNING("fail to receive handle cmd\n");
			}
            return;
        }

    }


}

uint32_t bondSpecifiedJoyStick(uint8_t joyStickID)
{
	uint32_t err_code = 0;
	static uint8_t init_done = 0;

	grequestBondHandleID = joyStickID;
	if(init_done == 1 && gCurRfstate == STATE_CONFIG)
	{
		return err_code;
	}
		
	gRF_ID[0] = (NRF_FICR->DEVICEID[0] & 0x000000ff);
    gRF_ID[1] = (NRF_FICR->DEVICEID[0] & 0x0000ff00) >> 8;
    gRF_ID[2] = (NRF_FICR->DEVICEID[0] & 0x00ff0000) >> 16;
    gRF_ID[3] = (NRF_FICR->DEVICEID[0] & 0xff000000) >> 24;
	
	gCurTimerhandler = bond_timer_handler;
    gCurTimerCnt = CUST_RF_TIMER_CNT_BOND;
    gCurrentChanelIndex = 0;
    gCurrentChanelmax = CUST_CHANNEL_NUM_4_BOND;
    gPCurrentChanel = gChanel4bond;
	gCurEventCb = bond_event_handler;
	gCurRfstate = STATE_CONFIG;
	
	nrf_esb_stop_rx();
    nrf_esb_disable();
    rf_change_ramup();
    gEsbDataBondconfig.event_handler = gCurEventCb;
    err_code = nrf_esb_init(&gEsbDataBondconfig);
    VERIFY_SUCCESS(err_code);

    rf_change_address(g_ID0, gRF_ID, gPrefixes, 3);

    err_code = nrf_esb_set_rf_channel(gPCurrentChanel[gCurrentChanelIndex]);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_esb_start_rx();
    APP_ERROR_CHECK(err_code);
	if (init_done == 0)
	{
		bond_timer_init(gCurTimerCnt, gCurTimerhandler);
		nrf_drv_timer_enable(&gBondTimerInstance);
	}
	init_done = 1;

	NRF_LOG_WARNING("[%s]init successfully\n", __func__);
	return err_code;
}
