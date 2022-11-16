/*******************************
shadowCreate controler .
v0.1. create for       20191111     zk.xu

******************************/


#ifndef __XCG__
#define __XCG__

#include "m_rf.h"

#define XCG_T0  300
#define XCG_T1  2

typedef enum _requestType_t
{
	getMasterNordicVersionRequest = 1,
	setVibStateRequest,
	bondJoyStickRequest,
	disconnectJoyStickRequest,
	getJoyStickBondStateRequest,
	hostEnterDfuStateRequest,
	getLeftJoyStickProductNameRequest,
	getRightJoyStickProductNameRequest,
	getLeftJoyStickFwVersionRequest,
	getRightJoyStickFwVersionRequest,
	invalidRequest,
	setControllerSleepMode,
}requestType_t;

typedef struct _request_t
{
	struct _requestHead
	{
		uint8_t requestType:7;
		uint8_t needAck:1;  //1:need to ack 0:don't need to ack
	} requestHead;
	uint8_t requestData[3];
}request_t;

typedef struct _acknowledge_t
{
	struct _acknowledgeHead
	{
		uint8_t requestType:7;
		uint8_t ack:1;  //1:ack 0:not ack
	} acknowledgeHead;
	uint8_t acknowledgeData[3];
}acknowledge_t;


extern uint32_t gLeftjoyStick_fwVersion;
extern uint32_t gRightjoyStick_fwVersion;
extern uint32_t gLeftjoyStick_productNameID;
extern uint32_t gRightjoyStick_productNameID;
extern rf_info_t gInfo;
void xcg_init(void);


//void xcg_xfr(uint8_t *datain,uint8_t *dataout);

void xcg_start(void);

void xcg_set_freq(int i);
void sc_spiSlave_init(void);
int sc_spi_transfer_timer_init(void);
int sc_spi_transfer_timer_start(void);

#endif



