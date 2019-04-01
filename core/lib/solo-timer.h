#ifndef SOLO_TIMER_H
#define SOLO_TIMER_H_

#include "lib/solo-conf.h"
#include "lib/solo-task.h"
#include "lib/solo-beacon.h"
#include "lib/solo-scheduler.h"

struct solo_timer {
  struct solo_beacon beacon;
  struct solo_scheduler scheduler;
};

void solo_timer_init();
void solo_timer_destroy();

// API
void solo_timer_service_start();
struct solo_task* solo_timer_add(clock_time_t interval, clock_time_t offset,
                                 void (*callback)(void*), void* callback_args);
void solo_timer_remove(struct solo_task* task);
void solo_timer_start(struct solo_task* task);
void solo_timer_stop(struct solo_task* task);

#endif
