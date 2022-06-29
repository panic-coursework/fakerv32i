#pragma once

#include "rv32i.h"

#define REG_COUNT 32

struct reg_store_t {
  reg_mut_t *registers[REG_COUNT];
  reg_mut_t *rob_id_set[REG_COUNT]; // rob_id_t
  reg_mut_t *rob_id_clear_if[REG_COUNT]; // rob_id_t
  reg_t *rob_ids; // _rob_ids_t
  reg_mut_t *clear; // bool
};

reg_store_t *reg_store_create (clk_t *clk);
void reg_store_free (reg_store_t *reg);

word_t reg_store_get (reg_store_t *reg, reg_id_t id);
void reg_store_set (reg_store_t *reg, reg_id_t id,
                    word_t value);

rob_id_t reg_store_rob_id_get (reg_store_t *reg,
                               reg_id_t id);
void reg_store_rob_id_set (reg_store_t *reg, reg_id_t id,
                           rob_id_t value);
void reg_store_rob_id_clear (reg_store_t *reg, reg_id_t id,
                             rob_id_t value);
void reg_file_clear (reg_store_t *reg);
