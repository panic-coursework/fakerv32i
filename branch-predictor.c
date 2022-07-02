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
  bp->state = reg_mut_create(sizeof(int), clk);
  return bp;
}
void bp_free (branch_predictor_t *bp) {
  reg_mut_free(bp->state);
  free(bp);
}

bool branch_predict (branch_predictor_t *bp, addr_t pc) {
  return *rm_read(bp->state, int) >= 2;
}

void bp_feedback (branch_predictor_t *bp, addr_t pc,
                  bool actual_take, bool correct) {
  ++bp->total;
  if (correct) ++bp->correct;
  int state = *rm_read(bp->state, int);
  if (actual_take) {
    if (state < 3) ++state;
  } else {
    if (state > 0) --state;
  }
  rm_write(bp->state, int) = state;
}

// TODO: jalr api design
