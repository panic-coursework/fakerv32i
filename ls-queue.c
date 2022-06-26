#include <assert.h>
#include <stdlib.h>

#include "ls-queue.h"
#include "lib/log.h"
#include "memory.h"
#include "queue.h"
#include "reg.h"
#include "rv32i.h"

#define LS_QUEUE_CAPACITY 8

void _ls_queue_tick (void *state, ...) {
  ls_queue_t *queue = (ls_queue_t *) state;
  if (queue_empty(queue->queue)) return;
  if (busy_wait_should_stall(queue->lock)) return;
  const ls_queue_payload_t *data = 
    rm_read(queue_pop(queue->queue), ls_queue_payload_t);
  switch (data->op) {
    case LS_LOAD:
    mem_request_load(queue->mem, data->addr, data->size,
                     data->base_msg);
    break;

    case LS_STORE:
    mem_request_store(queue->mem, data->addr, data->value,
                      data->size,
                      busy_wait_callback(queue->lock));
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
  return STATUS_SUCCESS;
}