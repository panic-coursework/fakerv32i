#pragma once

#include "lib/vector.h"
#include "rv32i.h"

struct clk_t {
  long clk;
  vector_t *regs;
};

clk_t *clk_create ();
void clk_destroy (clk_t *clk);
void clk_add_reg (clk_t *clk, reg_t *reg);

long clk_get (clk_t *clk);
void clk_next (clk_t *clk);
