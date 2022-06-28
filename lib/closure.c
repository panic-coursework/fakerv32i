#include "closure.h"

#include <stdarg.h>
#include <stdlib.h>

closure_t *closure_create (void (*call)(void *, va_list),
                           void *data) {
  closure_t *closure =
    (closure_t *) malloc(sizeof(closure_t));
  closure->call = call;
  closure->data = data;
  return closure;
}
void closure_free (closure_t *closure) {
  free(closure);
}

void closure_call (closure_t *closure, ...) {
  va_list args;
  va_start(args, closure);
  closure->call(closure->data, args);
  va_end(args);
}
