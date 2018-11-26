#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "net/rime/solotimer.h"
#include "net/rime/rime.h"
#include "net/rime/neighbor.h"
#include "lib/random.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define MAX_NEIGHBORS 16

struct msg {
  uint16_t id;
  uint16_t nsize;
  char dummy;
};

struct neighbor *n;
/*---------------------------------------------------------------------------*/
static void
send_beacon(struct solotimer_conn *c)
{
  struct msg *hdr;

  packetbuf_clear();
  packetbuf_set_datalen(sizeof(struct msg));
  hdr = packetbuf_dataptr();
  hdr->id = c->id;
  hdr->nsize = neighbor_size();
  broadcast_send(&c->c);
  printf("Broadcast %d\n", c->id);
}
/*---------------------------------------------------------------------------*/
static int
adjust(uint16_t my_offset, int interval, uint16_t your_nsize)
{
  uint16_t your_offset, your_distance, target_offset, target_distance, gap;

  your_offset = clock_time() % interval;
  your_distance = (my_offset + interval - your_offset) % interval;

  target_distance = interval / (your_nsize + 1);
  if (your_distance >= target_distance) {
    return -1;
  }
  
  target_offset = (your_offset + target_distance) % interval;
  gap = (target_offset + interval - my_offset) % interval;
  gap = gap / 2;
  my_offset = (my_offset + interval + gap) % interval;

  return my_offset;
}
/*---------------------------------------------------------------------------*/
static int
eta_from_current_time(uint16_t offset, int interval)
{
  int eta = (offset + interval - (clock_time() % interval)) % interval;
  if (2*eta > 3*interval) eta -=  interval;
  else if (2*eta < interval) eta +=  interval;
  return eta;
}
/*---------------------------------------------------------------------------*/
static int
eta_from_current_time_immediate(uint16_t offset, int interval)
{
  int eta = (offset + interval - (clock_time() % interval)) % interval;
  return eta;
}
/*---------------------------------------------------------------------------*/
static void
timer_callback(void *ptr)
{
  int interval, nsize, claim;
  struct solotimer_conn *c = ptr;
  interval = (int) c->interval;

  send_beacon(c);
  
  nsize = neighbor_size();
  printf("Nsize %d\n", nsize);

  if (c->is_myslot == 1) {
    claim = interval / (1 + nsize);
    printf("SlotLength %d/%d\n", interval, interval);
    printf("Deficit %d/%d/%d\n", claim - interval, claim, interval);
  }
  c->is_myslot = 1;

  ctimer_set(&c->timer,
             eta_from_current_time(c->my_offset, c->interval),
             timer_callback, c);
}
/*---------------------------------------------------------------------------*/
static void
beacon_received(struct broadcast_conn *bc, const linkaddr_t *from)
{
  int nsize, claim, share, offset;
  struct msg buf;
  struct solotimer_conn *c = (struct solotimer_conn *) bc;
  int interval = c->interval;
  
  memcpy(&buf, packetbuf_dataptr(), sizeof(buf));
  printf("Receive %d\n", buf.id);
  
  neighbor_add(buf.id);
  if (c->is_myslot == 1)  {
    nsize = neighbor_size();
    claim = interval / (1 + nsize);
    share = ((clock_time() % interval) + interval - c->my_offset) % interval;
    c->is_myslot = 0;

    printf("SlotLength %d/%d\n", share, interval);
    printf("Deficit %d/%d/%d\n", claim - share, claim, interval);
  }

  offset = adjust(c->my_offset, interval, buf.nsize);
  if (offset != -1) {
#if DEBUG
    PRINTF("ADJUSTMENT CAUSED BY %d\n", buf.id);
#endif
    c->my_offset = offset;
    ctimer_set(&c->timer,
               eta_from_current_time_immediate(c->my_offset, interval),
               timer_callback, c);
  }
}
/*---------------------------------------------------------------------------*/
static CC_CONST_FUNCTION struct broadcast_callbacks broadcast_callbacks = 
  {beacon_received, NULL};
/*---------------------------------------------------------------------------*/
void
solotimer_open(struct solotimer_conn *c, uint16_t channel,
               clock_time_t interval,
               uint16_t id,
               const struct solotimer_callbacks *cb)
{
  broadcast_open(&c->c, channel, &broadcast_callbacks);
  c->cb = cb;
  c->interval = interval;
  c->id = id;
}
/*---------------------------------------------------------------------------*/
void 
solotimer_close(struct solotimer_conn *c)
{
  broadcast_close(&c->c);
  ctimer_stop(&c->timer);
}
/*---------------------------------------------------------------------------*/
void
solotimer_start(struct solotimer_conn *c)
{
  uint16_t expiry;

  expiry = clock_time() + c->interval + c->interval * c->id / 50;
  c->my_offset = expiry % c->interval;
  ctimer_set(&c->timer,
             eta_from_current_time(c->my_offset, c->interval),
             timer_callback, c);
}
