#include "string.h"
#include "solo-vector.h"
#include <stdio.h>

void 
solo_vector_init(struct solo_vector* vec)
{
  vec->start_ind = 0;
  vec->length = 0;
}

int
solo_vector_pop(struct solo_vector* vec)
{
  if (vec->length == 0) return -1;
  vec->start_ind = (vec->start_ind + 1) % BUFFER_SIZE;
  vec->length -= 1;
  return vec->buffer[(vec->start_ind - 1) % BUFFER_SIZE];
}

struct solo_vector*
solo_vector_copy(struct solo_vector* dest, struct solo_vector* src)
{
  return (struct solo_vector*) memcpy(dest, src, sizeof(struct solo_vector));
}

void
solo_vector_append(struct solo_vector* vec, uint8_t val)
{
  if (vec->length == BUFFER_SIZE) {
    vec->buffer[vec->start_ind % BUFFER_SIZE] = val;
    vec->start_ind = (vec->start_ind + 1) % BUFFER_SIZE;
  } else {
    vec->length += 1;
    vec->buffer[(vec->start_ind + vec->length - 1) % BUFFER_SIZE] = val;
  }
}

int 
solo_vector_find(struct solo_vector* vec, uint8_t val)
{
  int cnt = 0;
  while (cnt < vec->length) {
    int ind = (vec->start_ind + cnt) % BUFFER_SIZE;
    if (vec->buffer[ind] == val)
      return ind;
    cnt++;
  }
  return -1;
}

void 
solo_vector_dump(struct solo_vector* vec)
{
  int cnt = 0;
  while (cnt < vec->length) {
    int ind = (vec->start_ind + cnt) % BUFFER_SIZE;
    printf("%d ", vec->buffer[ind]);
    cnt++;
  }
  printf("\n");
}
