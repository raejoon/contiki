#include "lib/solo-task.h"
#include "lib/solo-timer.h"
#include "lib/solo-conf.h"

#define MAX_TASKS 8

// Memory block to allocate task objects
MEMB(task_memb, struct solo_task, MAX_TASKS);

static clock_time_t
solo_task_expiry(struct solo_task* task)
{
  clock_time_t offset = 
    (task->scheduler->st->beacon).beacon_offset * task->interval / INTERVAL 
      + task->offset;
  
  unsigned int round = clock_time() / task->interval + 1;
  clock_time_t expiry = round * task->interval + offset;

  return expiry;
}

static void
solo_task_callback(void* ptr)
{
  struct solo_task *task = (struct solo_task*) ptr;
  clock_time_t expiry = solo_task_expiry(task);
  task->callback(task->callback_args);
  ctimer_set(&(task->ct), expiry - clock_time(), solo_task_callback, task);
}

struct solo_task* 
solo_task_init(struct solo_scheduler* ss,
               clock_time_t interval, clock_time_t offset,
               void (*callback)(void*), void* callback_args)
{
  struct solo_task *task = memb_alloc(&task_memb);
  if (task == NULL) return NULL;
  task->scheduler = ss;
  task->interval = interval;
  task->offset = offset;
  task->callback = callback;
  task->callback_args = callback_args;
  return task;
}

void 
solo_task_start(struct solo_task* task)
{
  clock_time_t expiry = solo_task_expiry(task);
  ctimer_set(&(task->ct), expiry - clock_time(), solo_task_callback, task);
}

void 
solo_task_stop(struct solo_task* task) 
{
  ctimer_stop(&(task->ct));
}

void 
solo_task_destroy(struct solo_task* task)
{
  solo_task_stop(task);
  memb_free(&task_memb, task);
}
