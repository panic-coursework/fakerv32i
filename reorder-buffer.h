#pragma once

#include "rv32i.h"

#include <stdbool.h>

// TODO: private or public?
struct rob_payload_t {
  rob_op_t op;
  word_t value;
  rs_id_t rs;
  addr_t pc; // for dump

  // for ROB_REGISTER, ROB_BRANCH and ROB_JALR
  reg_id_t dest;
  // for ROB_STORE and ROB_JALR
  ls_size_t size;
  addr_t addr;
  bool addr_ready, value_ready;
  // for ROB_BRANCH
  addr_t fallback;
  bool predicted_take;
  // for ROB_JALR
  addr_t predicted_addr;
};

struct reorder_buffer_t {
  rob_id_t id;
  reg_mut_t *ready; // bool
  reg_mut_t *payload; // rob_payload_t
};

#define ROB_CAPACITY 16

struct rob_unit_t {
  reg_store_t *reg_store;
  ls_queue_t *ls_queue;
  inst_unit_t *inst_unit; // for force_pc()
  rs_unit_t *rs_unit; // for clear()
  queue_t *robs; // reorder_buffer_t
  bus_t *cdb; // for clear()
  memory_t *mem; // for clear()
  branch_predictor_t *branch_predictor; // for feedback
  long commit_count;
};

rob_unit_t *rob_unit_create (reg_store_t *regs,
                             ls_queue_t *queue,
                             inst_unit_t *inst_unit,
                             rs_unit_t *rs_unit,
                             memory_t *mem,
                             branch_predictor_t *bp,
                             bus_t *cdb,
                             clk_t *clk);
void rob_unit_free (rob_unit_t *rob_unit);

const reorder_buffer_t *rob_unit_find (rob_unit_t *unit,
                                       rob_id_t id);
bool rob_unit_full (rob_unit_t *unit);
reorder_buffer_t *rob_unit_acquire (rob_unit_t *unit);
void rob_unit_clear (rob_unit_t *unit);
