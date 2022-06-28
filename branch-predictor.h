#pragma once

#include <stdbool.h>

#include "rv32i.h"

struct branch_predictor_t {
  // TODO
};

branch_predictor_t *bp_create (clk_t *clk);
void bp_free (branch_predictor_t *bp);

bool branch_predict (branch_predictor_t *bp);
