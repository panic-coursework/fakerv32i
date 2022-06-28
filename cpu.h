#pragma once

#include "rv32i.h"

struct cpu_t {
  clk_t *clk;
  bus_t *cdb; // cdb_message_t
  branch_predictor_t *branch_predictor;
  inst_unit_t *inst_unit;
  alu_pool_t *alu;
  ls_queue_t *ls_queue;
  memory_t *mem;
  reg_store_t *reg_store;
  rob_unit_t *rob;
  rs_unit_t *rs;
};

cpu_t *cpu_create ();
void cpu_free (cpu_t *cpu);

void cpu_tick (cpu_t *cpu);
