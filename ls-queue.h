#pragma once

#include <stdbool.h>

#include "common-data-bus.h"
#include "rv32i.h"

struct ls_queue_payload_t {
  ls_op_t op;
  addr_t addr;
  ls_size_t size;

  // load only
  cdb_message_t base_msg;
  // store only
  word_t value;
};

struct ls_queue_t {
  queue_t *queue;
  busy_wait_t *lock;
  memory_t *mem;
};

ls_queue_t *ls_queue_create (memory_t *mem, clk_t *clk);
void ls_queue_free (ls_queue_t *queue);

bool ls_queue_full (ls_queue_t *queue);
status_t ls_queue_push (ls_queue_t *queue,
                        ls_queue_payload_t payload);
