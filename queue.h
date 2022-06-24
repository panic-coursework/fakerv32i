#pragma once

#include <stdbool.h>

#include "rv32i.h"

struct queue_t {
  int capacity;
  reg_mut_t **data;
  reg_mut_t *head; // int
  reg_mut_t *tail; // int, past-the-end
};

queue_t *queue_create (size_t capacity, size_t size,
                       clk_t *clk);
void queue_free (queue_t *queue);

reg_mut_t *queue_nth (queue_t *queue, int ix);
size_t queue_length (queue_t *queue);
bool queue_full (queue_t *queue);
reg_mut_t *queue_push (queue_t *queue);
reg_mut_t *queue_pop (queue_t *queue);
reg_mut_t *queue_first (queue_t *queue);
