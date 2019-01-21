#ifndef APMAC_AP_H_
#define APMAC_AP_H_

#include "net/mac/rdc.h"

#define NUM_APS 3
#define BEACON_INTERVAL 10*CLOCK_SECOND

const uint16_t ap_list[NUM_APS] = {1, 3, 5};
const uint16_t client_list[NUM_APS] = {2, 4, 6};

extern const struct rdc_driver apmac_ap_driver;

#endif /* APMAC_AP_H_ */
