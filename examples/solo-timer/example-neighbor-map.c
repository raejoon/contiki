#include "contiki.h"
#include "lib/neighbor-map.h"
#include <stdio.h>

/* Expected output:
 * 1. id: 1, last_timestamp: 0, average_interval: 100
 * 2. id: 1, last_timestamp: 0, average_interval: 100
 *    id: 2, last_timestamp: 50, average_interval: 100
 * 3. id: 1, last_timestamp: 0, average_interval: 100
 *    id: 2, last_timestamp: 50, average_interval: 100
 *    id: 3, last_timestamp: 60, average_interval: 100
 * 4. id: 1, last_timestamp: 0, average_interval: 100
 *    id: 2, last_timestamp: 50, average_interval: 100
 *    id: 3, last_timestamp: 60, average_interval: 100
 *    id: 4, last_timestamp: 90, average_interval: 100
 * 5. id: 1, last_timestamp: 100, average_interval: 100
 *    id: 2, last_timestamp: 50, average_interval: 100
 *    id: 3, last_timestamp: 60, average_interval: 100
 *    id: 4, last_timestamp: 90, average_interval: 100
 * 6. id: 1, last_timestamp: 100, average_interval: 100
 *    id: 2, last_timestamp: 200, average_interval: 110
 *    id: 3, last_timestamp: 60, average_interval: 100
 *    id: 4, last_timestamp: 90, average_interval: 100
 * 7. id: 1, last_timestamp: 100, average_interval: 100
 *    id: 2, last_timestamp: 200, average_interval: 110
 *    id: 3, last_timestamp: 310, average_interval: 130
 *    id: 4, last_timestamp: 90, average_interval: 100
 * 8. id: 1, last_timestamp: 100, average_interval: 100
 *    id: 2, last_timestamp: 200, average_interval: 110
 */

PROCESS(example_neighbor_map_process, "Neighbor map example");
AUTOSTART_PROCESSES(&example_neighbor_map_process);

static const uint8_t neighbor_ids[7] = {1, 2, 3, 4, 1, 2, 3};
static const uint32_t last_timestamps[7] = {0, 50, 60, 90, 100, 200, 310};
static const uint32_t flush_time = 500;
static int state_cnt = 0;

PROCESS_THREAD(example_neighbor_map_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();
  
  while (1) 
  {
    etimer_set(&et, 4 * CLOCK_SECOND);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  
    if (state_cnt < 7) {
      printf("Update: id: %d, timestamp: %d\n",
             (int) neighbor_ids[state_cnt], (int) last_timestamps[state_cnt]);
      neighbor_map_update(neighbor_ids[state_cnt], last_timestamps[state_cnt]);
      neighbor_map_dump();
    } else if (state_cnt == 7) {
      printf("Flush, timestamp: %d\n", (int) flush_time);
      neighbor_map_flush(flush_time);
      neighbor_map_dump();
    }
    state_cnt++;

  }
  PROCESS_END();
}
