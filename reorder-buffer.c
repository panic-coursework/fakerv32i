#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#include "clk.h"
#include "memory.h"
#include "reorder-buffer.h"
#include "bus.h"
#include "lib/closure.h"
#include "inst-unit.h"
#include "ls-queue.h"
#include "reg-store.h"
#include "lib/log.h"
#include "queue.h"
#include "reg.h"
#include "reservation-station.h"
#include "rv32i.h"

bool _rob_is_mispredicted (rob_payload_t payload) {
  if (payload.op == ROB_BRANCH) {
    return payload.predicted_take != payload.value;
  } else if (payload.op == ROB_JALR) {
    return payload.predicted_addr != (payload.addr & ~1u);
  }
  debug_log("bad payload op %d", payload.op);
  assert(false);
}
addr_t _rob_addr (rob_payload_t payload) {
  if (payload.op == ROB_BRANCH) return payload.fallback;
  if (payload.op == ROB_JALR) return payload.addr & ~1u;
  debug_log("bad payload op %d", payload.op);
  assert(false);
}
void _rob_commit (const reorder_buffer_t *rob,
                  rob_unit_t *unit) {
  const rob_payload_t *data =
    rm_read(rob->payload, rob_payload_t);
  debug_log("ROB %2d commits for addr %08x!", rob->id,
            data->pc);
#ifdef PRINT_PC
  printf("%08x\n", data->pc);
#endif
  bool success = true;
  switch (data->op) {
    case ROB_BRANCH:
    case ROB_JALR:
    if (data->dest >= 0) { // JAL or JALR
      reg_store_set(unit->reg_store, data->dest,
                    data->value);
    }
    if (_rob_is_mispredicted(*data)) {
      addr_t addr = _rob_addr(*data);
      debug_log("branch prediction failed, writing pc addr %08x",
                addr);
      inst_unit_force_pc(unit->inst_unit, addr);
      reg_file_clear(unit->reg_store);
      rob_unit_clear(unit);
      rs_unit_clear(unit->rs_unit);
      ls_queue_clear(unit->ls_queue);
      bus_clear(unit->cdb);
      mem_clear(unit->mem);
    } else {
      debug_log("branch prediction success!");
    }
    break;

    case ROB_REGISTER:
    debug_log("commit: reg[%02d] = %08x", data->dest,
              data->value);
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

    case ROB_HCF:
    hcf = true;
    success = false;
    break;

    default:
    debug_log("unknown rob op %d", data->op);
    assert(false);
  }
  if (success) {
    if (data->dest >= 0) {
      reg_store_rob_id_clear(unit->reg_store, data->dest,
                            rob->id);
    }
    rm_write(rob->ready, bool) = false;
    queue_pop(unit->robs);
  }
}
void _rob_unit_tick (void *state, va_list args) {
  rob_unit_t *unit = (rob_unit_t *) state;
  if (*rm_read(unit->robs->clear, bool)) return;
  if (queue_empty(unit->robs)) return;
  const reorder_buffer_t *rob =
    rm_read(queue_first(unit->robs), reorder_buffer_t);
  if (!*rm_read(rob->ready, bool)) return;
  _rob_commit(rob, unit);
}
void _rob_unit_onmsg (void *state, va_list args) {
  cdb_message_t *msg = va_arg(args, cdb_message_t *);
  rob_unit_t *unit = (rob_unit_t *) state;
  if (*rm_read(unit->robs->clear, bool)) return;
  reg_mut_t *reg;
  queue_foreach (unit->robs, i, reg) {
    const reorder_buffer_t *buf =
      rm_read(reg, reorder_buffer_t);
    if (msg->rob == buf->id) {
      rob_payload_t *data =
        &rm_write(buf->payload, rob_payload_t);
      if (data->op == ROB_JALR) {
        data->addr = msg->result;
      } else {
        data->value = msg->result;
      }
      rm_write(buf->ready, bool) = true;
    }
  }
}
rob_unit_t *rob_unit_create (reg_store_t *regs,
                             ls_queue_t *queue,
                             inst_unit_t *inst_unit,
                             rs_unit_t *rs_unit,
                             memory_t *mem,
                             bus_t *cdb,
                             clk_t *clk) {
  rob_unit_t *unit =
    (rob_unit_t *) malloc(sizeof(rob_unit_t));
  unit->reg_store = regs;
  unit->ls_queue = queue;
  unit->inst_unit = inst_unit;
  unit->rs_unit = rs_unit;
  unit->mem = mem;
  unit->cdb = cdb;
  unit->robs = queue_create(ROB_CAPACITY,
                            sizeof(reorder_buffer_t), clk);
  for (int i = 0; i < ROB_CAPACITY; ++i) {
    reorder_buffer_t *buf =
      &rm_write(queue_id(unit->robs, i), reorder_buffer_t);
    buf->id = i + 1;
    buf->ready = reg_mut_create(sizeof(bool), clk);
    buf->payload =
      reg_mut_create(sizeof(rob_payload_t), clk);
  }

  bus_listen(cdb, closure_create(_rob_unit_onmsg, unit));
  clk_add_callback(clk, closure_create(_rob_unit_tick, unit));

  return unit;
}
void rob_unit_free (rob_unit_t *unit) {
  for (int i = 0; i < ROB_CAPACITY; ++i) {
    const reorder_buffer_t *rob =
      rm_read(queue_id(unit->robs, i), reorder_buffer_t);
    reg_mut_free(rob->ready);
    reg_mut_free(rob->payload);
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
