#include "contiki.h"
#include "lib/solo-conf.h"
#include "lib/solo-timer.h"
#include "sys/node-id.h"
#include "dev/watchdog.h"
#include <stdio.h>

// LED1
#define P67_OUT() (P6DIR |= BV(7))
#define P67_SEL() (P6SEL &= ~BV(7))
#define P67_1() (P6OUT |= BV(7))
#define P67_0() (P6OUT &= ~BV(7))
#define P67_IS_1 (P6OUT & BV(7))
// LED2
#define P66_OUT() (P6DIR |= BV(6))
#define P66_SEL() (P6SEL &= ~BV(6))
#define P66_1() (P6OUT |= BV(6))
#define P66_0() (P6OUT &= ~BV(6))
#define P66_IS_1 (P6OUT & BV(6))
// LED3
#define P62_OUT() (P6DIR |= BV(2))
#define P62_SEL() (P6SEL &= ~BV(2))
#define P62_1() (P6OUT |= BV(2))
#define P62_0() (P6OUT &= ~BV(2))
#define P62_IS_1 (P6OUT & BV(2))
// SIG2
#define P23_IN() (P2DIR &= ~BV(3))
#define P23_SEL() (P2SEL &= ~BV(3))
#define P23_RISE() (P2IES &= ~BV(3))
#define P23_CLEAR() (P2IFG &= ~BV(3))
#define P23_TRIGGERED() (P2IFG & BV(3))


PROCESS(sync_process, "time sync experiment");
AUTOSTART_PROCESSES(&sync_process);

static clock_time_t start;
static int toggle = 0;
static struct ctimer ct;

static void callback(void *ptr)
{
  if (toggle == 0) {
    toggle = 1;
    P67_1();
  } else {
    toggle = 0;
    P67_0();
  }
  printf("timer callback. systime: %u, rtime: %u\n",
         (unsigned int) clock_time(), (unsigned int) rtimer_arch_now());
  ctimer_reset(&ct);
}

PROCESS_THREAD(sync_process, ev, data)
{
  static struct etimer et;
  
  PROCESS_BEGIN();
  watchdog_stop();

  // Enable LED gpio
  P67_OUT();
  P67_SEL();
  P67_0();

  // Enable timesync gpio
  P23_IN();
  P23_SEL();
  P23_RISE();
  P23_CLEAR();

  // Give time to gpio actuation (1 second)
  start = clock_time();
  while (clock_time() - start < CLOCK_SECOND) {};
  printf("Before clock init. systime: %u, rtime: %u\n",
         (unsigned int) clock_time(), (unsigned int) rtimer_arch_now());
  while (1) { if (P23_TRIGGERED()) break; }
  clock_zero();
  printf("After clock init. systime: %u, rtime: %u\n",
         (unsigned int) clock_time(), (unsigned int) rtimer_arch_now());
  
  ctimer_set(&ct, CLOCK_SECOND, callback, NULL);

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
