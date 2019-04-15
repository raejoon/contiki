#include "contiki.h"
#include "net/rime/rime.h"
#include "sys/clock.h"
#include "sys/ctimer.h"
#include "sys/rtimer.h"
#include "stdio.h"
#include "net/rime/timesynch.h"
#include "dev/watchdog.h"

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

#define P23_IN() (P2DIR &= ~BV(3))
#define P23_SEL() (P2SEL &= ~BV(3))
#define P23_IS_1 (P2OUT & BV(3))
#define P23_WAIT_FOR_1() do{} while (!P23_IS_1)
#define P23_IS_0 (P2OUT & ~BV(3))
#define P23_WAIT_FOR_0() do{} while (!P23_IS_0)
#define P23_1() (P2OUT |= BV(3))
#define P23_0() (P2OUT &= ~BV(3))
#define P23_IS_1 (P2OUT & BV(3))

#define INTERVAL (CLOCK_SECOND/4)

PROCESS(baseline_process, "timestamp-baseline experiment");
AUTOSTART_PROCESSES(&baseline_process);

static int count = 0;
static int toggle = 0;
static struct ctimer ct;
static struct etimer et;
/*
static struct process *selecting_proc;
static void sig_init(struct process *proc)
{
  P27_IN(); 
  P27_SEL();

  P2IES &= ~BV(7);
  P2IFG &= ~BV(7);

  selecting_proc = proc;
  if (proc != NULL)
    P2IE |= BV(7);
  else
    P2IE &= ~BV(7);
}

ISR(PORT2, __signal_interrupt)
{
  if (selecting_proc != NULL) {
    process_post(selecting_proc, PROCESS_EVENT_MSG, 36);
  }
  LPM4_EXIT;
}
*/

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
};
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
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

static clock_time_t start;
PROCESS_THREAD(baseline_process, ev, data)
{
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
  PROCESS_BEGIN();
  watchdog_stop();
  broadcast_open(&broadcast, 129, &broadcast_call);

  P67_OUT();
  P67_SEL();
  P67_0();

  P66_OUT();
  P66_SEL();
  P66_0();
  P66_1();

  P23_IN();
  P23_SEL();
  P2IES &= ~BV(3);
  P2IFG &= ~BV(3);
  
  start = clock_time();
  while (clock_time() - start < 3*CLOCK_SECOND) {};
  
  printf("P23 state: %d\n", P23_IS_1);
  while (1) {
    if (P2IFG & BV(3)) break;
  }
  clock_init();
  printf("P23 state: %d\n", P23_IS_1);
  P66_0();
  P66_1();

  printf("reset my clock: %u\n", (unsigned int) clock_time());


  ctimer_set(&ct, INTERVAL, callback, NULL);
   
  while (1) {
    etimer_set(&et, 2*CLOCK_SECOND);
    //packetbuf_copyfrom("Hello", 6);
    //broadcast_send(&broadcast);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  PROCESS_END();
}
