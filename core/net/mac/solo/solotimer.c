#include "contiki-conf.h"

#include "net/mac/solo/solotimer.h"
#include "net/netstack.h"
#include "sys/ctimer.h"
#include "sys/rtimer.h"
#include "sys/pt.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "net/queuebuf.h"
#include <stdlib.h>

#ifdef CONTIKIMAC_CONF_CCA_SLEEP_TIME
#define CCA_SLEEP_TIME CONTIKIMAC_CONF_CCA_SLEEP_TIME
#else
#if RTIMER_ARCH_SECOND > 8000
#define CCA_SLEEP_TIME                     RTIMER_ARCH_SECOND / 2000
#else
#define CCA_SLEEP_TIME                     (RTIMER_ARCH_SECOND / 2000) + 1
#endif /* RTIMER_ARCH_SECOND > 8000 */
#endif /* CONTIKIMAC_CONF_CCA_SLEEP_TIME */

#ifdef CONTIKIMAC_CONF_CCA_CHECK_TIME
#define CCA_CHECK_TIME                     (CONTIKIMAC_CONF_CCA_CHECK_TIME)
#else
#define CCA_CHECK_TIME                     RTIMER_ARCH_SECOND / 8192
#endif

#ifdef CONTIKIMAC_CONF_CCA_COUNT_MAX_TX
#define CCA_COUNT_MAX_TX                   (CONTIKIMAC_CONF_CCA_COUNT_MAX_TX)
#else
#define CCA_COUNT_MAX_TX                   6
#endif

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

#define FAIR_SHARE (2 * CYCLE_TIME)

/* Solo timer interval, in rtimer ticks. */
//#define SOLO_CYCLE_TIME (RTIMER_ARCH_SECOND)
#define SOLO_CYCLE_TIME (NEIGHBOR_SIZE * FAIR_SHARE)

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
static volatile clock_time_t my_offset;
static volatile clock_time_t last_offset;
static volatile clock_time_t adjustment = 0;
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
/*---------------------------------------------------------------------------*/
static void
timer_callback(void *ptr)
{
  ctimer_set(&ct, SOLO_CYCLE_TIME, timer_callback, NULL);
  clock_time_t my_time = clock_time();
  my_offset = my_time % SOLO_CYCLE_TIME;

  struct rdc_buf_list *q = list_head(packet_list);
  if (q != NULL) {
    PRINTF("solotimer: firing: my time %lu\n", my_time);
    //PRINTF("solotimer: firing: my phase %lu\n", 
    //       my_offset % SOLO_CYCLE_TIME * 100 / SOLO_CYCLE_TIME);

    if (my_offset < last_offset) {
      my_offset = my_offset + SOLO_CYCLE_TIME - last_offset;
      last_offset = 0;
    }
    //PRINTF("solotimer: firing: phase diff %lu\n", 
    //       (my_offset - last_offset) % SOLO_CYCLE_TIME * 100 / SOLO_CYCLE_TIME);
   
    my_offset = my_time % SOLO_CYCLE_TIME;
    
    queuebuf_to_packetbuf(q->buf);

    list_remove(packet_list, q);
    queuebuf_free(q->buf);
    memb_free(&packet_memb, q);
    //PRINTF("solotimer: free queued packet, queue length %d, free packets %d\n",
    //       list_length(packet_list), memb_numfree(&packet_memb));

    NETSTACK_RDC.send(packet_sent, ptr);

  }

  adjustment = 0;
}
/*---------------------------------------------------------------------------*/
static unsigned long int
clocktick_to_ms(unsigned long int tick) {
  return tick * 1000 / CLOCK_SECOND;
}

