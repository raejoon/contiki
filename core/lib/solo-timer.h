#ifndef SOLO_TIMER_H_
#define SOLO_TIMER_H_

#include "sys/ctimer.h"
#include "net/rime/rime.h"

#define SOLO_TIMER_INTERVAL CLOCK_SECOND * 10

typedef void (*solo_timer_cb_t)(void *ptr);

struct solo_timer {
  clock_time_t interval;
  clock_time_t expiry;
  clock_time_t offset;
  int adjusted;
  
  struct ctimer ct;
  struct ctimer jt;
  struct broadcast_conn* broadcast;
};

uint8_t solo_timer_set(struct solo_timer *st, 
                       struct broadcast_conn* broadcast);
                       
uint8_t solo_timer_rx(struct solo_timer *st, char* buffer, const int len);


#endif
