#ifndef APMAC_CSMA_H_
#define APMAC_CSMA_H_

enum pkt_type {beacon, ready, data};

struct msg {
  enum pkt_type type;
  uint16_t node_id;
  uint16_t dummy[50];
};

void apmac_csma_prepare(enum pkt_type type);
void apmac_csma_send();
void apmac_csma_wait();

#endif
