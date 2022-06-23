#pragma once

#include "rv32i.h"

#include <stdbool.h>

// TODO: private or public?
struct rob_payload_t {
  rob_op_t op;
  word_t value;
  rs_id_t rs;

  // for ROB_REGISTER
  reg_id_t dest;
  // for ROB_STORE
  addr_t addr;
  // for ROB_BRANCH
  addr_t fallback;
  bool prediction;
  bool actual;
};

struct reorder_buffer_t {
  rob_id_t id;
  reg_mut_t *busy;
  reg_mut_t *payload;
};

#define ROB_UNIT_SIZE 16

struct rob_unit_t {
  reg_store_t *regs;
  memory_t *mem;
  reorder_buffer_t robs[ROB_UNIT_SIZE];
};

rob_unit_t *rob_unit_create (reg_store_t *regs,
                             memory_t *mem, clk_t *clk);
void rob_unit_free (rob_unit_t *rob_unit);
