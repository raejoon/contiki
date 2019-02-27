#include "contiki.h"
#include "lib/solo-beacon.h"
#include <stdio.h>

PROCESS(example_solo_beacon_process, "Solo beacon example");
AUTOSTART_PROCESSES(&example_solo_beacon_process);

PROCESS_THREAD(example_solo_beacon_process, ev, data)
{
  static struct etimer et;
  PROCESS_EXITHANDLER(solo_beacon_destroy();)

  PROCESS_BEGIN();
  
  solo_beacon_init();
  solo_beacon_start();

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  PROCESS_END();
}
