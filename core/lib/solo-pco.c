#include "solo-pco.h"
#include "lib/solo-conf.h"

clock_time_t
solo_pco_adjust(clock_time_t expiry, uint8_t degree)
{
  clock_time_t current_time = clock_time();
  clock_time_t new_expiry;
  
  degree = (degree == 0) ? 1 : degree;
  clock_time_t target_expiry = current_time + INTERVAL / (degree + 1);
  
  if (expiry >= target_expiry) return 0;
  
  new_expiry = (expiry + target_expiry) / 2;

  return new_expiry - expiry;
}
