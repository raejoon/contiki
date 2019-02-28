#include "lib/solo-timer.h"

void 
solo_timer_init(struct solo_timer* st)
{
  solo_beacon_init(&st->beacon);
}

void 
solo_timer_start(struct solo_timer* st)
{
  solo_beacon_start(&st->beacon);
}

void 
solo_timer_destroy(struct solo_timer *st)
{
  solo_beacon_destroy(&st->beacon);
}
