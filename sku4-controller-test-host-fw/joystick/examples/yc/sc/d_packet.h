/*********************************************

shadowCreate controler .
v0.1. create        20191111           zk.xu


********************************************/


#ifndef __DPACKET__
#define __DPACKET__

/*v0.2.  add PKT_USER_SIZE*/

#define PKT_USER_SIZE



//typedef   char int8_t;


#define MAX_PACK_SIZE 32
#define MAX_DATA_SIZE 28


typedef struct
{
    uint32_t ts;
    uint8_t size;
    uint8_t data[MAX_DATA_SIZE];
    //uint8_t seq;
} d_packet_t;


typedef struct
{
    volatile int8_t c_head;
    volatile int8_t p_head;
    volatile int8_t packDS;
    d_packet_t  data[MAX_PACK_SIZE];
} cp_buffer_t;


void d_packet_init(void);
void d_packet_add(d_packet_t *pcur);
void d_packet_init_sync(void);
int d_packet_get_next(d_packet_t *pcur);
int d_packet_get_latest(d_packet_t *pcur);
int d_packet_get_size(void);
void  d_packet_skip_next(void);
#ifdef PKT_USER_SIZE
    int d_packet_get_next_usr(void *udata, uint32_t *pts);
#endif

void d_packet_get_nexta(int16_t *bufa);

//   test
typedef struct
{
    uint8_t data[8];
} __attribute((aligned(4)))  d_test_t;


#endif

