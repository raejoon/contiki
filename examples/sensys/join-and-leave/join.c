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

PROCESS(join_process, "node join experiment");
AUTOSTART_PROCESSES(&join_process);

static clock_time_t start;
static int toggle[3] = {0, 0, 0};
static int task_id[3] = {0, 1, 2};
static int interval[3] = {512, 2048, 1024};
static int ind;
static int second_count = 0;

static void callback(void *ptr)
{
  int tid = *(int*)ptr;
  printf("[Task] callback for task id: %d, systime: %u\n",
         tid, (unsigned int) clock_time());
  if (toggle[tid] == 0) {
    toggle[tid] = 1;
  } else {
    toggle[tid] = 0;
  }
}

static struct solo_task *task;

PROCESS_THREAD(join_process, ev, data)
{
  static struct etimer et;
  
  PROCESS_BEGIN();
  watchdog_stop();

  printf("Beacon interval: %d\n", (int)SOLO_CONF_INTERVAL);

  // Enable timesync gpio
  P23_IN();
  P23_SEL();
  P23_RISE();
  P23_CLEAR();

  // Give time to gpio actuation (1 second)
  start = clock_time();
  while (clock_time() - start < CLOCK_SECOND) {};
  while (1) { if (P23_TRIGGERED()) break; }
  clock_init();
  printf("Clock reset: %u\n", (unsigned int) clock_time());
 
  // Setup every node except 25
  if (node_id != 25) {
    solo_timer_service_start();
    for (ind = 0; ind < 2; ind++) {
      task = solo_timer_add(interval[ind], 0, callback, &(task_id[ind]));
      printf("Solo timer task added: %d, %d, %d\n",
             task_id[ind], interval[ind], 0);
      solo_task_start(task); 
    }
  }

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    
    if (second_count == 200 && node_id == 25) {
      solo_timer_service_start();
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    second_count += 1;
  }

  PROCESS_END();
}
