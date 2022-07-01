#include <stdlib.h>

#include "branch-predictor.h"
#include "lib/log.h"
#include "rv32i.h"

branch_predictor_t *bp_create (clk_t *clk) {
  branch_predictor_t *bp = (branch_predictor_t *)
    malloc(sizeof(branch_predictor_t));
  bp->correct = 0;
  bp->total = 0;
  return bp;
}
void bp_free (branch_predictor_t *bp) {
  free(bp);
}

bool branch_predict (branch_predictor_t *bp, addr_t pc) {
  // TODO
  debug_log("prediction: branches!");
  return true;
}

void bp_feedback (branch_predictor_t *bp, addr_t pc,
                  bool actual_take, bool correct) {
  ++bp->total;
  if (correct) ++bp->correct;
  // TODO
}

// TODO: jalr api design
