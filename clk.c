#include "clk.h"

#include <stdlib.h>

#include "lib/closure.h"
#include "lib/vector.h"
#include "reg.h"
#include "rv32i.h"

clk_t *clk_create () {
  clk_t *clk = malloc(sizeof(clk_t));
  clk->callbacks = vector_create();
  clk->callbefores = vector_create();
  clk->clk = 0;
  return clk;
}
void clk_free (clk_t *clk) {
  closure_t *closure;
  vector_foreach(clk->callbacks, i, closure) {
    closure_free(closure);
  }
  vector_free(clk->callbacks);
  vector_foreach(clk->callbefores, i, closure) {
    closure_free(closure);
  }
  vector_free(clk->callbefores);
  free(clk);
}

void clk_add_callback (clk_t *clk, closure_t *callback) {
  vector_push(clk->callbacks, callback);
}
void clk_add_callbefore (clk_t *clk, closure_t *callback) {
  vector_push(clk->callbefores, callback);
}

long clk_get (clk_t *clk) {
  return clk->clk;
}
void clk_tick (clk_t *clk) {
  closure_t *callback;
  vector_foreach(clk->callbefores, i, callback) {
    closure_call(callback, clk);
  }
  ++clk->clk;
  vector_foreach(clk->callbacks, i, callback) {
    closure_call(callback, clk);
  }
}
