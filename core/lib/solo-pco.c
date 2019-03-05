#include "solo-pco.h"
#include "lib/solo-conf.h"

clock_time_t
solo_pco_adjust(clock_time_t my_offset, uint8_t degree)
{
  clock_time_t your_offset = clock_time() % INTERVAL;
  clock_time_t distance = (my_offset + INTERVAL - your_offset) % INTERVAL;
  
  degree = (degree == 0) ? 1 : degree;
  clock_time_t target_distance = INTERVAL / (degree + 1);
  
  if (distance >= target_distance) return 0;

  clock_time_t new_offset = 
    (my_offset + (target_distance - distance) / 2) % INTERVAL;

  return (new_offset + INTERVAL - my_offset) % INTERVAL;
}
