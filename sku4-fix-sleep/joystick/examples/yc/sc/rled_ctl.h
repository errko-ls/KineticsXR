#ifndef __RLEDCTL__
#define __RLEDCTL__

void rled_start(void);

void rled_stop(void);

void rled_init(void);

void rled_get(char *v);

void rled_deinit(void);

uint32_t rled_getraw(void);

void rled_setraw(uint32_t  v);

#endif

