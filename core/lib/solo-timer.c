#include "lib/solo-timer.h"
#include "stdio.h"

static struct solo_timer st;

void solo_timer_service_start()
{
  solo_beacon_start(&st.beacon);
}

void 
solo_timer_init()
{
  solo_beacon_init(&st.beacon);
  solo_scheduler_init(&st.scheduler, &st);
#if SOLO_CONF_START_AT_BOOT
  solo_timer_service_start();
#endif
}

struct solo_task*
solo_timer_add(clock_time_t interval, clock_time_t offset,
               void (*callback)(void*), void* callback_args)
{
  return solo_scheduler_add(&st.scheduler, interval, offset, 
                            callback, callback_args);
}

void
solo_timer_remove(struct solo_task* task)
{
  solo_scheduler_remove(&st.scheduler, task);
}

void
solo_timer_start(struct solo_task* task)
{
  solo_task_start(task);
}

void
solo_timer_stop(struct solo_task* task)
{
  solo_task_stop(task);
}

void 
solo_timer_destroy()
{
  solo_beacon_destroy(&st.beacon);
  solo_scheduler_destroy(&st.scheduler);
}