/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
  clock_time_t curr_time = clock_time();
  unsigned char* delay_ptr = packetbuf_dataptr() + packetbuf_datalen() - 2;

  rtimer_clock_t delay_rt = *(rtimer_clock_t *)delay_ptr;
  delay_rt += (CCA_SLEEP_TIME + CCA_CHECK_TIME) * CCA_COUNT_MAX_TX;
  clock_time_t delay_ct = delay_rt / 256 + 1;

  //unsigned long int delay_ms = clocktick_to_ms(delay_ct);
  //PRINTF("Delay: %lu ms\n", delay_ms);


  /* Remove delay field. */
  packetbuf_set_datalen(packetbuf_datalen() - 2);

  clock_time_t recv_time = curr_time - delay_ct;
  if (curr_time < delay_ct) {
    PRINTF("calibration overflow\n");
  }

  uint8_t addr[8];
  linkaddr_copy((linkaddr_t *)&addr, 
                packetbuf_addr(PACKETBUF_ADDR_SENDER)); 

  //PRINTF("%u\n", packetbuf_datalen());
  //PRINTF("solotimer: recv addr: ");
  //PRINTF("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
  //       longaddr[0], longaddr[1], longaddr[2], longaddr[3],
  //       longaddr[4], longaddr[5], longaddr[6], longaddr[7]);
  //
  
  unsigned long int recv_offset = recv_time % SOLO_CYCLE_TIME;
  unsigned long int next_offset = my_offset;

  last_offset = recv_offset;

  PRINTF("actual recv offset: %lu\n", 
         curr_time % SOLO_CYCLE_TIME * 100 / SOLO_CYCLE_TIME);
  PRINTF("calibr recv offset: %lu\n", 
         recv_offset % SOLO_CYCLE_TIME * 100 / SOLO_CYCLE_TIME); 

  if (next_offset < recv_offset) {
    //PRINTF("solotimer: COUNTER overflow!\n");
    next_offset = next_offset + SOLO_CYCLE_TIME - recv_offset;
    recv_offset = 0;
  }
  
  unsigned long int target_offset = 
    (recv_offset + FAIR_SHARE) % SOLO_CYCLE_TIME;

  unsigned long int space = 
    (next_offset + SOLO_CYCLE_TIME - recv_offset) % SOLO_CYCLE_TIME;
  
  if (space < FAIR_SHARE) {
    PRINTF("current diff %lu / %lu\n", 
           (next_offset + SOLO_CYCLE_TIME - recv_offset) % SOLO_CYCLE_TIME,
           SOLO_CYCLE_TIME);

    PRINTF("recv   offset %lu / %lu\n", recv_offset, SOLO_CYCLE_TIME);
    PRINTF("next   offset %lu / %lu\n", next_offset, SOLO_CYCLE_TIME);
    PRINTF("target offset %lu / %lu\n", target_offset, SOLO_CYCLE_TIME);

    if (target_offset >= next_offset)
      next_offset = (next_offset + target_offset) / 2;
    else
      next_offset = (next_offset + SOLO_CYCLE_TIME + target_offset) / 2;

    my_offset = next_offset;

    PRINTF("new    offset %lu / %lu\n", next_offset, SOLO_CYCLE_TIME);
    PRINTF("delay  offset %lu / %lu\n", delay_ct, SOLO_CYCLE_TIME);

    adjustment = 
      (next_offset + 2 * SOLO_CYCLE_TIME - recv_offset - delay_ct) % SOLO_CYCLE_TIME;

    PRINTF("adjusted diff %lu / %lu\n", 
           (next_offset + 2 * SOLO_CYCLE_TIME - recv_offset) % SOLO_CYCLE_TIME,
           SOLO_CYCLE_TIME);
    if (next_offset < recv_offset)
      PRINTF("solotimer: COUNTER overflow on adjustment!\n");

    my_offset = (curr_time + adjustment) % SOLO_CYCLE_TIME;
    ctimer_set(&ct, adjustment, timer_callback, NULL);
    PRINTF("backoff %lu / %lu\n", adjustment, SOLO_CYCLE_TIME);
  }

  
  NETSTACK_LLSEC.input();
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  struct rdc_buf_list *q;
  
  /* Adding room for delay field. */
  packetbuf_set_datalen(packetbuf_datalen() + sizeof(uint16_t));
  
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
    my_offset = clock_time() % SOLO_CYCLE_TIME;
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

  srand((unsigned int)RTIMER_NOW());
  
  ctimer_set(&ct, SOLO_CYCLE_TIME, timer_callback, NULL);
  my_offset = clock_time() % SOLO_CYCLE_TIME;
  solotimer_is_on = 1;

  
  PRINTF("CCA TIME: %lu\n", 
         (unsigned long int)(CCA_SLEEP_TIME + CCA_CHECK_TIME));
  
  //PRINTF("solotimer: clock second %lu\n", CLOCK_SECOND);
  //PRINTF("solotimer: cycle %lu\n", CYCLE_TIME);
  //PRINTF("solotimer: solo cycle %lu\n", SOLO_CYCLE_TIME);
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
