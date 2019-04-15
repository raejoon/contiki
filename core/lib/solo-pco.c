#include "solo-pco.h"
#include "lib/solo-conf.h"

#include <assert.h>
#include <stdio.h>

#ifndef INTERVAL
#define INTERVAL BEACON_INTERVAL
#endif

#define DEBUG 1

clock_time_t
solo_pco_adjust(clock_time_t recv_time, clock_time_t my_offset, uint8_t degree,
                struct solo_neighbor_map* neighbors)
{
  clock_time_t your_offset = recv_time % INTERVAL;
  clock_time_t distance = (my_offset + INTERVAL - your_offset) % INTERVAL;
  
  degree = (degree == 0) ? 1 : degree;
  clock_time_t target_distance = INTERVAL / (degree + 1);
#if DEBUG
  printf("recv_time: %u, distance: %u, target distance: %u\n", (unsigned int)
      recv_time, (unsigned int) distance, (unsigned int) target_distance);
#endif
  
  if (distance >= target_distance) {
    return 0;
  }

#if SOLO_CONF_CLAMPING_ENABLE
  clock_time_t successor = solo_neighbor_next(neighbors, my_offset);
  clock_time_t succ_from_me = (successor + INTERVAL - my_offset) % INTERVAL;
  clock_time_t succ_dist = distance + succ_from_me;
  if (target_distance > succ_dist) {
#if DEBUG
    printf("PCO adjustment clamped from %u to %u\n", 
           (unsigned int) target_distance, (unsigned int) succ_dist);
#endif
    target_distance = succ_dist;
  } 

  assert(target_distance <= INTERVAL / 2);
#endif

  clock_time_t new_offset;
  if (target_distance > distance + 5) {
  new_offset = 
    (my_offset + (target_distance - distance) * 20 / 100) % INTERVAL;
  } else {
    new_offset = 
      (my_offset + (target_distance - distance) * 90 / 100) % INTERVAL;
  }
    

  return (new_offset + INTERVAL - my_offset) % INTERVAL;
}
