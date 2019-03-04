#ifndef SOLO_SCHEDULER_H_
#define SOLO_SCHEDULER_H_

#include "sys/ctimer.h"
#include "lib/solo-task.h"
#include "lib/list.h"

struct solo_timer;

struct solo_scheduler {
  struct solo_timer *st;  
  LIST_STRUCT(task_list);
};

void solo_scheduler_init(struct solo_scheduler* ss, struct solo_timer *st);

struct solo_task* solo_scheduler_add(struct solo_scheduler* ss,
                   clock_time_t interval, clock_time_t offset,
                   void (*callback)(void*), void* callback_args);

void solo_scheduler_remove(struct solo_scheduler* ss, struct solo_task* task);
void solo_scheduler_destroy(struct solo_scheduler* ss);
#endif
