#ifndef SOLO_SCHEDULER_H_
#define SOLO_SCHEDULER_H_

#include "sys/ctimer.h"

struct solo_timer;

struct solo_scheduler {
  struct solo_timer *st;  
  struct ctimer task_timer;
  clock_time_t task_interval;
  clock_time_t task_offset;
  void (*callback)(void *ptr);
  void *callback_args;
};

void solo_scheduler_init(struct solo_scheduler* ss, 
                         clock_time_t interval, clock_time_t offset, 
                         void (*callback)(void*), void* callback_args);

void solo_scheduler_start(struct solo_scheduler* ss);
void solo_scheduler_stop(struct solo_scheduler* ss);

#endif
