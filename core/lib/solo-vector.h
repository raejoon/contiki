#ifndef SOLO_VECTOR_H_
#define SOLO_VECTOR_H_

#include "solo-conf.h"

#ifdef SOLO_VECTOR_SIZE
#define BUFFER_SIZE SOLO_VECTOR_SIZE
#else
#define BUFFER_SIZE 16
#endif

struct solo_vector {
  uint8_t start_ind;
  uint8_t length;
  uint8_t buffer[BUFFER_SIZE];
};

void solo_vector_init(struct solo_vector* vec);
int solo_vector_pop(struct solo_vector* vec);
struct solo_vector* solo_vector_copy(struct solo_vector* dest, 
                                     struct solo_vector* src);
void solo_vector_append(struct solo_vector* vec, uint8_t val);
int solo_vector_find(struct solo_vector* vec, uint8_t val);

#endif
