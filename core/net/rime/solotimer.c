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

#define RANDOMSTART 1
#define CLAMPING 1
#define PATHVECTOR 1

#define MAX_NEIGHBORS 30
#define MAX_PVLEN 30

struct msg {
  uint16_t id;
  uint16_t nsize;
#if PATHVECTOR
  uint8_t pvlen;
  uint8_t lvlen;
  uint8_t pathvec[MAX_PVLEN];
  uint8_t loopvec[MAX_PVLEN];
#endif
  char dummy;
};

#if PATHVECTOR
static uint8_t pathvec[MAX_PVLEN];
static uint8_t loopvec[MAX_PVLEN];
static uint8_t pvlen;
static uint8_t lvlen;
#endif
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
#if PATHVECTOR
  hdr->pvlen = pvlen;
  hdr->lvlen = lvlen;
  memcpy(hdr->pathvec, pathvec, pvlen);
  memcpy(hdr->loopvec, loopvec, lvlen);  
  pvlen = 0;
  lvlen = 0;
#endif

  broadcast_send(&c->c);
  printf("Broadcast %d\n", c->id);

  if (c->started == 0) c->started = 1;
}
/*---------------------------------------------------------------------------*/
static int
adjust(uint16_t my_offset, int interval, uint16_t your_nsize)
{
  uint16_t your_offset, your_distance, target_offset, target_distance, gap;
#if CLAMPING 
  struct neighbor *predecessor;
  uint16_t pred_offset, pred_distance;
#endif

  your_offset = clock_time() % interval;
  your_distance = (my_offset + interval - your_offset) % interval;
  
  your_nsize = (your_nsize == 0)? 1 : your_nsize;
  target_distance = interval / (your_nsize + 1);
  if (your_distance >= target_distance) {
    return -1;
  }

  target_offset = (your_offset + target_distance) % interval;

  gap = (target_offset + interval - my_offset) % interval;

#if CLAMPING
  predecessor = neighbor_predecessor(my_offset, interval);
  pred_offset = predecessor->timestamp % interval;
  pred_distance = (pred_offset + interval - my_offset) % interval;
  gap = (gap < pred_distance)? gap : pred_distance;
#endif 

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
static int
eta_from_current_time_after_interval(uint16_t offset, int interval)
{
  int eta = (offset + interval - (clock_time() % interval)) % interval;
  return eta + interval;
}
/*---------------------------------------------------------------------------*/
static void
timer_callback(void *ptr)
{
  int interval, nsize, claim, eta;
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

  eta = eta_from_current_time(c->my_offset, c->interval);

#if PATHVECTOR
  if (c->need_reset == 1) {
    printf("RESET OFFSET\n");
    c->my_offset = random_rand() % interval;
    c->need_reset = 0;
    eta = eta_from_current_time_after_interval(c->my_offset, c->interval);
  }
#endif

  ctimer_set(&c->timer, eta,
             timer_callback, c);
}
/*---------------------------------------------------------------------------*/
#if PATHVECTOR
static int
array_index(uint8_t* arr, uint8_t len, uint8_t val) {
  int ind;
  for (ind = 0; ind < len; ind++) {
    if (arr[ind] == val) return ind;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int
check_and_store_loop_from_pathvec(uint8_t my_id) {
  int ind = array_index(pathvec, pvlen, my_id);
  if (ind == -1) return 0;
  
  memcpy(loopvec, &pathvec[ind + 1], pvlen - (ind + 1));
  lvlen = pvlen - (ind + 1);
  return 1;
}
#endif
/*---------------------------------------------------------------------------*/
static void
beacon_received(struct broadcast_conn *bc, const linkaddr_t *from)
{
  int nsize, claim, share, offset, eta;
  struct msg buf;
  struct solotimer_conn *c = (struct solotimer_conn *) bc;
  int interval = c->interval;

  int ind;
  
  if (c->started == 0) return;
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

#if PATHVECTOR
  if (array_index(buf.loopvec, buf.lvlen, c->id) == 0) {
    lvlen = buf.lvlen - 1;
    memcpy(loopvec, &buf.loopvec[1], lvlen);
    pvlen = 0;  // no need to forward pathvec if resetting
    PRINTF("LOOP RECEIVED, RESET\n");
    c->need_reset = 1;
    return;
  } 
#endif

  offset = adjust(c->my_offset, interval, buf.nsize);
  if (offset != -1) {
#if PATHVECTOR
    pvlen = buf.pvlen;
    memcpy(pathvec, buf.pathvec, pvlen);
    pathvec[pvlen] = (uint8_t) buf.id;
    pvlen += 1;
    
    if (check_and_store_loop_from_pathvec(c->id) == 1) {
      PRINTF("LOOP DETECTED, RESET\n");
      pvlen = 0;  // no need to forward pathvec if resetting
      c->need_reset = 1; 
      return;
    }
#endif

    eta = eta_from_current_time_immediate(c->my_offset, interval);
#if DEBUG
    PRINTF("ADJUSTMENT CAUSED BY %d\n", buf.id);
    PRINTF("SCHEDULE %d/%d\n", eta, interval);
#endif
    c->my_offset = offset;
    ctimer_set(&c->timer, eta, timer_callback, c);
  }

#if PATHVECTOR
  else {
    pvlen = 0;  // no need to forward pathvec if not pushed
  }
#endif
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
  c->started = 0;
  c->need_reset = 0;
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
  uint16_t expiry, eta;

#if RANDOMSTART 
  expiry = clock_time() + c->interval + random_rand() % c->interval;
#else
  expiry = clock_time() + c->interval + c->interval * c->id / 50;
#endif

  c->my_offset = expiry % c->interval;
  eta = eta_from_current_time_immediate(c->my_offset, c->interval);
  ctimer_set(&c->timer, eta, timer_callback, c);

#if DEBUG
  PRINTF("SCHEDULE %d/%d\n", eta, (int)c->interval);
#endif
}
