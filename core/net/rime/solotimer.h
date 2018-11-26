#ifndef SOLOTIMER_H_
#define SOLOTIMER_H_

#include "sys/ctimer.h"
#include "net/rime/broadcast.h"

struct solotimer_conn;

struct solotimer_callbacks {
  void (* recv)(struct solotimer_conn *c, const linkaddr_t *from);
  void (* sent)(struct solotimer_conn *c);
};

struct solotimer_conn {
  struct broadcast_conn c;
  const struct solotimer_callbacks *cb;
  struct ctimer timer;
  uint16_t my_offset;
  clock_time_t interval;
  uint16_t id;
  int is_myslot;
};

void solotimer_open(struct solotimer_conn *c, uint16_t channel,
                    clock_time_t interval,
                    uint16_t id,
                    const struct solotimer_callbacks *cb);
void solotimer_close(struct solotimer_conn *c);
void solotimer_start(struct solotimer_conn *c);

#endif /* SOLOTIMER_H_ */
