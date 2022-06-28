#include <stdlib.h>

#include "branch-predictor.h"
#include "lib/log.h"
#include "rv32i.h"

branch_predictor_t *bp_create (clk_t *clk) {
  branch_predictor_t *bp = (branch_predictor_t *)
    malloc(sizeof(branch_predictor_t));
  return bp;
}
void bp_free (branch_predictor_t *bp) {
  free(bp);
}

bool branch_predict (branch_predictor_t *bp) {
  // TODO
  debug_log("prediction: branches!");
  return true;
}

// TODO: jalr api design
