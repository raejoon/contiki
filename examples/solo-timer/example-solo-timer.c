#include "contiki.h"
#include "lib/solo-conf.h"
#include "lib/solo-timer.h"
#include <stdio.h>

PROCESS(example_solo_timer_process, "Solo timer example");
AUTOSTART_PROCESSES(&example_solo_timer_process);

static int id1 = 1;
static int id2 = 2;
static int id3 = 3;

static void ss_callback(void *ptr)
{
  int* id = (int*)ptr;
  printf("Solo timer callback! timer id: %d\n", *id);
}

static struct solo_task *task1, *task2, *task3;

PROCESS_THREAD(example_solo_timer_process, ev, data)
{
  static struct etimer et;
  PROCESS_EXITHANDLER(solo_timer_destroy();)

  PROCESS_BEGIN();
  
  solo_timer_init();
  task1 = solo_timer_add(10*INTERVAL, 0, ss_callback, &id1);
  task2 = solo_timer_add(10*INTERVAL, INTERVAL, ss_callback, &id2);
  task3 = solo_timer_add(20*INTERVAL, 0, ss_callback, &id3);
  solo_task_start(task1);
  solo_task_start(task2);
  solo_task_start(task3);

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  PROCESS_END();
}
