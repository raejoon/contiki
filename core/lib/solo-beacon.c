#include "lib/solo-beacon.h"
#include "net/rime/broadcast.h"
#include "net/rime/rime.h"
#include "sys/ctimer.h"
#include "stdio.h"

#define INTERVAL CLOCK_SECOND

static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);

static struct ctimer ct;
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;


static void 
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast received.\n");
}

void
solo_beacon_init(void)
{
  broadcast_open(&broadcast, 129, &broadcast_call); 
}

void
solo_beacon_callback(void* ptr)
{
  packetbuf_copyfrom("Hello", 6);
  broadcast_send(&broadcast);
  ctimer_reset(&ct);  
}

void
solo_beacon_start(void)
{
  ctimer_set(&ct, INTERVAL, solo_beacon_callback, NULL);
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
