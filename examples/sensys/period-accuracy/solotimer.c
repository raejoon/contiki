#include "contiki.h"
#include "lib/solo-conf.h"
#include "lib/solo-timer.h"
#include "sys/ctimer.h"
#include "dev/watchdog.h"
#include <stdio.h>

#define P66_OUT() (P6DIR |= BV(6))
#define P66_SEL() (P6SEL &= ~BV(6))
#define P66_1() (P6OUT |= BV(6))
#define P66_0() (P6OUT &= ~BV(6))
#define P66_IS_1 (P6OUT & BV(6))

#define P67_OUT() (P6DIR |= BV(7))
#define P67_SEL() (P6SEL &= ~BV(7))
#define P67_1() (P6OUT |= BV(7))
#define P67_0() (P6OUT &= ~BV(7))
#define P67_IS_1 (P6OUT & BV(7))

#define P23_IN() (P2DIR &= ~BV(3))
#define P23_SEL() (P2SEL &= ~BV(3))
#define P23_RISE() (P2IES &= ~BV(3))
#define P23_CLEAR() (P2IFG &= ~BV(3))
#define P23_TRIGGERED() (P2IFG & BV(3))

PROCESS(baseline_accuracy_process, "baseline accuracy experiment");
AUTOSTART_PROCESSES(&baseline_accuracy_process);

static clock_time_t start;
static int toggle = 0;

static void callback(void *ptr)
{
  if (toggle == 0) {
    P67_1();
    toggle = 1;
  } else {
    P67_0();
    toggle = 0;
  }
}

static struct solo_task* task;
static int ind = 3;
static int tid_arr[4] = {1, 2, 3, 4};
static int interval_arr[4] = {32, 128, 512, 2048};

PROCESS_THREAD(baseline_accuracy_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  watchdog_stop();

  printf("Beacon interval: %d\n", (int)SOLO_CONF_INTERVAL);
  
  // Enable task gpio
  P67_OUT();
  P67_SEL();
  P67_0();
  
  // Enable timesync gpio
  P66_OUT();
  P66_SEL();
  P66_0();

  P23_IN();
  P23_SEL();
  P23_RISE();
  P23_CLEAR();
  
  // Give time to gpio actuation (1 second)
  start = clock_time();
  while (clock_time() - start < CLOCK_SECOND) {};
  while (1) { if (P23_TRIGGERED()) break; }
  clock_init();
  P66_1();
  printf("Clock reset: %u\n", (unsigned int) clock_time());

	solo_timer_service_start();
	
	task = solo_timer_add(interval_arr[ind], 0, callback, &(tid_arr[ind]));
	printf("Solo timer task added: %d, %d, %d\n",
				 tid_arr[ind], interval_arr[ind], 0);
	solo_task_start(task);

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
