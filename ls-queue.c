#include <assert.h>
#include <stdlib.h>

#include "clk.h"
#include "lib/closure.h"
#include "ls-queue.h"
#include "lib/log.h"
#include "memory.h"
#include "queue.h"
#include "reg.h"
#include "rv32i.h"

#define LS_QUEUE_CAPACITY 8

void _ls_queue_tick (void *state, va_list args) {
  ls_queue_t *queue = (ls_queue_t *) state;
  if (busy_wait_should_stall(queue->lock)) {
    debug_log("ls queue busy waiting");
    return;
  }
  if (queue_empty(queue->queue)) {
    debug_log("ls queue empty, skipping");
    return;
  }
  const ls_queue_payload_t *data = 
    rm_read(queue_pop(queue->queue), ls_queue_payload_t);
  switch (data->op) {
    case LS_LOAD:
    debug_log("ls queue load addr %08x for ROB #%02d",
              data->addr, data->base_msg.rob);
    mem_request_load(queue->mem, data->addr, data->size,
                     data->base_msg);
    break;

    case LS_STORE:
    debug_log("ls queue store addr %08x value %08x",
              data->addr, data->value);
    mem_request_store(queue->mem, data->addr, data->value,
                      data->size,
                      busy_wait_callback(queue->lock));
    busy_wait(queue->lock);
    break;

    default:
    debug_log("unknown ls queue op %d", data->op);
    assert(false);
  }
}
ls_queue_t *ls_queue_create (memory_t *mem, clk_t *clk) {
  ls_queue_t *queue =
    (ls_queue_t *) malloc(sizeof(ls_queue_t));
  queue->queue = queue_create(LS_QUEUE_CAPACITY,
                              sizeof(ls_queue_payload_t),
                              clk);
  queue->lock = busy_wait_create(clk);
  queue->mem = mem;

  clk_add_callback(clk, closure_create(_ls_queue_tick, queue));

  return queue;
}
void ls_queue_free (ls_queue_t *queue) {
  queue_free(queue->queue);
  busy_wait_free(queue->lock);
  free(queue);
}

bool ls_queue_full (ls_queue_t *queue) {
  return queue_full(queue->queue);
}
status_t ls_queue_push (ls_queue_t *queue,
                        ls_queue_payload_t payload) {
  if (ls_queue_full(queue)) return STATUS_FAIL;
  reg_mut_t *reg = queue_push(queue->queue);
  rm_write(reg, ls_queue_payload_t) = payload;
  debug_log("recv req %s addr %08x size %d",
            payload.op == LS_LOAD ? "load" : "store",
            payload.addr, payload.size);
  return STATUS_SUCCESS;
}
