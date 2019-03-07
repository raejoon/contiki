#include "solo-pco.h"
#include "lib/solo-conf.h"

#include <assert.h>

#ifndef INTERVAL
#define INTERVAL BEACON_INTERVAL
#endif

clock_time_t
solo_pco_adjust(clock_time_t my_offset, uint8_t degree,
                struct solo_neighbor_map* neighbors)
{
  clock_time_t your_offset = clock_time() % INTERVAL;
  clock_time_t distance = (my_offset + INTERVAL - your_offset) % INTERVAL;
  
  degree = (degree == 0) ? 1 : degree;
  clock_time_t target_distance = INTERVAL / (degree + 1);
  
  if (distance >= target_distance) return 0;

#if SOLO_CONF_CLAMPING_ENABLE
  clock_time_t successor = solo_neighbor_next(neighbors, my_offset);
  clock_time_t succ_dist = (succesor + INTERVAL - my_offset) % INTERVAL;
  if (target_distance - distance > succ_dist)
    target_distance = distance + succ_dist;
  
  assert(target_distance <= INTERVAL / 2);
#endif

  clock_time_t new_offset = 
    (my_offset + (target_distance - distance) / 2) % INTERVAL;

  return (new_offset + INTERVAL - my_offset) % INTERVAL;
}
