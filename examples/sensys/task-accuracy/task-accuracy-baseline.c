#include "contiki.h"
#include "lib/solo-conf.h"
#include "lib/solo-timer.h"
#include "sys/ctimer.h"
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

PROCESS(task_accuracy_process, "task-accuracy experiment");
AUTOSTART_PROCESSES(&task_accuracy_process);

static struct ctimer ct;

static void ss_callback(void *ptr)
{
  int* tid = (int*)ptr;
  printf("[Task] callback for task id: %d\n", *tid);
  ctimer_reset(&ct);
}

static int task_id;
static int tid_arr[9];
static int interval_arr[9];
static unsigned int task_interval;

PROCESS_THREAD(task_accuracy_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  printf("Beacon interval: %d\n", (int)SOLO_CONF_INTERVAL);
  
  for (task_id = 1; task_id < 2; task_id++) {
    task_interval = 1 << ((task_id - 1) % 9 + 4); // 0.125s ~ 32s

    tid_arr[task_id - 1] = task_id;
    interval_arr[task_id - 1] = task_interval;

    ctimer_set(&ct, task_interval, ss_callback, &(tid_arr[task_id - 1]));
  
    printf("Solo timer task added: %d,%u,%d\n", 
           task_id, task_interval, 0);
  }

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
