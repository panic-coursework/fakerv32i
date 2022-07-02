#pragma once

#include <bits/types.h>
#include <stdbool.h>

#include "rv32i.h"

#define BP_HIST_LENGTH 8
#define BP_HIST_MASK 0b11111111
#define BP_BITS (1 << BP_HIST_LENGTH)
#define BP_SIZE 0x100
#define BP_MASK 0x0ff
#define BP_MAX 3
#define BP_THRESHOLD 2

struct branch_predictor_t {
  __uint8_t state[BP_BITS][BP_SIZE];
  int hist;
  long total;
  long correct;
  long jalr_total;
  long jalr_correct;
};

struct bp_result_t {
  bool take;
  int hist;
};

branch_predictor_t *bp_create (clk_t *clk);
void bp_free (branch_predictor_t *bp);

bp_result_t branch_predict (branch_predictor_t *bp, addr_t pc);
void bp_feedback (branch_predictor_t *bp, addr_t pc,
                  bp_result_t res, bool actual_take);
void bp_feedback_jalr (branch_predictor_t *bp, addr_t pc,
                       addr_t actual, bool correct);
