/*******************************
shadowCreate controler .
v0.1. create        20191122     zk.xu

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
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf.h"
#include "nrf_drv_timer.h"
//#include "sw3153_driver.h"
#include "persist.h"






#define COLOR_GREEN     "\033[1;32m"
#define COLOR_YELLOW    "\033[1;33m"
#define COLOR_CYAN      "\033[1;36m"

#define NRF_LOG_CYAN(...)   NRF_LOG_INFO(COLOR_CYAN   __VA_ARGS__)
#define NRF_LOG_YELLOW(...) NRF_LOG_INFO(COLOR_YELLOW __VA_ARGS__)
#define NRF_LOG_GREEN(...)  NRF_LOG_INFO(COLOR_GREEN  __VA_ARGS__)



static bool volatile m_fds_done;

static void wait_for_fds(void)
{
    while (!m_fds_done)
    {
    }
}

static void fds_evt_handler(fds_evt_t const *p_evt)
{

    switch (p_evt->id)
    {
    case FDS_EVT_INIT:
        if (p_evt->result == FDS_SUCCESS)
        {
            m_fds_done = true;
        }
        //NRF_LOG_INFO("m_fds_initialized result =%d ", p_evt->result);
        break;

    case FDS_EVT_WRITE:
    case FDS_EVT_UPDATE:
    {
        if (p_evt->result == FDS_SUCCESS)
        {
            // NRF_LOG_INFO("Record ID:\t0x%04x",  p_evt->write.record_id);
            //  NRF_LOG_INFO("File ID:\t0x%04x",    p_evt->write.file_id);
            // NRF_LOG_INFO("Record key:\t0x%04x", p_evt->write.record_key);
        }
        m_fds_done = true;
    }
    break;

    case FDS_EVT_DEL_RECORD:
    {
        if (p_evt->result == FDS_SUCCESS)
        {
            //NRF_LOG_INFO("Record ID:\t0x%04x",  p_evt->del.record_id);
            //NRF_LOG_INFO("File ID:\t0x%04x",    p_evt->del.file_id);
            //NRF_LOG_INFO("Record key:\t0x%04x", p_evt->del.record_key);
        }
        m_fds_done = true;
    }
    break;

    default:
        break;
    }
}


ret_code_t persist_delete(uint32_t fileId, uint32_t recKey, uint32_t sync)
{
    fds_record_desc_t   record_desc;
    fds_find_token_t    ftok;

    ftok.page = 0;
    ftok.p_addr = NULL;
    // Loop and find records with same ID and rec key and mark them as deleted.
    while (fds_record_find(fileId, recKey, &record_desc, &ftok) == FDS_SUCCESS)
    {
        m_fds_done = 0;
        ret_code_t ret = fds_record_delete(&record_desc);
        if (ret != FDS_SUCCESS)
        {
            return ret;
        }
        //NRF_LOG_INFO("Deleted record ID: %d \r\n",record_desc.record_id);
        if (sync)
        {
            wait_for_fds();
        }

    }

    return NRF_SUCCESS;
}



ret_code_t persist_write(uint32_t *writeData, uint32_t fileId, uint32_t recKey, uint32_t size, uint32_t sync)
{
    //static uint32_t m_deadbeef[2] = {0xDEADBEEF,0xBAADF00D};
    fds_record_t        record;
    fds_record_desc_t   record_desc = {0};

    ret_code_t ret;

    fds_find_token_t  tok   = {0};


    // Set up record.
    record.file_id              = fileId;
    record.key                      = recKey;
    record.data.p_data       = writeData;
    record.data.length_words   = size;

    m_fds_done = 0;

    ret = fds_record_find(fileId, recKey, &record_desc, &tok);
    if (ret == FDS_SUCCESS)
    {
        ret = fds_record_update(&record_desc, &record);
        if (ret != FDS_SUCCESS)
        {

            NRF_LOG_INFO("fds_record_update failed Record ID = %d \r\n", record_desc.record_id);
            if (ret == FDS_ERR_NO_SPACE_IN_FLASH)
            {
                fds_gc();
            }
            return ret;
        }
    }
    else
    {
        ret_code_t ret = fds_record_write(&record_desc, &record);
        if (ret != FDS_SUCCESS)
        {
            NRF_LOG_INFO("fds_record_write failed Record ID = %d \r\n", record_desc.record_id);
            return ret;
        }

    }

    if (sync)
        wait_for_fds();

    // NRF_LOG_INFO("Writing Record ID = %d \r\n",record_desc.record_id);
    return NRF_SUCCESS;
}




ret_code_t persist_read(uint32_t *readData, uint32_t fileId, uint32_t recKey)
{
    fds_flash_record_t  flash_record;
    fds_record_desc_t   record_desc;
    fds_find_token_t    ftok = {0}; //Important, make sure you zero init the ftok token
    uint32_t *data;
    uint32_t err_code;

    uint32_t found = 0;

    //NRF_LOG_INFO("---------------------------");
    //NRF_LOG_INFO("persist_read");
    // Loop until all records with the given key and file ID have been found.
    while (fds_record_find(fileId, recKey, &record_desc, &ftok) == FDS_SUCCESS)
    {
        err_code = fds_record_open(&record_desc, &flash_record);
        if (err_code != FDS_SUCCESS)
        {
            return err_code;
        }

        //NRF_LOG_INFO("Found Record ID=%d",record_desc.record_id);
        //NRF_LOG_INFO("Data= ");
        data = (uint32_t *) flash_record.p_data;
        for (uint8_t i = 0; i < flash_record.p_header->length_words; i++)
        {
            //NRF_LOG_INFO("0x%x",data[i]);
            readData[i] = data[i];
        }
        found = 1;
        //NRF_LOG_INFO("\r\n");
        // Access the record through the flash_record structure.
        // Close the record when done.
        err_code = fds_record_close(&record_desc);
        if (err_code != FDS_SUCCESS)
        {
            return err_code;
        }
    }

    if (found)
        return NRF_SUCCESS;
    else
        return NRF_ERROR_NOT_FOUND;

}



void persist_init(void)
{
    ret_code_t rc;

    rc=fds_register(fds_evt_handler);
    m_fds_done = 0;
    NRF_LOG_WARNING("fds_register=%d",rc);
	
    rc = fds_init();
	    NRF_LOG_WARNING("fds_init=%d",rc);
    APP_ERROR_CHECK(rc);


    wait_for_fds();

}


