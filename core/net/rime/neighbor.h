#ifndef NEIGHBORS_H_
#define NEIGHBORS_H_

#include "contiki.h"
#include "lib/list.h"

struct neighbor {
  struct neighbor *next;
  uint8_t id;
  clock_time_t timestamp;
  clock_time_t interval;
};

void neighbor_init();
int neighbor_add(const uint8_t id);
struct neighbor* neighbor_get(const uint8_t id);
int neighbor_size();

struct neighbor* neighbor_head();
struct neighbor* neighbor_next(struct neighbor* curr);
struct neighbor* neighbor_predecessor(uint16_t offset, uint16_t interval);

#endif
