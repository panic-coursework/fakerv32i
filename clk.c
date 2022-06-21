#include "clk.h"

#include <stdlib.h>

#include "lib/vector.h"
#include "reg.h"
#include "rv32i.h"

clk_t *clk_create () {
  clk_t *clk = malloc(sizeof(clk_t));
  clk->regs = vector_create();
  clk->clk = 0;
  return clk;
}
void clk_destroy (clk_t *clk) {
  size_t sz = vector_size(clk->regs);
  for (size_t i = 0; i < sz; ++i) {
    reg_t *reg = (reg_t *) vector_read(clk->regs, i);
    reg_destroy(reg);
  }
  vector_destroy(clk->regs);
  free(clk);
}

void clk_add_reg (clk_t *clk, reg_t *reg) {
  vector_push(clk->regs, reg);
}

long clk_get (clk_t *clk) {
  return clk->clk;
}
void clk_next (clk_t *clk) {
  ++clk->clk;
  size_t sz = vector_size(clk->regs);
  for (size_t i = 0; i < sz; ++i) {
    reg_t *reg = (reg_t *) vector_read(clk->regs, i);
    reg_tick(reg);
  }
}
