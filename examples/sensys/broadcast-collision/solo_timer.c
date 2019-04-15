#include "contiki.h"
#include "net/rime/rime.h"
#include "sys/node-id.h"
#include "dev/leds.h"
#include "core/lib/random.h"

#include <stdio.h>

struct msg {
  uint16_t node_id;
  uint16_t seq_num;
  uint16_t padding[32];
};

/*---------------------------------------------------------------------------*/
PROCESS(solo_timer_process, "solo_timer example");
AUTOSTART_PROCESSES(&solo_timer_process);
/*---------------------------------------------------------------------------*/
static struct msg send_msg;
static struct msg recv_msg;

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  memcpy(&recv_msg, packetbuf_dataptr(), sizeof(recv_msg));
  printf("broadcast message received from %u (%d)\n", 
         recv_msg.node_id, recv_msg.seq_num);
  
  if (recv_msg.seq_num > send_msg.seq_num) {
    send_msg.seq_num = recv_msg.seq_num;
  }
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;

PROCESS_THREAD(solo_timer_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();
  send_msg.seq_num = (node_id == 1)? 1 : 0;
  send_msg.node_id = node_id;
  
  broadcast_open(&broadcast, 129, &broadcast_call);
  etimer_set(&et, random_rand() % (CLOCK_SECOND * 32));
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_set(&et, (CLOCK_SECOND * 32 / 2) + 1 + random_rand() % (CLOCK_SECOND * 32 / 2));
  }

  PROCESS_END();
}
