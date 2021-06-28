#include "contiki.h"
#include "lib/solo-conf.h"
#include "lib/solo-timer.h"
#include <stdio.h>

PROCESS(client_offset_process, "client offset experiment");
AUTOSTART_PROCESSES(&client_offset_process);

static int interval = 1024;
static int task_id[3] = {0, 1, 2};
static int offset[3] = {32, 256, 512};
static int ind;

static struct solo_task *task;

static void callback(void *ptr)
{
  int tid = *(int*)ptr;
  printf("[Task] callback for task id: %d, system: %u\n",
         tid, (unsigned int) clock_time());
}


PROCESS_THREAD(client_offset_process, ev, data)
{
  static struct etimer et;  
  PROCESS_BEGIN();

  printf("Beacon interval: %d\n", (int)SOLO_CONF_INTERVAL);

  solo_timer_service_start();
  for (ind = 0; ind < 3; ind++) {
    task = solo_timer_add(interval, offset[ind], callback, &(task_id[ind]));
    printf("dtimer added: %d, %d, %d\n", task_id[ind], interval, offset[ind]);
    solo_task_start(task);
  }

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
