#ifndef SLEEPWELL_H_
#define SLEEPWELL_H_

#include "sys/ctimer.h"
#include "net/rime/broadcast.h"

struct sleepwell_conn;

struct sleepwell_callbacks {
  void (* recv)(struct sleepwell_conn *c, const linkaddr_t *from);
  void (* sent)(struct sleepwell_conn *c);
};

struct sleepwell_conn {
  struct broadcast_conn c;
  const struct sleepwell_callbacks *cb;
  struct ctimer timer;
  clock_time_t interval;
  uint16_t id;
};

void sleepwell_open(struct sleepwell_conn *c, uint16_t channel,
                    clock_time_t interval,
                    uint16_t id,
                    const struct sleepwell_callbacks *cb);
void sleepwell_close(struct sleepwell_conn *c);
void sleepwell_start(struct sleepwell_conn *c);

#endif  /* SLEEPWELL_H_ */
