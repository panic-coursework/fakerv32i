#pragma once

#include "rv32i.h"

#include <stdbool.h>

// TODO: private or public?
struct rob_payload_t {
  rob_op_t op;
  word_t value;
  rs_id_t rs;

  // for ROB_REGISTER, ROB_BRANCH and ROB_JALR
  reg_id_t dest;
  // for ROB_STORE and ROB_JALR
  ls_size_t size;
  addr_t addr;
  // for ROB_BRANCH
  addr_t fallback;
  bool predicted_take;
  bool actual_take;
  // for ROB_JALR
  addr_t predicted_addr;
};

struct reorder_buffer_t {
  rob_id_t id;
  reg_mut_t *ready; // bool
  reg_mut_t *payload; // rob_payload_t
};

#define ROB_UNIT_SIZE 16

struct rob_unit_t {
  reg_store_t *reg_store;
  ls_queue_t *ls_queue;
  queue_t *robs; // reorder_buffer_t
};

rob_unit_t *rob_unit_create (reg_store_t *regs,
                             ls_queue_t *queue, clk_t *clk);
void rob_unit_free (rob_unit_t *rob_unit);

const reorder_buffer_t *rob_unit_find (rob_unit_t *unit,
                                       rob_id_t id);
bool rob_unit_full (rob_unit_t *unit);
reorder_buffer_t *rob_unit_acquire (rob_unit_t *unit);
