#include "lib/neighbor-map.h"
#include "stdio.h"

#define BETA 20
#define INTERVAL_THRESHOLD 120
#define TIMESTAMP_THRESHOLD 400

MEMB(neighbor_memb, struct neighbor, MAX_NEIGHBORS);
LIST(neighbor_list);

void
neighbor_map_init(void)
{
  list_init(neighbor_list);
}

static struct neighbor*
neighbor_map_find(uint8_t id)
{
  struct neighbor* curr;
  curr = (struct neighbor *) list_head(neighbor_list);
  while (curr != NULL) {
    if (curr->id == id) return curr;
    curr = (struct neighbor *) list_item_next(curr);
  }
  return NULL;
}

int
neighbor_map_update(uint8_t id, uint32_t timestamp)
{
  struct neighbor* n;
  uint32_t last_interval;
  
  n = neighbor_map_find(id);
  if (n == NULL) {
    n = memb_alloc(&neighbor_memb);
    if (n == NULL) return -1;
    n->id = id;
    n->average_interval = 100;
    list_add(neighbor_list, n);
  } else {
    last_interval = timestamp - n->last_timestamp;
    n->average_interval = 
      ((100 - BETA) * n->average_interval + BETA * last_interval) / 100;
  }
  
  n->last_timestamp = timestamp;
  return 0;
}

void 
neighbor_map_flush(uint32_t current_time)
{
  struct neighbor *curr, *next;
  curr = (struct neighbor *) list_head(neighbor_list);
  while (curr != NULL) {
    next = (struct neighbor*) list_item_next(curr);
    if (curr->average_interval > INTERVAL_THRESHOLD) {
      list_remove(neighbor_list, curr);
    } else if (curr->last_timestamp + TIMESTAMP_THRESHOLD < current_time) {
      list_remove(neighbor_list, curr);
    }
    curr = next;
  }
}

void neighbor_map_dump(void)
{
  struct neighbor* curr;
  curr = (struct neighbor *) list_head(neighbor_list);
  printf("---- neighbor map ----\n");
  while (curr != NULL) {
    printf("id: %u, last_timestamp: %u, average_interval: %u\n",
           (unsigned int) curr->id, 
           (unsigned int) curr->last_timestamp, 
           (unsigned int) curr->average_interval);
    curr = (struct neighbor*) list_item_next(curr);
  }
  printf("----------------------\n");
}
