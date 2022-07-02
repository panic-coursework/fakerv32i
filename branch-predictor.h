#pragma once

#include <stdbool.h>

#include "rv32i.h"

struct branch_predictor_t {
  // TODO
  reg_mut_t *state; // int
  long total;
  long correct;
};

branch_predictor_t *bp_create (clk_t *clk);
void bp_free (branch_predictor_t *bp);

bool branch_predict (branch_predictor_t *bp, addr_t pc);
void bp_feedback (branch_predictor_t *bp, addr_t pc,
                  bool actual_take, bool correct);
