#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "net/rime/sleepwell.h"
#include "net/rime/rime.h"
#include "lib/random.h"

struct msg {
  uint16_t id;
  char dummy[1];
};

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
}
/*---------------------------------------------------------------------------*/
static void
timer_callback(void *ptr)
{
  struct sleepwell_conn *c = ptr;
  clock_time_t interval = c->interval;

  send_beacon(c);
  
  ctimer_set(&c->timer, c->interval, timer_callback, c);
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
  ctimer_set(&c->timer, c->interval * c->id / 50 + c->interval, 
             timer_callback, c); 
}
