#ifndef SOLO_PCO_H_
#define SOLO_PCO_H_

#include "contiki-conf.h"
#include "solo-neighbor.h"

clock_time_t solo_pco_adjust(clock_time_t expiry, uint8_t degree);

#endif
