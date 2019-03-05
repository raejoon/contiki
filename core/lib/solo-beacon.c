#include "lib/solo-beacon.h"
#include "sys/node-id.h"
#include "stdio.h"
#include "lib/solo-conf.h"
#include "lib/solo-pco.h"

#define DEBUG 1

struct solo_beacon_data {
  uint8_t id;
  uint8_t degree;
};

static struct solo_beacon_data send_buf, recv_buf;
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);

static void
ctimer_callback(void* ptr)
{

#if DEBUG
  printf("Broadcast sent.\n");
#endif

  struct solo_beacon* sb = (struct solo_beacon*) ptr;

  sb->beacon_offset = clock_time() % INTERVAL;

  send_buf.id = sb->id;
  send_buf.degree = solo_neighbor_size(&sb->neighbors);
  packetbuf_copyfrom(&send_buf, sizeof(send_buf));
  broadcast_send(&(sb->broadcast));
  
  ctimer_stop(&(sb->ct));
  clock_time_t time_left = 
    (sb->beacon_offset + INTERVAL - clock_time() % INTERVAL) % INTERVAL;
  if (time_left == 0) time_left = INTERVAL;
  ctimer_set(&(sb->ct), time_left, ctimer_callback, sb);
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
  
  clock_time_t delay = solo_pco_adjust(sb->beacon_offset, recv_buf.degree);
  clock_time_t time_left = 
    (sb->beacon_offset + INTERVAL - clock_time() % INTERVAL) % INTERVAL;
  sb->beacon_offset = (sb->beacon_offset + delay) % INTERVAL;

  ctimer_stop(&(sb->ct));
  ctimer_set(&(sb->ct), time_left + delay, ctimer_callback, sb);
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
  ctimer_set(&(sb->ct), INTERVAL, ctimer_callback, sb);
}

void
solo_beacon_destroy(struct solo_beacon *sb)
{
  ctimer_stop(&(sb->ct));
  broadcast_close(&(sb->broadcast));
  solo_neighbor_destroy(&sb->neighbors);
}
