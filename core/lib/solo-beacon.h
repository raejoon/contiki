#ifndef SOLO_BEACON_H_
#define SOLO_BEACON_H_

#include "net/rime/rime.h"
#include "net/rime/broadcast.h"
#include "sys/ctimer.h"
#include "stdlib.h"

struct solo_beacon {
  uint8_t id;
  clock_time_t beacon_offset;
  
  struct ctimer ct;
  struct broadcast_conn broadcast;
  struct broadcast_callbacks broadcast_call;
};

void solo_beacon_init(struct solo_beacon *sb);
void solo_beacon_start(struct solo_beacon *sb);
void solo_beacon_delay(void);
void solo_beacon_destroy(struct solo_beacon *sb);

#endif