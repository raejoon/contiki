#ifndef APMAC_AP_H_
#define APMAC_AP_H_

#include "net/mac/rdc.h"

#define NUM_APS 2

const uint16_t ap_list[NUM_APS] = {1, 3};
const uint16_t client_list[NUM_APS] = {2, 4};

struct beacon_msg {
  uint16_t node_id;
  uint16_t dummy;
};

extern const struct rdc_driver apmac_ap_driver;

#endif /* APMAC_AP_H_ */
