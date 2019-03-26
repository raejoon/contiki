#ifndef SOLO_BEACON_H_
#define SOLO_BEACON_H_

#include "net/rime/rime.h"
#include "net/rime/broadcast.h"
#include "sys/ctimer.h"
#include "lib/solo-neighbor.h"
#include "lib/solo-vector.h"
#include "stdlib.h"

struct solo_beacon {
  struct broadcast_conn broadcast;
  struct broadcast_callbacks broadcast_call;

  uint8_t id;
  uint16_t seqno;
  clock_time_t beacon_offset;
  struct solo_neighbor_map neighbors;

#if SOLO_CONF_LOOP_DETECT_ENABLE
  struct solo_vector pathvec;
  struct solo_vector loopvec;
#endif 
  uint8_t reset;
  uint8_t accept;
  
  struct ctimer ct;
};

void solo_beacon_init(struct solo_beacon *sb);
void solo_beacon_start(struct solo_beacon *sb);
void solo_beacon_destroy(struct solo_beacon *sb);

#endif
