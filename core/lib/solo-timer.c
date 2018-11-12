#include <stdio.h>
#include "lib/solo-timer.h"
#include "lib/list.h"
#include "lib/random.h"
#include "sys/node-id.h"

#define MAX_NEIGHBORS 16
#define MAX_CHAINLEN 16
#define MAX_PACKETLEN 100

struct neighbor {
  struct neighbor *next;
  uint8_t addr;
  clock_time_t timestamp;
  clock_time_t interval;
};

MEMB(neighbors_memb, struct neighbor, MAX_NEIGHBORS);
LIST(neighbors_list);
struct neighbor *n;
/*---------------------------------------------------------------------------*/
static struct solo_timer *locst;
static clock_time_t loctime;
static char buffer[MAX_PACKETLEN];
static uint8_t pathvec[MAX_CHAINLEN];
static int pathvec_len = 0;
static uint8_t loopvec[MAX_CHAINLEN];
static int loopvec_len = 0;
static int need_to_reset = 0;
static uint8_t jitter;

static void fire(void *ptr);
static void emit_packet(void *ptr);
static int add_neighbor(const uint8_t src_id);
static int num_neighbor();
static int get_interval(const uint8_t src_id);
static int pco_adjust(struct solo_timer *st, uint8_t nsize, 
                      uint8_t recv_jitter, clock_time_t* eta);
static int find_loop();
/*---------------------------------------------------------------------------*/
uint8_t
solo_timer_set(struct solo_timer *st, struct broadcast_conn *broadcast)
{
  st->interval = SOLO_TIMER_INTERVAL;

  st->expiry = clock_time() + (random_rand() % st->interval);
  //st->expiry = (clock_time() / st->interval + 1) * st->interval 
  //              + st->interval / 4 * node_id;
  //printf("My clock time: %d, My expiry: %d\n", (int)clock_time(), (int)st->expiry);
  //st->expiry = clock_time() + st->interval;
  st->broadcast = broadcast;
  
  ctimer_set(&st->ct, st->expiry - clock_time(), fire, st);
  return 0;
}

uint8_t
solo_timer_rx(struct solo_timer *st, char* buffer, const int len)
{
  int i;
  char* ptr = buffer;
  uint8_t src_id = *(uint8_t *)ptr;
  ptr += 1;
  uint8_t recv_jitter = *(uint8_t *)ptr;
  ptr += 1;
  uint8_t nsize = *(uint8_t *)ptr;
  ptr += 1;
  pathvec_len = *(uint8_t *)ptr;
  ptr += 1;
  if (pathvec_len > 0) {
    memcpy(pathvec, ptr, pathvec_len);
    ptr += pathvec_len * sizeof(uint8_t);
  }
  loopvec_len = *(uint8_t *)ptr;
  ptr += 1;
  if (loopvec_len > 0) {
    memcpy(loopvec, ptr, loopvec_len);
    ptr += loopvec_len * sizeof(uint8_t);
  }

  printf("Received path vector: ");
  for (i = 0; i < pathvec_len; ++i) {
    printf("%d-", pathvec[i]);
  }
  printf("\n");

  printf("Received loop vector: ");
  for (i = 0; i < loopvec_len; ++i) {
    printf("%d-", loopvec[i]);
  }
  printf("\n");

  /* Enlist neighbor. */
  add_neighbor(src_id);

  /* Check if link is strong enough */
  i = get_interval(src_id);
  printf("Update interval %d %d\n", (int)src_id, i);
  if (4 * get_interval(src_id) > 5 * SOLO_TIMER_INTERVAL) {
    return 0;
  }
  printf("Received solo neighbor %d\n", (int)src_id);

  /* Check if I am in a loop and need to reset. */
  if ((loopvec_len > 0) && (loopvec[0] == node_id)) {
    pathvec_len = 0;
    need_to_reset = 1;
    return 0;
  }

  /* PCO dynamics */
  if (pco_adjust(st, nsize, recv_jitter, &loctime) == -1) {
    pathvec_len = 0;
    return 0;
  }
  else {
    pathvec[pathvec_len] = src_id;
    pathvec_len += 1;
    if (find_loop() == 0) {
      pathvec_len = 0;
      need_to_reset = 1;
    }
    else {
    printf("Delaying my broadcast\n");
    ctimer_set(&st->ct, loctime, fire, st);
    }
  }
  
  return 0;
}

