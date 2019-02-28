#include "contiki.h"
#include "lib/solo-conf.h"
#include "lib/solo-timer.h"
#include "lib/solo-scheduler.h"
#include <stdio.h>

PROCESS(example_solo_scheduler_process, "Solo scheduler example");
AUTOSTART_PROCESSES(&example_solo_scheduler_process);

static int id1 = 1;
static int id2 = 2;
static int id3 = 3;
static void ss_callback(void *ptr)
{
  int* id = (int*)ptr;
  printf("Scheduler callback! scheduler id: %d\n", *id);
}

static struct solo_timer st;  // dummy solo timer
static struct solo_scheduler ss1;
static struct solo_scheduler ss2;
static struct solo_scheduler ss3;


PROCESS_THREAD(example_solo_scheduler_process, ev, data)
{
  static struct etimer et;
  PROCESS_EXITHANDLER(solo_timer_destroy(&st);)

  PROCESS_BEGIN();
  
  st.beacon.beacon_offset = 0;
  ss1.st = &st;
  ss2.st = &st;
  ss3.st = &st;
  solo_scheduler_init(&ss1, 10*INTERVAL, 0, ss_callback, &id1);
  solo_scheduler_init(&ss2, 10*INTERVAL, INTERVAL, ss_callback, &id2);
  solo_scheduler_init(&ss3, 20*INTERVAL, 0, ss_callback, &id3);
  solo_scheduler_start(&ss1);
  solo_scheduler_start(&ss2);
  solo_scheduler_start(&ss3);

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  PROCESS_END();
}
