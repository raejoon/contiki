#include "lib/solo-beacon.h"
#include "sys/node-id.h"
#include "stdio.h"
#include "lib/solo-conf.h"
#include "lib/solo-pco.h"

#define DEBUG 0

struct solo_beacon_data {
  uint8_t id;
  uint8_t degree;
};

static struct solo_beacon_data send_buf, recv_buf;
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);

static void
ctimer_callback(void* ptr)
{
  struct solo_beacon* sb = (struct solo_beacon*) ptr;

  sb->beacon_offset = clock_time() % INTERVAL;

  send_buf.id = sb->id;
  send_buf.degree = solo_neighbor_size(&sb->neighbors);
  packetbuf_copyfrom(&send_buf, sizeof(send_buf));
  broadcast_send(&(sb->broadcast));
  
  sb->beacon_expiry += INTERVAL;
  ctimer_set(&(sb->ct), sb->beacon_expiry - clock_time(), ctimer_callback, sb);
}


static void 
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  struct solo_beacon *sb = (struct solo_beacon *) c;
  memcpy(&recv_buf, packetbuf_dataptr(), sizeof(recv_buf));
  solo_neighbor_update(&sb->neighbors, recv_buf.id, clock_time());
  solo_neighbor_flush(&sb->neighbors, clock_time());

#if DEBUG
  printf("Broadcast received.\n");
  solo_neighbor_dump(&sb->neighbors);
#endif
  
  clock_time_t delay = solo_pco_adjust(sb->beacon_expiry, recv_buf.degree);
  sb->beacon_expiry += delay;
  ctimer_stop(&(sb->ct));
  ctimer_set(&(sb->ct), sb->beacon_expiry - clock_time(), ctimer_callback, sb);
}

void
solo_beacon_init(struct solo_beacon *sb)
{
  sb->id = node_id;

  sb->broadcast_call.recv = broadcast_recv;
  broadcast_open(&(sb->broadcast), 129, &(sb->broadcast_call)); 
  
  solo_neighbor_init(&sb->neighbors);
}

void
solo_beacon_start(struct solo_beacon *sb)
{
  sb->beacon_offset = clock_time() % INTERVAL;
  sb->beacon_expiry = clock_time() + INTERVAL;
  printf("after: %u\n", sb->beacon_expiry - clock_time());
  ctimer_set(&(sb->ct), sb->beacon_expiry - clock_time(), ctimer_callback, sb);
}

void
solo_beacon_destroy(struct solo_beacon *sb)
{
  broadcast_close(&(sb->broadcast));
  solo_neighbor_destroy(&sb->neighbors);
}
