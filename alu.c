#include <stdbool.h>
#include <stdlib.h>

#include "alu-ops.h"
#include "alu.h"
#include "bus.h"
#include "clk.h"
#include "common-data-bus.h"
#include "lib/closure.h"
#include "lib/log.h"
#include "reg.h"
#include "rv32i.h"

typedef struct _alu_result_buf_t {
  cdb_message_t msg;
  bool busy;
} _alu_result_buf_t;
// TODO: test
void _alu_tick (void *state, va_list args) {
  alu_t *alu = (alu_t *) state;
  if (bh_should_clear(alu->cdb_helper)) {
    rm_write(alu->result_buf, _alu_result_buf_t).busy = false;
    return;
  }
  bool busy = *rr_read(alu->busy, bool);
  _alu_result_buf_t buf =
    *rm_read(alu->result_buf, _alu_result_buf_t);
  if (buf.busy) {
    debug_log("ALU stall due to cdb full!");
    reg_mut_t *reg = bh_acquire(alu->cdb_helper);
    if (!reg) return;
    rm_write(reg, cdb_message_t) = buf.msg;
    buf.busy = false;
    rm_write(alu->result_buf, _alu_result_buf_t) = buf;
    reg_reduce_write(alu->busy, &busy);
    return;
  }
  if (busy) {
    alu_task_t task = *rm_read(alu->task, alu_task_t);
    reg_mut_t *reg = bh_acquire(alu->cdb_helper);
    cdb_message_t msg = task.base_msg;
    msg.result =
      alu_execute(task.op, task.value1, task.value2);
    debug_log("ALU task complete, writing ROB #%02d = %08x",
              msg.rob, msg.result);
    if (!reg) {
      debug_log("^ can't write cdb!");
      _alu_result_buf_t buf;
      buf.msg = msg;
      buf.busy = true;
      rm_write(alu->result_buf, _alu_result_buf_t) = buf;
      return;
    }
    rm_write(reg, cdb_message_t) = msg;
  }
}
alu_t *alu_create (bus_t *cdb) {
  alu_t *alu = (alu_t *) malloc(sizeof(alu_t));
  alu->cdb = cdb;
  alu->cdb_helper = bh_create(cdb);
  alu->task = reg_mut_create(sizeof(alu_task_t), cdb->clk);
  alu->result_buf =
    reg_mut_create(sizeof(_alu_result_buf_t), cdb->clk);
  alu->busy = reg_or_create(cdb->clk);

  clk_add_callback(cdb->clk, closure_create(_alu_tick, alu));

  return alu;
}
void alu_free (alu_t *alu) {
  bh_free(alu->cdb_helper);
  reg_mut_free(alu->task);
  reg_mut_free(alu->result_buf);
  reg_reduce_free(alu->busy);
  free(alu);
}

status_t alu_task (alu_t *alu, alu_task_t task) {
  if (bh_should_clear(alu->cdb_helper)) {
    debug_log("ALU rejecting task due to stall!");
    return STATUS_SUCCESS;
  }
  if (bh_should_stall(alu->cdb_helper)) {
    debug_log("ALU rejecting task due to cdb full!");
    return STATUS_FAIL;
  }
  debug_log("ALU recv task for ROB #%02d", task.base_msg.rob);
  rm_write(alu->task, alu_task_t) = task;
  bool t = true;
  reg_reduce_write(alu->busy, &t);
  return STATUS_SUCCESS;
}

alu_pool_t *alu_pool_create (bus_t *cdb) {
  alu_pool_t *alu_pool =
    (alu_pool_t *) malloc(sizeof(alu_pool_t));
  for (int i = 0; i < ALU_COUNT; ++i) {
    alu_pool->alus[i] = alu_create(cdb);
  }
  return alu_pool;
}
void alu_pool_free (alu_pool_t *alu_pool) {
  for (int i = 0; i < ALU_COUNT; ++i) {
    alu_free(alu_pool->alus[i]);
  }
  free(alu_pool);
}
