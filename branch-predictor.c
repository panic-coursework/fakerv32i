#include <stdlib.h>

#include "branch-predictor.h"
#include "lib/log.h"
#include "reg.h"
#include "rv32i.h"

branch_predictor_t *bp_create (clk_t *clk) {
  branch_predictor_t *bp = (branch_predictor_t *)
    malloc(sizeof(branch_predictor_t));
  bp->correct = 0;
  bp->total = 0;
  bp->jalr_total = 0;
  bp->jalr_correct = 0;
  for (int i = 0; i < BP_BITS; ++i) {
    for (int j = 0; j < BP_SIZE; ++j) {
      bp->state[i][j] = BP_THRESHOLD;
    }
  }
  return bp;
}
void bp_free (branch_predictor_t *bp) {
  free(bp);
}

int _bp_hash (addr_t pc) {
  return (pc >> 2) & BP_MASK;
}
int _bp_hist (branch_predictor_t *bp) {
  return bp->hist;
}

bp_result_t branch_predict (branch_predictor_t *bp, addr_t pc) {
  int hist = _bp_hist(bp);
  return (bp_result_t) {
    .take = bp->state[hist][_bp_hash(pc)] >= BP_THRESHOLD,
    .hist = hist,
  };
}

void bp_feedback (branch_predictor_t *bp, addr_t pc,
                  bp_result_t res, bool actual_take) {
  ++bp->total;
  if (actual_take == res.take) ++bp->correct;

  int hash = _bp_hash(pc);
  int state = bp->state[res.hist][hash];
  if (actual_take) {
    if (state < BP_MAX) ++state;
  } else {
    if (state > 0) --state;
  }
  bp->state[res.hist][hash] = state;

  bp->hist = ((bp->hist << 1) | actual_take) & BP_HIST_MASK;
}

void bp_feedback_jalr (branch_predictor_t *bp, addr_t pc,
                       addr_t actual, bool correct) {
  ++bp->jalr_total;
  if (correct) ++bp->jalr_correct;
}
