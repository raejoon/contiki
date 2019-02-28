#include "contiki.h"
#include "lib/solo-timer.h"
#include <stdio.h>

PROCESS(example_solo_timer_process, "Solo timer example");
AUTOSTART_PROCESSES(&example_solo_timer_process);

static struct solo_timer st;

PROCESS_THREAD(example_solo_timer_process, ev, data)
{
  static struct etimer et;
  PROCESS_EXITHANDLER(solo_timer_destroy(&st);)

  PROCESS_BEGIN();
  
  solo_timer_init(&st);
  solo_timer_start(&st);

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  PROCESS_END();
}
