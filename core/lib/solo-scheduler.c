#include "lib/solo-timer.h"
#include "lib/solo-scheduler.h"
#include "lib/solo-conf.h"
#include <stdio.h>

void
solo_scheduler_init(struct solo_scheduler* ss, struct solo_timer *st)
{
  ss->st = st;
  LIST_STRUCT_INIT(ss, task_list); 
};

struct solo_task*
solo_scheduler_add(struct solo_scheduler* ss,
                   clock_time_t interval, clock_time_t offset,
                   void (*callback)(void*), void* callback_args)
{
  struct solo_task* task = 
    solo_task_init(ss, interval, offset, callback, callback_args);
  if (task == NULL) return NULL;

  list_add(ss->task_list, task);
  return task;
}

void
solo_scheduler_remove(struct solo_scheduler* ss, struct solo_task* task)
{
  list_remove(ss->task_list, task);
  solo_task_destroy(task);
}

void
solo_scheduler_destroy(struct solo_scheduler* ss)
{
  struct solo_task* head;
  while ((head = list_head(ss->task_list)) != NULL) 
  {
    solo_scheduler_remove(ss, head);
  }
}
