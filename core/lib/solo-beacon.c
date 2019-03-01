#include "lib/solo-beacon.h"
#include "sys/node-id.h"
#include "stdio.h"
#include "lib/neighbor-map.h"
#include "lib/solo-conf.h"

#define DEBUG 0

struct solo_beacon_data {
  uint8_t id;
  uint8_t degree;
};

static struct solo_beacon_data send_buf, recv_buf;
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);

static void 
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  memcpy(&recv_buf, packetbuf_dataptr(), sizeof(recv_buf));
  neighbor_map_update(recv_buf.id, clock_time());
  neighbor_map_flush(clock_time());
#if DEBUG
  printf("Broadcast received.\n");
  neighbor_map_dump();
#endif
}

static void
ctimer_callback(void* ptr)
{
  struct solo_beacon* sb = (struct solo_beacon*) ptr;

  sb->beacon_offset = clock_time() % INTERVAL;

  send_buf.id = sb->id;
  send_buf.degree = neighbor_map_size();
  packetbuf_copyfrom(&send_buf, sizeof(send_buf));
  broadcast_send(&(sb->broadcast));
  ctimer_reset(&(sb->ct));  
}

void
solo_beacon_init(struct solo_beacon *sb)
{
  sb->id = node_id;

  sb->broadcast_call.recv = broadcast_recv;
  broadcast_open(&(sb->broadcast), 129, &(sb->broadcast_call)); 

  neighbor_map_init();
}

void
solo_beacon_start(struct solo_beacon *sb)
{
  sb->beacon_offset = clock_time() % INTERVAL;
  ctimer_set(&(sb->ct), INTERVAL, ctimer_callback, sb);
}

void
solo_beacon_delay(void)
{

}

void
solo_beacon_destroy(struct solo_beacon *sb)
{
  broadcast_close(&(sb->broadcast));
}
