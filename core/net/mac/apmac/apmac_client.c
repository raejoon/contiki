#include "net/netstack.h"
#include "packetbuf.h"
#include "node-id.h"

#include "net/mac/apmac/apmac_ap.h"
#include "net/mac/apmac/apmac_client.h"
#include "net/mac/apmac/apmac_csma.h"
#include <stdio.h>
#include <string.h>

#define EARLY_WINDOW (BEACON_INTERVAL / 10)
#define READY_WINDOW (BEACON_INTERVAL / 10)

static volatile uint8_t apmac_is_on = 0;
static volatile uint16_t my_apid;
static struct ctimer beacon_timer;
static struct ctimer awake_timer;

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static void
beacon_timer_callback(void *ptr)
{
  PRINTF("Waking up to receive beacon.\n");
  NETSTACK_RADIO.on();
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent_callback, void *ptr)
{
}
/*---------------------------------------------------------------------------*/
static void
send_list(mac_callback_t sent_callback, void *ptr, struct rdc_buf_list *list)
{
}
/*---------------------------------------------------------------------------*/
static void
awake_timer_callback(void *ptr)
{
  if (ctimer_expired(&beacon_timer)) return;

  NETSTACK_RADIO.off();
  PRINTF("Going to sleep.\n");
}
/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
  struct msg msgdata;
  struct msg *outmsg;

  NETSTACK_FRAMER.parse();
  memcpy(&msgdata, packetbuf_dataptr(), sizeof(struct msg));

  if (msgdata.node_id != my_apid) return;
  
  if (msgdata.type == beacon) {
    PRINTF("Received beacon (%u)\n", msgdata.node_id);
    ctimer_set(&beacon_timer, BEACON_INTERVAL - EARLY_WINDOW, 
               beacon_timer_callback, NULL);
    
    apmac_csma_wait();
    apmac_csma_prepare(ready);
    apmac_csma_send();
    PRINTF("Ready frame sent\n");

    ctimer_set(&awake_timer, READY_WINDOW, awake_timer_callback, NULL);
  } else if (msgdata.type == data) {
    PRINTF("Received data (%u).\n", msgdata.node_id);
    ctimer_set(&awake_timer, READY_WINDOW, awake_timer_callback, NULL);
  }

}
/*---------------------------------------------------------------------------*/
static int
turn_on(void)
{
  apmac_is_on = 1;
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
turn_off(int keep_radio_on)
{
  apmac_is_on = 0;
  if (apmac_is_on) {
    return NETSTACK_RADIO.on();
  } else {
    return NETSTACK_RADIO.off();
  }
}
/*---------------------------------------------------------------------------*/
void
apmac_init(void)
{
  int ind;

  turn_on();
  NETSTACK_RADIO.on();

  for (ind = 0; ind < NUM_APS; ++ind) {
    if (client_list[ind] == node_id) {
      my_apid = ap_list[ind];
      break;
    }
  }

  if (ind == NUM_APS) 
    printf("Warning: AP not found!\n");
  else
    printf("My AP is %u\n", my_apid);
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver apmac_client_driver = 
{
  "AP-MAC (Client)",
  apmac_init,
  send_packet,
  send_list,
  input_packet,
  turn_on,
  turn_off,
  channel_check_interval
};
