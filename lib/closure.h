#pragma once

#include <stdarg.h>

typedef struct closure_t {
  void (*call)(void *, va_list);
  void *data;
} closure_t;

closure_t *closure_create (void (*call)(void *, va_list),
                           void *data);
void closure_free (closure_t *closure);

void closure_call (closure_t *closure, ...);
