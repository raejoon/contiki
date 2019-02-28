#ifndef SOLO_TIMER_H
#define SOLO_TIMER_H_

#include "lib/solo-beacon.h"

struct solo_timer {
  struct solo_beacon beacon;
};

void solo_timer_init(struct solo_timer* st);
void solo_timer_start(struct solo_timer* st);
void solo_timer_destroy(struct solo_timer* st);

#endif
