
/*********************************************

shadowCreate controler .
v0.1. create        20191111           zk.xu
v0.1. add k11,fix bug      2020           zk.xu


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
//#include "sw3153/sw3153_driver.h"
//#include "app_timer.h"
// pravate

#include "app_scheduler.h"
#include "nrf_clock.h"

#include "app_timer.h" //BSP_DEFINES_ONLY


#include "persist.h"
#include "m_rf.h"
#include "sys.h"
#include "d_packet.h"
#include "data.h"
#include "user_config.h"


#include "xcg.h"
#include "rled_ctl.h"
#include "rf0.h"



#define SCHED_MAX_EVENT_DATA_SIZE sizeof(app_timer_event_t)

#define SCHED_QUEUE_SIZE                10

// for fast use
extern uint8_t  gleden;
extern uint32_t  gvib;
extern uint32_t  gledval;

static uint16_t glastidL = 0xffff;
static uint16_t glastidR = 0xffff;

uint32_t get_req_tick(void);
uint32_t get_req_cnt(void);

uint32_t logging_init(void)
{
    uint32_t err_code;
    err_code = NRF_LOG_INIT(NULL);

    NRF_LOG_DEFAULT_BACKENDS_INIT();

    return err_code;
}

uint8_t glastseqL = 0;
uint8_t glastseqR = 0;

static uint32_t glossL = 0;
static uint32_t glossR = 0;
uint32_t gAploss = 0;


static uint8_t firstL = 1;
static uint8_t firstR = 1;

static uint32_t glossLbk = 0;
static uint32_t glossRbk = 0;
static uint32_t glossApbk = 0;


void tem_init(void)
{

    if ((NRF_UICR->NFCPINS & UICR_NFCPINS_PROTECT_Msk) == (UICR_NFCPINS_PROTECT_NFC << UICR_NFCPINS_PROTECT_Pos))
    {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NRF_UICR->NFCPINS &= ~UICR_NFCPINS_PROTECT_Msk;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NVIC_SystemReset();
    }
    
    NRF_POWER->DCDCEN = 1;
}


static void   protocal_data_start(void)
{
    xcg_set_freq(XCG_T1);
}




static void   protocal_req_cb(uint8_t *pdata, uint8_t size, uint32_t rts)
{
    uint8_t dtrt = 0;
    uint32_t  ledval = 0;

//  ___LDBG___
    //gleden=0x03;// for debug use always on

    if (gleden & 0x01)
        pdata[0] |= REQ_LEFT;

    if (gleden & 0x02)
        pdata[0] |= REQ_RIGHT;

    ledval = gledval; //rled_getraw();
    if (ledval != 0)
    {
        //NRF_LOG_WARNING("=====");
        if ((rts - ledval) >= RLDCFG)
        {
            //NRF_LOG_WARNING("dtrt0:-%d",(rts-ledval-RLDCFG));
            dtrt = 0xff;
            gledval = 0;
        }
        else
        {
            dtrt = RLDCFG + ledval - rts;
            //NRF_LOG_WARNING("dtrt1:%d",dtrt);
            //NRF_LOG_WARNING("ont:%d",rts+dtrt);
        }
        
    }

    pdata[1] = dtrt;
	pdata[2] = (gvib&0x00ff);
	pdata[3] = ((gvib&0xff00)>>8);
	gvib = 0; //the data is invalid after being set once
}



#if 0// test
void   protocal_data_cb(uint8_t *pdata, uint8_t size, uint8_t idx)
{
    uint32_t  ts;
    uint8_t  seq;
    memcpy(&ts, (void *)pdata, 4);

    seq = pdata[4];
    if ((ts == 0) && (seq == 0))
    {
        NRF_LOG_WARNING("hskip0 sizd[%d]", idx);
    }
    else
    {

        if (idx == 1)
        {
            NRF_LOG_WARNING("R seq %d,last %d ts=%d", seq, glastseqR, ts);
            glastseqR = seq;
        }


        NRF_LOG_FLUSH();

    }

}


#else

#ifdef  HOST_DIR_AP
void   protocal_data_cb(uint8_t *pdata, uint8_t size, uint8_t idx)
{
    d_packet_t pak;
    uint8_t buffer;
    uint32_t reqtick;

    //  if(size<3)return;

    set_entry(pdata);

    // buffer=get_data(PI_OF,PI_BW);
    buffer = pdata[1];

    if (idx == 0)
    {
        if (glastidL == buffer)
        {
            //NRF_LOG_WARNING("discard0 ");
            return;
        }
        glastidL = buffer;


    }
    else
    {
        if (glastidR == buffer)
        {
            //NRF_LOG_WARNING("discard1 ");
            return;
        }
        glastidR = buffer;

    }
#if 1
    if (idx == 0)
    {
        uint8_t  diff = buffer - glastseqL;

        if (!firstL)
        {

            if ((diff != 1) && (diff != 0))
            {
                glossL = glossL + diff - 1;
                //NRF_LOG_WARNING("L :diff=%d,seq %d,last %d",diff,buffer,glastseqL);
            }
        }
        firstL = 0;
        glastseqL = buffer;
    }
    else
    {
        uint8_t  diff = buffer - glastseqR;

        if (!firstR)
        {

            if ((diff != 1) && (diff != 0))
            {
                glossR = glossR + diff - 1;
                //NRF_LOG_WARNING("R diff=%d,seq %d,last %d",diff,buffer,glastseqR);
            }
        }
        firstR = 0;
        glastseqR = buffer;
    }


#endif


    //buffer=(uint8_t)get_data(TM_OF,TM_BW); //rts

    buffer = pdata[0];

    reqtick = get_req_tick();

    if (buffer & 0x80) // ibr
    {
        pak.ts = reqtick - (buffer & 0x7f);
    }
    else
    {
        pak.ts = reqtick + (buffer & 0x7f);
    }

    if (idx == 0)
    {
        pdata[0] = 0xFF;
        // pdata[1]=0xAA;
    }
    else
    {
        pdata[0] = 0xFE;
        //pdata[1]=0xBB;
    }

    memcpy(pak.data, (void *)pdata, size);
    pak.size = size;
    d_packet_add(&pak);

}
#else

void   protocal_data_cb(uint8_t *pdata, uint8_t size, uint8_t idx)
{


    uint32_t  ts;
    uint8_t  seq;
    memcpy(&ts, (void *)pdata, 4);

    seq = pdata[4];
    if ((ts == 0) && (seq == 0))
    {
        //NRF_LOG_WARNING("hskip0 sizd[%d]",idx);
    }
    else
    {

        if (idx == 0)
        {
            uint8_t  diff = seq - glastseqL;

            if (!firstL)
            {

                if ((diff != 1) && (diff != 0))
                {
                    glossL = glossL + diff - 1;
                    NRF_LOG_WARNING("L :diff=%d,seq %d,last %d", diff, seq, glastseqL);
                }
            }
            firstL = 0;
            glastseqL = seq;
        }
        else
        {
            uint8_t  diff = seq - glastseqR;

            if (!firstR)
            {

                if ((diff != 1) && (diff != 0))
                {
                    glossR = glossR + diff - 1;
                    NRF_LOG_WARNING("R diff=%d,seq %d,last %d", diff, seq, glastseqR);
                }
            }
            firstR = 0;
            glastseqR = seq;
        }
    }

}
#endif

#endif



void  paras_ap_data(uint8_t *pdata)
{
    //if(pdata[0]==)



}


uint8_t get_generate_host_rf_idx(void)
{

    uint32_t readData;
    ret_code_t err_code;
    uint8_t rand;

    err_code = persist_read(&readData, BOND_FILE_ID, GROUP_IDX);

    if ((err_code == FDS_SUCCESS) && (readData < 7))
    {

        rand = readData & 0x0000000ff;
    }
    else
    {
        readData = sys_rand_get();
        err_code = persist_write(&readData, BOND_FILE_ID, GROUP_IDX, 1, 1);
        if (err_code == FDS_SUCCESS)
        {
            NRF_LOG_WARNING("write rf idx info ok ");
        }
        rand = readData;
    }
    return rand;

}



void test_off()
{
#define RAM_RETENTION_OFF       (0x00000003UL)

#if 1
    NRF_POWER->RAM[0].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[1].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[2].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[3].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[4].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[5].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[6].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[7].POWER = RAM_RETENTION_OFF;

    // Set nRF5 into System OFF. Reading out value and looping after setting the register
    // to guarantee System OFF in nRF52.
    NRF_POWER->SYSTEMOFF = 0x1;
    (void) NRF_POWER->SYSTEMOFF;

    while (true);
#endif

}

int main(void)
{
    uint32_t err_code;
    rf_config_t  cfg;
	
    tem_init();

    err_code = logging_init();
    APP_ERROR_CHECK(err_code);

    NRF_LOG_WARNING("host init");
    NRF_LOG_FLUSH();

    if (!nrf_clock_lf_is_running())
    {
        nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);
    }

    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);

    err_code = app_timer_init();
    NRF_LOG_WARNING("\r\napp_timer_init err_code=%d \n", err_code);
    NRF_LOG_FLUSH();

    nrf_gpio_cfg_input(21, NRF_GPIO_PIN_NOPULL);

    rled_init();

    //xcg_init();
    d_packet_init();


    persist_init();


    sys_clocks_switch();

    NRF_LOG_WARNING("sys_clocks_switch v2");
    sys_tick_init();
    sys_tick_start();

    cfg.data_cb = protocal_data_cb;
    cfg.data_start = protocal_data_start;
    cfg.req_cb = protocal_req_cb;

    cfg.randidx = get_generate_host_rf_idx();
    cfg.rldcfg = RLDCFG;

    NRF_LOG_WARNING("randidx:%d", cfg.randidx);
    rf_module_init_and_start(&cfg);
    //xcg_start();


	sc_spiSlave_init();
	sc_spi_transfer_timer_init();
	sc_spi_transfer_timer_start();
	
	NRF_LOG_FLUSH();
    while (true) 
    {
        app_sched_execute();

        if (NRF_LOG_PROCESS() == false)
        {
            if ((glossL != glossLbk) || (glossR != glossRbk) || (gAploss != glossApbk))
                NRF_LOG_WARNING("lL=%d,lR=%d,Ap %d [%d]", glossL, glossR, gAploss, get_req_cnt());
            glossLbk = glossL;
            glossRbk = glossR;
            glossApbk = gAploss;
            NRF_LOG_FLUSH();
        }
    }
}
