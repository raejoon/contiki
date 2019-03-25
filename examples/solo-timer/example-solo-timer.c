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
  int* task_id = (int*)ptr;
  printf("[Task] callback for task id: %d\n", *task_id);
}

static struct solo_task *task1, *task2, *task3;

PROCESS_THREAD(example_solo_timer_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  
  task1 = solo_timer_add(BEACON_INTERVAL, 0, ss_callback, &id1);
  printf("Solo timer task added: 1,%lu,%lu\n", 
         BEACON_INTERVAL, (long unsigned int)0);
  task2 = solo_timer_add(BEACON_INTERVAL, BEACON_INTERVAL/4, ss_callback, &id2);
  printf("Solo timer task added: 2,%lu,%lu\n", 
         BEACON_INTERVAL, BEACON_INTERVAL/4);
  task3 = solo_timer_add(BEACON_INTERVAL, BEACON_INTERVAL/2, ss_callback, &id3);
  printf("Solo timer task added: 3,%lu,%lu\n", 
         BEACON_INTERVAL, BEACON_INTERVAL/2);
  solo_task_start(task1);
  solo_task_start(task2);
  solo_task_start(task3);

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  PROCESS_END();
}
