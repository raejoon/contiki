#ifndef NEIGHBOR_MAP_H_
#define NEIGHBOR_MAP_H_

#include "stdlib.h"
#include "lib/memb.h"
#include "lib/list.h"

#define MAX_NEIGHBORS 32

struct neighbor {
  struct neighbor* next;
  uint8_t id;
  uint32_t last_timestamp;
  uint32_t average_interval;
};

void neighbor_map_init(void);
int neighbor_map_update(uint8_t id, uint32_t timestamp);
void neighbor_map_flush(uint32_t timestamp);
void neighbor_map_dump(void);

#endif
