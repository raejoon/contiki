#include "lib/solo-neighbor.h"
#include "lib/solo-conf.h"
#include "lib/memb.h"
#include "stdio.h"

#define MAX_NEIGHBORS 32
#define BETA 20
#define INTERVAL_THRESHOLD 2*INTERVAL
#define TIMESTAMP_THRESHOLD 4*INTERVAL

MEMB(neighbor_memb, struct solo_neighbor, MAX_NEIGHBORS);

void 
solo_neighbor_init(struct solo_neighbor* neighbors)
{
  LIST_STRUCT_INIT(neighbors, neighbor_list);
}

static struct solo_neighbor*
solo_neighbor_find(struct solo_neighbor* neighbors, uint8_t id)
{
  struct solo_neighbor* curr;
  curr = (struct solo_neighbor *) list_head(neighbors->neighbor_list);
  while (curr != NULL) {
    if (curr->id == id) return curr;
    curr = (struct solo_neighbor *) list_item_next(curr);
  }
  return NULL;
}

int 
solo_neighbor_update(struct solo_neighbor* neighbors, 
                     uint8_t id, clock_time_t timestamp)
{
  struct solo_neighbor* n;
  uint32_t last_interval;
  
  n = solo_neighbor_find(neighbors, id);
  if (n == NULL) {
    n = memb_alloc(&neighbor_memb);
    if (n == NULL) return -1;
    n->id = id;
    n->average_interval = INTERVAL;
    list_add(neighbors->neighbor_list, n);
  } else {
    last_interval = timestamp - n->last_timestamp;
    n->average_interval = 
      ((100 - BETA) * n->average_interval + BETA * last_interval) / 100;
  }
  
  n->last_timestamp = timestamp;
  return 0;
}

void 
solo_neighbor_flush(struct solo_neighbor* neighbors, clock_time_t current_time)
{
  struct solo_neighbor *curr, *next;
  curr = (struct solo_neighbor *) list_head(neighbors->neighbor_list);
  while (curr != NULL) {
    next = (struct solo_neighbor*) list_item_next(curr);
    if (curr->average_interval > INTERVAL_THRESHOLD) {
      list_remove(neighbors->neighbor_list, curr);
      memb_free(&neighbor_memb, curr);
    } else if (curr->last_timestamp + TIMESTAMP_THRESHOLD < current_time) {
      list_remove(neighbors->neighbor_list, curr);
      memb_free(&neighbor_memb, curr);
    }
    curr = next;
  }
}

void 
solo_neighbor_dump(struct solo_neighbor* neighbors)
{
  struct solo_neighbor* curr;
  curr = (struct solo_neighbor *) list_head(neighbors->neighbor_list);
  printf("---- neighbor map ----\n");
  while (curr != NULL) {
    printf("id: %u, last_timestamp: %u, average_interval: %u\n",
           (unsigned int) curr->id, 
           (unsigned int) curr->last_timestamp, 
           (unsigned int) curr->average_interval);
    curr = (struct solo_neighbor*) list_item_next(curr);
  }
  printf("----------------------\n");
}

int solo_neighbor_size(struct solo_neighbor* neighbors)
{
  return list_length(neighbors->neighbor_list);
}

void solo_neighbor_destroy(struct solo_neighbor* neighbors)
{
  struct solo_neighbor* head;
  while ((head = list_head(neighbors->neighbor_list)) != NULL)
  {
      list_remove(neighbors->neighbor_list, head);
      memb_free(&neighbor_memb, head);
  }
}
