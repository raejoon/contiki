#include "lib/solo-neighbor.h"
#include "lib/solo-conf.h"
#include "lib/memb.h"
#include "stdio.h"

#define DEBUG 0

#define MAX_NEIGHBORS 32

#ifdef SOLO_CONF_BETA
#define BETA SOLO_CONF_BETA
#else
#define BETA 10
#endif

#ifdef SOLO_CONF_INTERVAL_THRESHOLD 
#define INTERVAL_THRESHOLD SOLO_CONF_INTERVAL_THRESHOLD
#else
#define INTERVAL_THRESHOLD 2*INTERVAL
#endif

#ifdef SOLO_CONF_TIMESTAMP_THRESHOLD
#define TIMESTAMP_THRESHOLD SOLO_CONF_TIMESTAMP_THRESHOLD
#else
#define TIMESTAMP_THRESHOLD 4*INTERVAL
#endif

MEMB(neighbor_memb, struct solo_neighbor, MAX_NEIGHBORS);

void 
solo_neighbor_init(struct solo_neighbor_map* neighbors)
{
  LIST_STRUCT_INIT(neighbors, neighbor_list);
}

static struct solo_neighbor*
solo_neighbor_find(struct solo_neighbor_map* neighbors, uint8_t id)
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
solo_neighbor_update(struct solo_neighbor_map* neighbors, 
                     uint8_t id, clock_time_t timestamp)
{
  struct solo_neighbor* n;
  int added = 0;
  uint32_t last_interval;
  
  n = solo_neighbor_find(neighbors, id);
  if (n == NULL) {
    n = memb_alloc(&neighbor_memb);
    if (n == NULL) return -1;
    added = 1;
    n->id = id;
    n->average_interval = BEACON_INTERVAL;
    list_add(neighbors->neighbor_list, n);
  } else {
    if (timestamp < n->last_timestamp) {
      last_interval = ((uint32_t) -1) - n->last_timestamp + timestamp + 1;
    }
    else {
      last_interval = timestamp - n->last_timestamp;
    }
    
    n->average_interval = 
      ((100 - BETA) * n->average_interval + BETA * last_interval) / 100;
  }
  
  n->last_timestamp = timestamp;
  return added;
}

int
solo_neighbor_flush(struct solo_neighbor_map* neighbors, 
                    clock_time_t current_time)
{
  struct solo_neighbor *curr, *next;
  int removed = 0;
  curr = (struct solo_neighbor *) list_head(neighbors->neighbor_list);
  while (curr != NULL) {
    next = (struct solo_neighbor*) list_item_next(curr);
    if (curr->average_interval > INTERVAL_THRESHOLD) {
      list_remove(neighbors->neighbor_list, curr);
      memb_free(&neighbor_memb, curr);
      removed = 1;
    } else if (curr->last_timestamp + TIMESTAMP_THRESHOLD < current_time) {
      list_remove(neighbors->neighbor_list, curr);
      memb_free(&neighbor_memb, curr);
      removed = 1;
    }
    curr = next;
  }
#if DEBUG
  solo_neighbor_dump(neighbors);
#endif
  return removed;
}

void 
solo_neighbor_dump(struct solo_neighbor_map* neighbors, int verbose)
{
  struct solo_neighbor* curr;
  curr = (struct solo_neighbor *) list_head(neighbors->neighbor_list);
  if (verbose == 0) printf("(");
  while (curr != NULL) {
    if (verbose == 0) {
      printf("%u,", (unsigned int) curr->id);
    } else {
      printf("id: %u, last_timestamp: %u, average_interval: %u\n",
             (unsigned int) curr->id, 
             (unsigned int) curr->last_timestamp, 
             (unsigned int) curr->average_interval);
    }
    curr = (struct solo_neighbor*) list_item_next(curr);
  }
  if (verbose == 0) printf(")\n");
}

int solo_neighbor_size(struct solo_neighbor_map* neighbors)
{
  return list_length(neighbors->neighbor_list);
}

clock_time_t 
solo_neighbor_next(struct solo_neighbor_map* neighbors, clock_time_t offset)
{
  clock_time_t cand_offset, cand_diff;
  clock_time_t neighbor_offset, neighbor_diff;
  
  cand_offset = offset;
  cand_diff = BEACON_INTERVAL;

  struct solo_neighbor* curr;
  for (curr = (struct solo_neighbor*) list_head(neighbors->neighbor_list);
       curr != NULL;
       curr = (struct solo_neighbor*) list_item_next(curr)) {
    neighbor_offset = curr->last_timestamp % BEACON_INTERVAL;
    neighbor_diff = 
      (neighbor_offset + BEACON_INTERVAL - offset) % BEACON_INTERVAL;
    neighbor_diff = (neighbor_diff == 0)? BEACON_INTERVAL : neighbor_diff;

    if (neighbor_diff < cand_diff) {
      cand_offset = neighbor_offset;
      cand_diff = neighbor_diff;
    }
  }
  
  return cand_offset;
}

void solo_neighbor_destroy(struct solo_neighbor_map* neighbors)
{
  struct solo_neighbor* head;
  while ((head = list_head(neighbors->neighbor_list)) != NULL)
  {
      list_remove(neighbors->neighbor_list, head);
      memb_free(&neighbor_memb, head);
  }
}
