#pragma once

typedef struct closure_t {
  void (*call)(void *, ...);
  void *data;
} closure_t;

closure_t *closure_create (void (*call)(void *, ...),
                           void *data);
void closure_free (closure_t *closure);

void closure_call (closure_t *closure, ...);
