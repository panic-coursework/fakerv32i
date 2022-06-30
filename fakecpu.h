#pragma once

#include "rv32i.h"

typedef struct fakecpu_t {
  clk_t *clk;
  bus_t *cdb;
  memory_t *mem;
  word_t regs[32];
  addr_t pc;
} fakecpu_t;

fakecpu_t *fakecpu_create ();
void fakecpu_free (fakecpu_t *cpu);

void fakecpu_tick (fakecpu_t *cpu);
