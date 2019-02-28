#include "lib/solo-timer.h"
#include "lib/solo-scheduler.h"
#include "lib/solo-conf.h"
#include <stdio.h>

void 
solo_scheduler_init(struct solo_scheduler* ss, 
                    clock_time_t interval, clock_time_t offset, 
                    void (*callback)(void*), void* callback_args)
{
  ss->task_interval = interval;
  ss->task_offset = offset; 
  ss->callback = callback;
  ss->callback_args = callback_args;
}

static clock_time_t
solo_scheduler_expiry(struct solo_scheduler* ss)
{
  clock_time_t offset = 
    (ss->st->beacon).beacon_offset * ss->task_interval / INTERVAL + ss->task_offset;

  unsigned int round = clock_time() / ss->task_interval + 1;
  clock_time_t expiry = round * ss->task_interval + offset;

  return expiry;
}

static void
solo_scheduler_callback(void* ptr)
{
  struct solo_scheduler *ss = (struct solo_scheduler*) ptr;
  clock_time_t expiry = solo_scheduler_expiry(ss);
  ss->callback(ss->callback_args);
  ctimer_set(&(ss->task_timer), expiry - clock_time(), solo_scheduler_callback, ss);
}

void 
solo_scheduler_start(struct solo_scheduler* ss)
{
  clock_time_t expiry = solo_scheduler_expiry(ss);
  ctimer_set(&(ss->task_timer), expiry - clock_time(), solo_scheduler_callback, ss);
}

void solo_scheduler_stop(struct solo_scheduler* ss)
{
  ctimer_stop(&(ss->task_timer));
}