static int
find_loop() {
  int i;
  for (i = 0; i < pathvec_len; ++i) {
    if (pathvec[i] == node_id)
      break;
  }
  if (i < pathvec_len - 3) {
    loopvec_len = pathvec_len - i;
    memcpy(loopvec, &pathvec[i], loopvec_len * sizeof(uint8_t));

    printf("Got loop vector: ");
    for (i = 0; i < loopvec_len; ++i) {
      printf("%d-", loopvec[i]);
    }
    printf("\n");
    return 0;
  }
  else {
    loopvec_len = 0;
    return -1;
  }
}

static int 
get_interval(const uint8_t src_id) {
  for (n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {
    if (n->addr == src_id) {
      return n->interval;
    }
  }
  return 2 * SOLO_TIMER_INTERVAL;
}

static int
add_neighbor(const uint8_t src_id) {
  // Search neighbor
  for (n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {
    if (n->addr == src_id) {
      break;
    }
  }

  // Adding new neighbor
  if (n == NULL) {
    n = memb_alloc(&neighbors_memb);
    if (n == NULL) {
      return -1;
    }
    n->addr = src_id;
    list_add(neighbors_list, n);
    n->interval = 2 * SOLO_TIMER_INTERVAL;
  }
  else {
    n->interval = n->interval * 80 / 100 + 
      (clock_time() - n->timestamp) * 20 / 100;
  }  
  // Update time
  n->timestamp = clock_time();
  return 0;
}

static int
num_neighbor() {
  int cnt = 0;
  for (n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {
    if (2 * n->interval < 3 * SOLO_TIMER_INTERVAL) {
      cnt += 1;
    }
  }
  printf("Solo neighbors: %d\n", cnt);
  return cnt;
}

static int
pco_adjust(struct solo_timer *st, uint8_t nsize, 
           uint8_t recv_jitter, clock_time_t* eta) {
  loctime = clock_time() - recv_jitter;
  clock_time_t distance = st->expiry - loctime;
  //nsize = 3; // for debugging loops

  if (distance * nsize * 100 >= st->interval * 96) {
    return -1;
  }

  clock_time_t target_time = loctime + st->interval / nsize;
  st->expiry = (5 * target_time + 5 * st->expiry) / 10;
  *eta = st->expiry - clock_time();
  return 0;
}

static void
fire(void *ptr)
{
  printf("Solo expiry\n");
  locst = (struct solo_timer *)ptr;

  //jitter = random_rand() % 20;
  //loctime = locst->expiry + jitter - clock_time();
  //ctimer_set(&locst->jt, loctime, emit_packet, locst);
  jitter = 0;
  emit_packet(locst);
  
  if (need_to_reset == 1) {
    locst->expiry += locst->interval / 2 + (random_rand() % locst->interval);
    need_to_reset = 0;
    printf("Reset timing\n");
  }
  else {
    locst->expiry += locst->interval; 
  }
  loctime = locst->expiry - clock_time();
  ctimer_set(&locst->ct, loctime, fire, locst);
}

static void
emit_packet(void *ptr) {
  printf("emit_packet\n");
  locst = (struct solo_timer *)ptr;
  
  char* curr = buffer;
  memset(buffer, 0, MAX_PACKETLEN);
  *curr = node_id;
  curr += sizeof(uint8_t);
  *curr = (uint8_t) jitter;
  curr += sizeof(uint8_t);
  *curr = num_neighbor() + 1;
  curr += sizeof(uint8_t);
  *curr = pathvec_len;
  curr += sizeof(uint8_t);
  if (pathvec_len > 0) {
    memcpy(curr, pathvec, pathvec_len * sizeof(uint8_t));
    curr += (pathvec_len * sizeof(uint8_t));
  }
  *curr = (loopvec_len > 1)? loopvec_len - 1 : 0;
  curr += sizeof(uint8_t);
  if (loopvec_len - 1 > 0) {
    memcpy(curr, &loopvec[1], (loopvec_len - 1) * sizeof(uint8_t));
    curr += ((loopvec_len - 1) * sizeof(uint8_t));
  }
  
  packetbuf_copyfrom(buffer, curr - buffer);
  broadcast_send(locst->broadcast);
}
