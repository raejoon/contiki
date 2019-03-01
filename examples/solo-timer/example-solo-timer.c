#include "contiki.h"
#include "lib/solo-conf.h"
#include "lib/solo-timer.h"
#include <stdio.h>

PROCESS(example_solo_timer_process, "Solo timer example");
AUTOSTART_PROCESSES(&example_solo_timer_process);

static void ss_callback(void *ptr)
{
  struct solo_task** task_ptr = (struct solo_task**)ptr;
  printf("Solo timer callback for task: %p\n", *task_ptr);
}

static struct solo_task *task1, *task2, *task3;

PROCESS_THREAD(example_solo_timer_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  
  task1 = solo_timer_add(10*INTERVAL, 0, ss_callback, &task1);
  printf("Solo timer task added: %p\n", task1);
  task2 = solo_timer_add(10*INTERVAL, INTERVAL, ss_callback, &task2);
  printf("Solo timer task added: %p\n", task2);
  task3 = solo_timer_add(20*INTERVAL, 0, ss_callback, &task3);
  printf("Solo timer task added: %p\n", task3);
  solo_task_start(task1);
  solo_task_start(task2);
  solo_task_start(task3);

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  PROCESS_END();
}
