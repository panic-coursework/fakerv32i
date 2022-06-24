#pragma once

#include "rv32i.h"

struct reg_file_t {
  reg_mut_t *rob_id[REG_COUNT]; // rob_id_t
};

reg_file_t *reg_file_create (clk_t *clk);
void reg_file_free (reg_file_t *reg_file);
