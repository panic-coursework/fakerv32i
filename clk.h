#pragma once

#include "lib/closure.h"
#include "lib/vector.h"
#include "rv32i.h"

struct clk_t {
  long clk;
  vector_t *callbacks;
  vector_t *regmuts;
  vector_t *regs;
};

clk_t *clk_create ();
void clk_free (clk_t *clk);
void clk_add_callback (clk_t *clk, closure_t *callback);
void clk_add_regmut (clk_t *clk, closure_t *callback);
void clk_add_reg (clk_t *clk, closure_t *callback);

long clk_get (clk_t *clk);
void clk_tick (clk_t *clk);

void clk_random_shuffle (clk_t *clk);
