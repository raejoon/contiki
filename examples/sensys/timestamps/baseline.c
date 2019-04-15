#include "contiki.h"
#include "sys/ctimer.h"
#include "sys/rtimer.h"
#include "stdio.h"

#define P67_OUT() (P6DIR |= BV(7))
#define P67_SEL() (P6SEL &= ~BV(7))
#define P67_1() (P6OUT |= BV(7))
#define P67_0() (P6OUT &= ~BV(7))
#define P67_IS_1 (P6OUT & BV(7))

#define P62_1() (P6OUT |= BV(2))
#define P62_0() (P6OUT &= ~BV(2))

#define P66_OUT() (P6DIR |= BV(6))
#define P66_SEL() (P6SEL &= ~BV(6))
#define P66_1() (P6OUT |= BV(6))
#define P66_0() (P6OUT &= ~BV(6))
#define P66_IS_1 (P6OUT & BV(6))

#define INTERVAL (CLOCK_SECOND/4)

PROCESS(baseline_process, "timestamp-baseline experiment");
AUTOSTART_PROCESSES(&baseline_process);

static int count = 0;
static int toggle = 0;
static struct ctimer ct;
static struct etimer et;

static void callback (void *ptr)
{
  if (count < 200) {
    if (toggle == 0) {
      P67_1();
      toggle = 1;
    } else {
      P67_0();
      toggle = 0;
    }
    printf("Count: %d | rtimer: %u | ctimer: %u\n",
           count, (unsigned int)RTIMER_NOW(), (unsigned int)clock_time());
  } else {
    P66_0();
  }
  count++;
  ctimer_reset(&ct);
}

PROCESS_THREAD(baseline_process, ev, data)
{
  PROCESS_BEGIN();
  P67_OUT();
  P67_SEL();
  P67_0();

  P66_OUT();
  P66_SEL();
  P66_0();
  P66_1();


  ctimer_set(&ct, INTERVAL, callback, NULL);
   
  while (1) {
    etimer_set(&et, 10*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  PROCESS_END();
}
