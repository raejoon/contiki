#include "lib/solo-beacon.h"
#include "sys/node-id.h"
#include "stdio.h"
#include "lib/solo-conf.h"
#include "lib/solo-pco.h"
#include "sys/rtimer.h"
#include "lib/random.h"
#include <assert.h>

#define DEBUG 1

struct solo_beacon_data {
  uint8_t id;
  uint8_t degree;
#if SOLO_CONF_LOOP_DETECT_ENABLE
  struct solo_vector pathvec;
  struct solo_vector loopvec;
#else
  uint8_t padding[30];
#endif
  uint16_t solo_timestamp;
  uint16_t phy_timestamp;
};

static struct solo_beacon_data send_buf, recv_buf;
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);

static void
construct_packet(struct solo_beacon* sb, rtimer_clock_t now_rt)
{
  send_buf.id = sb->id;
  send_buf.degree = solo_neighbor_size(&sb->neighbors);
  send_buf.solo_timestamp = now_rt;
 
#if SOLO_CONF_LOOP_DETECT_ENABLE
  solo_vector_copy(&send_buf.pathvec, &sb->pathvec);
  solo_vector_copy(&send_buf.loopvec, &sb->loopvec);
#endif
  
  packetbuf_copyfrom(&send_buf, sizeof(send_buf));
  packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
                     PACKETBUF_ATTR_PACKET_TYPE_TIMESTAMP);
}

static void
ctimer_callback(void* ptr)
{
  rtimer_clock_t rtimer_now = rtimer_arch_now();
#if DEBUG
  printf("[solo-beacon] Broadcast send. rtimer: %u\n", (unsigned int)rtimer_now);
#endif
  struct solo_beacon* sb = (struct solo_beacon*) ptr;

  if (sb->accept == 0) sb->accept = 1;
  sb->beacon_offset = clock_time() % INTERVAL;

  construct_packet(sb, rtimer_now);
  broadcast_send(&(sb->broadcast));
  
  ctimer_stop(&(sb->ct));
  clock_time_t time_left;
  time_left = 
    (sb->beacon_offset + INTERVAL - clock_time() % INTERVAL) % INTERVAL;
  if (time_left == 0) time_left = INTERVAL;
  ctimer_set(&(sb->ct), time_left, ctimer_callback, sb);
}

static clock_time_t
calibrate_recv_time(clock_time_t recv_time_st)
{
  uint32_t recv_delay_rt = recv_buf.phy_timestamp;
  if (recv_buf.phy_timestamp < recv_buf.solo_timestamp) {
    recv_delay_rt += ((uint16_t)(-1) - recv_buf.solo_timestamp);
  } else {
    recv_delay_rt -= recv_buf.solo_timestamp;
  }
  clock_time_t recv_delay_st = recv_delay_rt * CLOCK_SECOND / RTIMER_SECOND;
  return recv_time_st - recv_delay_st;
}

static void 
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  struct solo_beacon *sb = (struct solo_beacon *) c;
  if (sb->accept != 1) return;

  clock_time_t recv_st = clock_time();
  memcpy(&recv_buf, packetbuf_dataptr(), sizeof(recv_buf));
  
#if DEBUG
  printf("[solo-beacon] Broadcast received from %u\n", recv_buf.id);
#endif
  
  clock_time_t delay = 0;
  recv_st = calibrate_recv_time(recv_st);
  solo_neighbor_update(&sb->neighbors, recv_buf.id, recv_st);
  solo_neighbor_flush(&sb->neighbors, recv_st);
#if SOLO_CONF_PCO_ENABLE
  delay = solo_pco_adjust(recv_st, sb->beacon_offset, 
                          recv_buf.degree, &sb->neighbors);
  delay = (delay < 5)? 0 : delay;
  sb->beacon_offset = (sb->beacon_offset + delay) % INTERVAL;

#if SOLO_CONF_LOOP_DETECT_ENABLE
  if (delay != 0) {
    solo_vector_copy(&sb->pathvec, &recv_buf.pathvec);
    solo_vector_dump(&sb->pathvec);
    int loop_start = solo_vector_find(&sb->pathvec, sb->id);
    if (loop_start != -1) {
      solo_beacon_init(sb);
      solo_beacon_start(sb);
      return;
    } else {
      solo_vector_append(&sb->pathvec, recv_buf.id);
    }
  }
  else {
    solo_vector_init(&sb->pathvec);
  }
#endif
#endif

  ctimer_stop(&(sb->ct));
  clock_time_t time_left = 
    (sb->beacon_offset + INTERVAL - clock_time() % INTERVAL) % INTERVAL;
  ctimer_set(&(sb->ct), time_left + delay, ctimer_callback, sb);
}

void
solo_beacon_init(struct solo_beacon *sb)
{
  sb->id = node_id;

  sb->broadcast_call.recv = broadcast_recv;
  broadcast_open(&(sb->broadcast), 129, &(sb->broadcast_call)); 
  
  solo_neighbor_init(&sb->neighbors);
#if SOLO_CONF_LOOP_DETECT_ENABLE
  solo_vector_init(&sb->pathvec);
  solo_vector_init(&sb->loopvec);
#endif

  sb->reset = 0;
  sb->accept = 0;
}

void
solo_beacon_start(struct solo_beacon *sb)
{
  ctimer_stop(&(sb->ct));
  sb->beacon_offset = random_rand() % INTERVAL;
  clock_time_t time_left = 
    (sb->beacon_offset + INTERVAL - clock_time() % INTERVAL) % INTERVAL;
  ctimer_set(&(sb->ct), time_left, ctimer_callback, sb);
  sb->accept = 0;
}

void
solo_beacon_destroy(struct solo_beacon *sb)
{
  ctimer_stop(&(sb->ct));
  broadcast_close(&(sb->broadcast));
  solo_neighbor_destroy(&sb->neighbors);
}
