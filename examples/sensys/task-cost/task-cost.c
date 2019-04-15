#include "contiki.h"
#include "lib/solo-conf.h"
#include "lib/solo-timer.h"
#include <stdio.h>

#ifdef TASK_CONF_NUM
#define TASK_NUM TASK_CONF_NUM
#else
#define TASK_NUM 1
#endif 

#ifdef TASK_CONF_INTERVAL 
#define TASK_INTERVAL TASK_CONF_INTERVAL
#else
#define TASK_INTERVAL SOLO_CONF_INTERVAL
#endif 

PROCESS(task_cost_process, "cost-over-task experiment");
AUTOSTART_PROCESSES(&task_cost_process);

static void ss_callback(void *ptr)
{
  int* tid = (int*)ptr;
  printf("[Task] callback for task id: %d\n", *tid);
}

static struct solo_task *task;
static int task_id;
static int tid_arr[TASK_NUM];
static int second_count = 0;

PROCESS_THREAD(task_cost_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  printf("Beacon interval: %d\n", (int)SOLO_CONF_INTERVAL);
  
  for (task_id = 1; task_id < TASK_NUM + 1; task_id++) {
    tid_arr[task_id - 1] = task_id;
    task = solo_timer_add(TASK_INTERVAL, 0, ss_callback, &(tid_arr[task_id - 1]));
    printf("Solo timer task added: %d,%u,%d\n", 
           task_id, (unsigned int) TASK_INTERVAL, 0);
    solo_task_start(task);
  }

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    second_count += 1;
  }

  PROCESS_END();
}
