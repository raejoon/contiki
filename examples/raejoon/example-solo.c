#include "contiki.h"
#include "net/rime/rime.h"
#include "lib/solo-timer.h"
#include "dev/button-sensor.h"
#include "sys/node-id.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(example_solo_process, "Solo timer example");
AUTOSTART_PROCESSES(&example_solo_process);
/*---------------------------------------------------------------------------*/
uint16_t TOS_NODE_ID;
static struct solo_timer st;
static struct broadcast_conn broadcast;

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast message received from %u\n",
         *(uint8_t*)packetbuf_dataptr());
  solo_timer_rx(&st, packetbuf_dataptr(), packetbuf_datalen());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_solo_process, ev, data)
{
  node_id = TOS_NODE_ID;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();
  
  solo_timer_set(&st, &broadcast);
  broadcast_open(&broadcast, 129, &broadcast_call);
  SENSORS_ACTIVATE(button_sensor);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
  }
  PROCESS_END();
}
