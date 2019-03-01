#ifndef SOLO_SCHEDULER_H_
#define SOLO_SCHEDULER_H_

#include "sys/ctimer.h"

struct solo_timer;

struct solo_task {
  LIST_STRUCT(task_list);
  struct solo_scheduler* scheduler;
  struct ctimer ct;
  clock_time_t interval;
  clock_time_t offset;
  void (*callback)(void *ptr);
  void *callback_args;
};

struct solo_scheduler {
  struct solo_timer *st;  
  struct solo_task *tasks;
};

void solo_scheduler_init(struct solo_scheduler* ss, struct solo_timer *st);

struct solo_task* solo_scheduler_add(struct solo_scheduler* ss,
                   clock_time_t interval, clock_time_t offset,
                   void (*callback)(void*), void* callback_args);

void solo_scheduler_start(struct solo_task* task);
void solo_scheduler_stop(struct solo_task* task);

#endif
