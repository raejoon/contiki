#include "contiki.h"
#include "net/rime/neighbor.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(example_neighbor_process, "Neighbor example");
AUTOSTART_PROCESSES(&example_neighbor_process);
/*---------------------------------------------------------------------------*/
static int test_id = 0;
static struct neighbor *n;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_neighbor_process, ev, data)
{
  static struct etimer et;
  
  PROCESS_BEGIN();
  
  neighbor_init();
  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    neighbor_add(test_id); 
    test_id += 1;
    printf("Current neighbor size: %d\n", neighbor_size());
    n = neighbor_get(test_id - 1); 
    printf("Last id: %d, timestamp: %d\n", n->id, (int) n->timestamp);
  }
  PROCESS_END();
}
