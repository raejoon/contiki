#ifndef SOLO_TASK_H_
#define SOLO_TASK_H_

#include "lib/list.h"
#include "sys/ctimer.h"

struct solo_scheduler;

struct solo_task {
  LIST_STRUCT(task_list);
  struct solo_scheduler *scheduler;
  struct ctimer ct;
  clock_time_t interval;
  clock_time_t offset;
  void (*callback)(void *ptr);
  void *callback_args;
};

struct solo_task* solo_task_init(struct solo_scheduler* ss,
                                 clock_time_t interval, clock_time_t offset,
                                 void (*callback)(void*), void* callback_args);
void solo_task_start(struct solo_task* task);
void solo_task_stop(struct solo_task* task);
void solo_task_destroy(struct solo_task* task);

#endif
