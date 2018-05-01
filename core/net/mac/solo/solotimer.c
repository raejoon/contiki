#include "contiki-conf.h"

#include "net/mac/solo/solotimer.h"
#include "net/netstack.h"
#include "sys/ctimer.h"
#include "sys/rtimer.h"
#include "sys/pt.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "net/queuebuf.h"

#ifdef CONTIKIMAC_CONF_CYCLE_TIME
#define CYCLE_TIME (CONTIKIMAC_CONF_CYCLE_TIME)
#else
#define CYCLE_TIME (CLOCK_SECOND / NETSTACK_RDC_CHANNEL_CHECK_RATE)
#endif

/* Number of 1 hop neighbors plus one. */
#ifdef NEIGHBOR_CONF_SIZE
#define NEIGHBOR_SIZE NEIGHBOR_CONF_SIZE
#else
#define NEIGHBOR_SIZE 2
#endif

/* Solo timer interval, in rtimer ticks. */
//#define SOLO_CYCLE_TIME (RTIMER_ARCH_SECOND)
#define SOLO_CYCLE_TIME (3 * CYCLE_TIME * NEIGHBOR_SIZE)

#ifdef SOLOTIMER_CONF_DEBUG
#define DEBUG SOLOTIMER_CONF_DEBUG
#else
#define DEBUG 0
#endif

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

#define MAX_QUEUED_PACKETS 3

MEMB(packet_memb, struct rdc_buf_list, MAX_QUEUED_PACKETS);
LIST(packet_list);

static struct ctimer ct;
static volatile uint8_t solotimer_is_on = 0;
static clock_time_t next_time;
static clock_time_t adjustment = 0;
static void timer_callback(void *ptr);


/*---------------------------------------------------------------------------*/
static void
packet_sent(void *ptr, int status, int num_transmissions)
{
  switch(status) {
    case MAC_TX_OK:
    case MAC_TX_NOACK:
      break;
    case MAC_TX_COLLISION:
      PRINTF("solotimer: COLLISION!\n");
    case MAC_TX_DEFERRED:
      break;
  }
}
static void
timer_callback(void *ptr)
{
  struct rdc_buf_list *q = list_head(packet_list);
  if (q != NULL) {
    PRINTF("solotimer: calling RDC to send.\n");
    
    queuebuf_to_packetbuf(q->buf);

    list_remove(packet_list, q);
    queuebuf_free(q->buf);
    memb_free(&packet_memb, q);
    //PRINTF("solotimer: free queued packet, queue length %d, free packets %d\n",
    //       list_length(packet_list), memb_numfree(&packet_memb));

    NETSTACK_RDC.send(packet_sent, ptr);
  }
  //else {
  //  PRINTF("solotimer: no packet to send.\n");
  //}
  if (adjustment == 0) {
    ctimer_reset(&ct);
  }
  else {
    ctimer_set(&ct, SOLO_CYCLE_TIME, timer_callback, NULL);
  }
  next_time = next_time + SOLO_CYCLE_TIME;

  adjustment = 0;
}
/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
  clock_time_t recv_time = clock_time();
  uint8_t addr[8];
  linkaddr_copy((linkaddr_t *)&addr, 
                packetbuf_addr(PACKETBUF_ADDR_SENDER)); 

  //PRINTF("%u\n", packetbuf_datalen());
  //PRINTF("solotimer: recv addr: ");
  //PRINTF("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
  //       longaddr[0], longaddr[1], longaddr[2], longaddr[3],
  //       longaddr[4], longaddr[5], longaddr[6], longaddr[7]);
  //

  //PRINTF("solotimer: recv time %lu, recv phase %lu\n", 
  //       recv_time, ((next_time - recv_time) % SOLO_CYCLE_TIME) * 100 / SOLO_CYCLE_TIME);
  
  clock_time_t target = recv_time + SOLO_CYCLE_TIME / NEIGHBOR_CONF_SIZE;
  if (next_time < target) {
    next_time = next_time / 2 + target / 2;
    adjustment = next_time - recv_time;
    
    PRINTF("solotimer: time until next send: %lu\n", CLOCK_SECOND);
    ctimer_set(&ct, adjustment, timer_callback, NULL);
  } 
  
  NETSTACK_LLSEC.input();
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  struct rdc_buf_list *q;
  
  q = memb_alloc(&packet_memb);
  if (q == NULL) {
    PRINTF("solotimer: could not allocate packet, dropping packet\n");
    mac_call_sent_callback(sent, ptr, MAC_TX_ERR, 1);
    return;
  }

  q->buf = queuebuf_new_from_packetbuf();
  if (q->buf == NULL) {
    memb_free(&packet_memb, q);
    PRINTF("solotimer: could not allocate packet, dropping packet\n");
    mac_call_sent_callback(sent, ptr, MAC_TX_ERR, 1);
    return;
  }

  list_add(packet_list, q);
  PRINTF("solotimer: send_packet, queue length %d, free packets %d\n",
         list_length(packet_list), memb_numfree(&packet_memb));

}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  if (solotimer_is_on == 0) {
    solotimer_is_on = 1;
    ctimer_set(&ct, SOLO_CYCLE_TIME, timer_callback, NULL);
    next_time = clock_time() + SOLO_CYCLE_TIME;
  }

  return NETSTACK_RDC.on();
}
/*---------------------------------------------------------------------------*/
static int
off(keep_radio_on)
{
  solotimer_is_on = 0;
  return NETSTACK_RDC.off(keep_radio_on);
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  if (NETSTACK_RDC.channel_check_interval) {
    return NETSTACK_RDC.channel_check_interval();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  memb_init(&packet_memb);
  
  ctimer_set(&ct, SOLO_CYCLE_TIME, timer_callback, NULL);
  next_time = clock_time() + SOLO_CYCLE_TIME;
  solotimer_is_on = 1;
}
/*---------------------------------------------------------------------------*/
const struct mac_driver solotimer_driver = {
  "solotimer_mac",
  init,
  send_packet,
  input_packet,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
