#ifndef SOLOTIMER_H_
#define SOLOTIMER_H_

#include "net/mac/mac.h"
#include "dev/radio.h"

extern const struct mac_driver solotimer_driver;

const struct mac_driver *solotimer_init(const struct mac_driver *r);

#endif /* SOLOTIMER_H_ */
