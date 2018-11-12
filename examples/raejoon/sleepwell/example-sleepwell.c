#include "contiki.h"
#include "net/rime/rime.h"
#include "net/rime/sleepwell.h"
#include "sys/node-id.h"
#include "sys/etimer.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(example_sleepwell_process, "Sleepwell example");
AUTOSTART_PROCESSES(&example_sleepwell_process);

static struct sleepwell_conn sleepwell;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_sleepwell_process, ev, data)
{
	static struct etimer et;
  PROCESS_EXITHANDLER(sleepwell_close(&sleepwell);)
  PROCESS_BEGIN();
	

  sleepwell_open(&sleepwell, 129, 10*CLOCK_SECOND, node_id, NULL);
  sleepwell_start(&sleepwell);

  while (1) {
		etimer_set(&et, CLOCK_SECOND);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  PROCESS_END();
}
