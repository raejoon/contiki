#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "net/rime/sleepwell.h"
#include "net/rime/rime.h"
#include "net/rime/neighbor.h"
#include "lib/random.h"

#define MAX_NEIGHBORS 16

struct msg {
  uint16_t id;
  char dummy[1];
};

struct gap {
  int start;
  int end;
  int size;
};

struct neighbor *n;
/*---------------------------------------------------------------------------*/
static void
send_beacon(struct sleepwell_conn *c)
{
  struct msg *hdr;
  
  packetbuf_clear();
  packetbuf_set_datalen(sizeof(struct msg));
  hdr = packetbuf_dataptr();
  hdr->id = c->id;
  broadcast_send(&c->c);
  printf("%d.%d: Sending sleepwell beacon with id %d\n",
         linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
         c->id);
}
/*---------------------------------------------------------------------------*/
static void
beacon_received(struct broadcast_conn* bc, const linkaddr_t *from)
{
  struct sleepwell_conn *c = (struct sleepwell_conn *) bc;
  struct msg buf;
  
  memcpy(&buf, packetbuf_dataptr(), sizeof(buf));
  printf("%d.%d: Sleepwell beacon received from %d.%d with id %d\n",
         linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
         from->u8[0], from->u8[1], buf.id);

  neighbor_add(buf.id);
}
/*---------------------------------------------------------------------------*/
int intcmp (const void *a, const void *b) {
  return (*(int*)a - *(int*)b);
}
/*---------------------------------------------------------------------------*/
void sort(int* arr, int size) {
  int i, j;
  i = 1;
  while (i < size) {
    j = i;   
    while (j > 0 && arr[j - 1] > arr[j]) {
      arr[j] = arr[j] + arr[j - 1];
      arr[j - 1] = arr[j] - arr[j - 1];
      arr[j] = arr[j] - arr[j - 1]; 
      j--;
    }
    i++;
  }
}
/*---------------------------------------------------------------------------*/
static struct gap
largest_gap(int interval)
{
  int offsets[MAX_NEIGHBORS];
  int s_ind, e_ind, cnt, size;
  struct gap max_gap = {0, 0, 0};
  
  cnt = 0;
  for (n = neighbor_head(); n != NULL; n = neighbor_next(n)) {
    offsets[cnt] = n->timestamp % interval;
    cnt += 1;
  }

  if (cnt == 1) {
    max_gap.start = offsets[0];
    max_gap.end = offsets[0] + interval;
    max_gap.size = interval;
    return max_gap;
  }
  
  sort(offsets, cnt);
  for (s_ind = 0; s_ind < cnt; ++s_ind) {
    e_ind = (s_ind == cnt - 1)? 0 : s_ind + 1;
    size = (offsets[e_ind] + interval - offsets[s_ind]) % interval;
    if (size > max_gap.size) {
      max_gap.start = offsets[s_ind];
      max_gap.end = offsets[e_ind];
      max_gap.size = size; 
    }
  }
  return max_gap;
}
/*---------------------------------------------------------------------------*/
static uint16_t
share(uint16_t my_offset, int interval) {
  uint16_t diff;
  uint16_t min_diff = interval;
  for (n = neighbor_head(); n != NULL; n = neighbor_next(n)) {
    diff = (n->timestamp % interval + interval - my_offset) % interval;
    printf("timestamp of %d: %u\n", n->id, (uint16_t)n->timestamp);
    printf("distance with %d: %d/%d\n", n->id, diff, interval);
    if (diff < min_diff)
      min_diff = diff;
  }
  return min_diff;
}

/*---------------------------------------------------------------------------*/
static int
adjust(uint16_t my_offset, int interval)
{
  int nsize, claim, prev_offset;
  struct gap max_gap;

  nsize = neighbor_size();
  if (nsize == 0)
    return -1;
  
  claim = interval / (nsize + 1);
  if (share(my_offset, interval) >= claim) {
    return -1;
  }
  
  prev_offset = my_offset;
  max_gap = largest_gap(interval);
  printf("Max gap: %d - %d (%d)\n", max_gap.start, max_gap.end, max_gap.size);

  if (max_gap.size >= claim * 2)
    my_offset = (max_gap.start + max_gap.size / 2) % interval;
  else
    my_offset = (max_gap.end + interval - claim) % interval;

  printf("Adjustment %d -> %d\n", prev_offset, my_offset);
  
  return my_offset;
}
/*---------------------------------------------------------------------------*/
static int
eta_from_offset(int offset, int expiry, int interval)
{
  int diff = offset - expiry % interval;
  if (2 * diff < interval / 2 && 2 * diff >= -interval)
    return interval + diff;
  else {
    if (diff < 0)
      return interval + (diff + interval);
    else {
      return interval + (diff - interval);
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
eta_from_current_time(uint16_t offset, int interval)
{
  int eta = (offset + interval - (clock_time() % interval)) % interval;
  if (2*eta > 3*interval) eta -= interval;
  else if (2*eta < interval) eta += interval;
  return eta;
}

/*---------------------------------------------------------------------------*/
static void
timer_callback(void *ptr)
{
  int interval, offset;
  struct sleepwell_conn *c = ptr;
  interval = (int) c->interval;

  send_beacon(c);
  
  offset = adjust(c->my_offset, interval);
  if (offset != -1) {
    c->my_offset = offset;
  }
  printf("my offset: %u\n", c->my_offset);
  
  ctimer_set(&c->timer, 
             eta_from_current_time(c->my_offset, c->interval), 
             timer_callback, c);
}
/*---------------------------------------------------------------------------*/
static CC_CONST_FUNCTION struct broadcast_callbacks broadcast_callbacks =
  {beacon_received, NULL};
/*---------------------------------------------------------------------------*/
void
sleepwell_open(struct sleepwell_conn *c, uint16_t channel,
               clock_time_t interval, 
               uint16_t id,
               const struct sleepwell_callbacks *cb)
{
  broadcast_open(&c->c, channel, &broadcast_callbacks);
  c->cb = cb;
  c->interval = interval;
  c->id = id;
}
/*---------------------------------------------------------------------------*/
void
sleepwell_close(struct sleepwell_conn *c) 
{
  broadcast_close(&c->c);
  ctimer_stop(&c->timer);
}
/*---------------------------------------------------------------------------*/
void
sleepwell_start(struct sleepwell_conn *c)
{
  int interval;
  c->expiry = clock_time() + c->interval + c->interval * c->id / 50;
  c->my_offset = c->expiry % c->interval;
  ctimer_set(&c->timer, 
             eta_from_current_time(c->my_offset, c->interval), 
             timer_callback, c); 
}
