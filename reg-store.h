#pragma once

#include "rv32i.h"

#define REG_COUNT 32

struct reg_store_t {
  reg_mut_t *registers[REG_COUNT];
};

reg_store_t *reg_store_create (clk_t *clk);
void reg_store_free (reg_store_t *reg);

word_t reg_store_get (reg_store_t *reg, int id);
void reg_store_set (reg_store_t *reg, int id, word_t value);
