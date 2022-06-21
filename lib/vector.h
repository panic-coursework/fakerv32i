#pragma once

#include <stddef.h>

typedef struct vector_t vector_t;

vector_t *vector_create ();
void vector_destroy (vector_t *vec);
void vector_write (vector_t *vec, size_t ix, void *value);
void *vector_read (vector_t *vec, size_t ix);
size_t vector_push (vector_t *vec, void *value);
size_t vector_size (vector_t *vec);
