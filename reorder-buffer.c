#include <stdlib.h>

#include "reorder-buffer.h"
#include "rv32i.h"

void _rob_unit_tick (void *state, ...) {
  rob_unit_t *unit = (rob_unit_t *) state;
  // TODO
}
rob_unit_t *rob_unit_create (reg_store_t *regs,
                             ls_queue_t *queue,
                             clk_t *clk) {
  ;
}
void rob_unit_free (rob_unit_t *rob_unit);

reorder_buffer_t *rob_unit_find (rob_unit_t *unit,
                                 rob_id_t id);
bool rob_unit_full (rob_unit_t *unit);
reorder_buffer_t *rob_unit_acquire (rob_unit_t *unit);
