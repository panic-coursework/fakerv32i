#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "reorder-buffer.h"
#include "inst-unit.h"
#include "ls-queue.h"
#include "reg-store.h"
#include "lib/log.h"
#include "queue.h"
#include "reg.h"
#include "reservation-station.h"
#include "rv32i.h"

#define ROB_CAPACITY 16

bool _rob_is_mispredicted (rob_payload_t payload) {
  if (payload.op == ROB_BRANCH) {
    return payload.predicted_take == payload.actual_take;
  } else if (payload.op == ROB_JALR) {
    return payload.predicted_addr == payload.addr;
  }
  debug_log("bad payload op %d", payload.op);
  assert(false);
}
addr_t _rob_addr (rob_payload_t payload) {
  if (payload.op == ROB_BRANCH) return payload.fallback;
  if (payload.op == ROB_JALR) return payload.addr;
  debug_log("bad payload op %d", payload.op);
  assert(false);
}
void _rob_commit (const reorder_buffer_t *rob,
                  rob_unit_t *unit) {
  const rob_payload_t *data =
    rm_read(rob->payload, rob_payload_t);
  bool success = true;
  switch (data->op) {
    case ROB_BRANCH:
    case ROB_JALR:
    if (data->dest >= 0) { // JAL or JALR
      reg_store_set(unit->reg_store, data->dest,
                    data->value);
    }
    if (_rob_is_mispredicted(*data)) {
      inst_unit_force_pc(unit->inst_unit, _rob_addr(*data));
      reg_file_clear(unit->reg_store);
      rob_unit_clear(unit);
      rs_unit_clear(unit->rs_unit);
    }
    break;

    case ROB_REGISTER:
    reg_store_set(unit->reg_store, data->dest, data->value);
    break;

    case ROB_STORE:
    if (ls_queue_full(unit->ls_queue)) {
      success = false;
    } else {
      ls_queue_push(unit->ls_queue, (ls_queue_payload_t) {
        .op = LS_STORE,
        .size = data->size,
        .addr = data->addr,
        .value = data->value,
      });
    }
    break;

    default:
    debug_log("unknown rob op %d", data->op);
    assert(false);
  }
  if (success) {
    if (data->rs >= 0) {
      reg_mut_t *reg =
        reg_store_rob_id(unit->reg_store, data->rs);
      rob_id_t id = *rm_read(reg, rob_id_t);
      if (id == rob->id) rm_write(reg, rob_id_t) = 0;
    }
    rm_write(rob->ready, bool) = false;
    queue_pop(unit->robs);
  }
}
void _rob_unit_tick (void *state, ...) {
  rob_unit_t *unit = (rob_unit_t *) state;
  if (queue_empty(unit->robs)) return;
  const reorder_buffer_t *rob =
    rm_read(queue_first(unit->robs), reorder_buffer_t);
  if (!*rm_read(rob->ready, bool)) return;
  _rob_commit(rob, unit);
}
rob_unit_t *rob_unit_create (reg_store_t *regs,
                             ls_queue_t *queue,
                             inst_unit_t *inst_unit,
                             rs_unit_t *rs_unit,
                             clk_t *clk) {
  rob_unit_t *unit = (rob_unit_t *) unit;
  unit->reg_store = regs;
  unit->ls_queue = queue;
  unit->inst_unit = inst_unit;
  unit->rs_unit = rs_unit;
  unit->robs = queue_create(ROB_CAPACITY,
                            sizeof(reorder_buffer_t), clk);
  for (int i = 0; i < ROB_CAPACITY; ++i) {
    reorder_buffer_t *buf =
      &rm_write(queue_id(unit->robs, i), reorder_buffer_t);
    buf->id = i + 1;
    buf->payload =
      reg_mut_create(sizeof(rob_payload_t), clk);
  }
  // TODO: cdb related business
  return unit;
}
void rob_unit_free (rob_unit_t *unit) {
  for (int i = 0; i < ROB_CAPACITY; ++i) {
    reorder_buffer_t *buf =
      (reorder_buffer_t *) queue_id(unit->robs, i);
    reg_mut_free(buf->payload);
  }
  queue_free(unit->robs);
  free(unit);
}

const reorder_buffer_t *rob_unit_find (rob_unit_t *unit,
                                       rob_id_t id) {
  return rm_read(queue_id(unit->robs, id - 1),
    reorder_buffer_t);
}
bool rob_unit_full (rob_unit_t *unit) {
  return queue_full(unit->robs);
}
reorder_buffer_t *rob_unit_acquire (rob_unit_t *unit) {
  reg_mut_t *rm = queue_push(unit->robs);
  if (!rm) return NULL;
  return &rm_write(rm, reorder_buffer_t);
}
void rob_unit_clear (rob_unit_t *unit) {
  queue_clear(unit->robs);
}
