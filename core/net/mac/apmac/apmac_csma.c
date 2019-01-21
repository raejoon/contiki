#include "lib/random.h"
#include "net/netstack.h"
#include "packetbuf.h"
#include "node-id.h"
#include "net/mac/apmac/apmac_csma.h"
#include "dev/watchdog.h"

#define IFS 2
#define CW 32

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
void
apmac_csma_prepare(enum pkt_type type)
{
  struct msg *hdr;

  packetbuf_clear();
  packetbuf_set_datalen(sizeof(struct msg));
  hdr = packetbuf_dataptr();
  hdr->type = type;
  hdr->node_id = node_id;
  NETSTACK_FRAMER.create();
  packetbuf_compact();

  NETSTACK_RADIO.prepare(packetbuf_hdrptr(), packetbuf_totlen());
}
/*---------------------------------------------------------------------------*/
void
apmac_csma_send()
{
  PRINTF("Radio send\n");
  NETSTACK_RADIO.transmit(packetbuf_totlen());
}
/*---------------------------------------------------------------------------*/
static int
apmac_csma_channel_clear()
{
  int first_check, second_check;
  rtimer_clock_t t0;

  t0 = RTIMER_NOW();
  first_check = NETSTACK_RADIO.channel_clear();
  while (RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + RTIMER_SECOND/500)) {}
  second_check = NETSTACK_RADIO.channel_clear();

  return first_check & second_check;
}
/*---------------------------------------------------------------------------*/
void
apmac_csma_wait()
{
  int ifs_count, cw_count;
  watchdog_stop();

  ifs_count = IFS;
  PRINTF("Start IFS check\n");
  while (ifs_count > 0) {
    if (apmac_csma_channel_clear() == 1) {
      ifs_count--;
    } else {
      ifs_count = IFS;
    }
  }

  cw_count = random_rand() % CW;
  PRINTF("Start CW countdown (%d)\n", cw_count);
  while (cw_count > 0) {
    if (apmac_csma_channel_clear() == 1) cw_count--;
  }
  PRINTF("CW counter expired\n");
  watchdog_start();
}
/*---------------------------------------------------------------------------*/

