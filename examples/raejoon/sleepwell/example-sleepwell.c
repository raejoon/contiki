#include "contiki.h"
#include "net/rime/rime.h"
#include "net/rime/sleepwell.h"
#include "sys/node-id.h"
#include "dev/button-sensor.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(example_sleepwell_process, "Sleepwell example");
AUTOSTART_PROCESSES(&example_sleepwell_process);
/*---------------------------------------------------------------------------*/
uint16_t TOS_NODE_ID;
static struct sleepwell_conn sleepwell;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_sleepwell_process, ev, data)
{
	node_id = TOS_NODE_ID;

  PROCESS_EXITHANDLER(sleepwell_close(&sleepwell);)
  PROCESS_BEGIN();

	SENSORS_ACTIVATE(button_sensor);

  sleepwell_open(&sleepwell, 129, 10*CLOCK_SECOND, node_id, NULL);
  sleepwell_start(&sleepwell);

  while (1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
  }
  PROCESS_END();
}
