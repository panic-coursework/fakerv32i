#pragma once

#include "rv32i.h"

#define INST_QUEUE_SIZE 16

struct inst_queue_t {
  queue_t *queue;
  rs_unit_t *rs_unit;
  rob_unit_t *rob_unit;
};

// TODO: _inst_queue_tick
inst_queue_t *inst_queue_create (rs_unit_t *rs_unit,
                                 rob_unit_t *rob_unit,
                                 clk_t *clk);
void inst_queue_free (inst_queue_t *queue);
