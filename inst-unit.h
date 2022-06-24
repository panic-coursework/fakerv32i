#pragma once

#include "rv32i.h"

struct inst_unit_t {
  reg_mut_t *pc;
  memory_t *mem;
  rs_unit_t *rs_unit;
  rob_unit_t *rob_unit;
  reg_file_t *reg_file;
  reg_store_t *reg_store;
  bus_t *cdb;
};

inst_unit_t *inst_unit_create (memory_t *mem,
                               rs_unit_t *rs_unit,
                               rob_unit_t *rob_unit,
                               reg_file_t *reg_file,
                               reg_store_t *reg_store,
                               bus_t *cdb,
                               clk_t *clk);
void inst_unit_free (inst_unit_t *unit);
