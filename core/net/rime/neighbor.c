#include "net/rime/neighbor.h"
#include "lib/memb.h"
#include <stdio.h>

#define MAX_NEIGHBORS 30

MEMB(neighbors_memb, struct neighbor, MAX_NEIGHBORS);
LIST(neighbors_list);

static struct neighbor *n;
/*---------------------------------------------------------------------------*/

void neighbor_init() {
  memb_init(&neighbors_memb);
}

int neighbor_add(const uint8_t id) {
  n = neighbor_get(id);
  if (n == NULL) {
    n = memb_alloc(&neighbors_memb);
    if (n == NULL) {
      printf("neighbor_add: cannot alloc memory!\n");
      return -1;
    }
    n->id = id;
    list_add(neighbors_list, n); 
  }
  n->timestamp = clock_time();
  return 0;
}

struct neighbor* neighbor_get(const uint8_t id) {
  for (n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {
    if (n->id == id) {
      return n;
    }
  }
  return NULL;
}

int neighbor_size() {
  int cnt = 0;
  for (n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {
    cnt += 1;
  }
  return cnt;
}

struct neighbor* neighbor_head() {
  return list_head(neighbors_list);
}

struct neighbor* neighbor_next(struct neighbor* curr) {
  return list_item_next(curr);
}
