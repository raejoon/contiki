#ifndef SOLO_NEIGHBOR_H_
#define SOLO_NEIGHBOR_H_

#include "lib/list.h"
#include "contiki-conf.h"
#include "stdlib.h"

struct solo_neighbor {
  LIST_STRUCT(neighbor_list);
  uint8_t id;
  uint32_t last_timestamp;
  uint32_t average_interval;
};

void solo_neighbor_init(struct solo_neighbor* neighbors);
int solo_neighbor_update(struct solo_neighbor* neighbors, 
                         uint8_t id, clock_time_t timestamp);
void solo_neighbor_flush(struct solo_neighbor* neighbors, 
                         clock_time_t current_time);
void solo_neighbor_dump(struct solo_neighbor* neighbors);
int solo_neighbor_size(struct solo_neighbor* neighbors);
void solo_neighbor_destroy(struct solo_neighbor* neighbors);

#endif
