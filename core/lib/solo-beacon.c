#include "lib/solo-beacon.h"
#include "net/rime/broadcast.h"
#include "net/rime/rime.h"
#include "sys/ctimer.h"
#include "sys/node-id.h"
#include "stdio.h"
#include "lib/neighbor-map.h"
#include "lib/solo-conf.h"


struct solo_beacon_data {
  uint8_t id;
  uint8_t degree;
};

static struct ctimer ct;

static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;

static struct solo_beacon_data send_buf, recv_buf;

static void 
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("Broadcast received.\n");
  memcpy(&recv_buf, packetbuf_dataptr(), sizeof(recv_buf));
  neighbor_map_update(recv_buf.id, clock_time());
  neighbor_map_flush(clock_time());
  neighbor_map_dump();
}

void
solo_beacon_init(void)
{
  broadcast_open(&broadcast, 129, &broadcast_call); 
  neighbor_map_init();
}

void
ctimer_callback(void* ptr)
{
  send_buf.id = node_id;
  send_buf.degree = neighbor_map_size();
  packetbuf_copyfrom(&send_buf, sizeof(send_buf));
  broadcast_send(&broadcast);
  ctimer_reset(&ct);  
}

void
solo_beacon_start(void)
{
  ctimer_set(&ct, INTERVAL, ctimer_callback, NULL);
}

void
solo_beacon_delay(void)
{

}

void
solo_beacon_destroy(void)
{
  broadcast_close(&broadcast);
}
