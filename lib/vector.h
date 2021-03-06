#pragma once

#include <stddef.h>

typedef struct vector_t vector_t;

vector_t *vector_create ();
void vector_free (vector_t *vec);
void vector_write (vector_t *vec, size_t ix, void *value);
void *vector_read (vector_t *vec, size_t ix);
#define v_read(vec, ix, type) \
  *(type *) vector_read(vec, ix)
size_t vector_push (vector_t *vec, void *value);
size_t vector_size (vector_t *vec);

void vector_random_shuffle (vector_t *vec);

#define vector_foreach(vec, i, var) \
  for (size_t i = 0; \
       _vector_foreach(vec, i, (void **) &var); \
       ++i)
int _vector_foreach (vector_t *vec, size_t i, void **var);
