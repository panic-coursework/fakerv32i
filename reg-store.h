#pragma once

#include "rv32i.h"

#define REG_COUNT 32

struct reg_store_t {
  reg_mut_t *registers[REG_COUNT];
  reg_mut_t *rob_id[REG_COUNT]; // rob_id_t
  reg_mut_t *clear; // bool
};

reg_store_t *reg_store_create (clk_t *clk);
void reg_store_free (reg_store_t *reg);

word_t reg_store_get (reg_store_t *reg, reg_id_t id);
void reg_store_set (reg_store_t *reg, reg_id_t id,
                    word_t value);

reg_mut_t *reg_store_rob_id (reg_store_t *reg, reg_id_t id);
void reg_file_clear (reg_store_t *reg);
