#ifndef SOLO_NEIGHBOR_H_
#define SOLO_NEIGHBOR_H_

#include "lib/list.h"
#include "contiki-conf.h"
#include "stdlib.h"

struct solo_neighbor {
  struct solo_neighbor* next;
  uint8_t id;
  clock_time_t last_timestamp;
  clock_time_t average_interval;
};

struct solo_neighbor_map {
  LIST_STRUCT(neighbor_list);
};

void solo_neighbor_init(struct solo_neighbor_map* neighbors);
int solo_neighbor_update(struct solo_neighbor_map* neighbors, 
                         uint8_t id, clock_time_t timestamp);
void solo_neighbor_flush(struct solo_neighbor_map* neighbors, 
                         clock_time_t current_time);
void solo_neighbor_dump(struct solo_neighbor_map* neighbors);
int solo_neighbor_size(struct solo_neighbor_map* neighbors);
void solo_neighbor_destroy(struct solo_neighbor_map* neighbors);

#endif
