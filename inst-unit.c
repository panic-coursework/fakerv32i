#include <stdlib.h>

#include "clk.h"
#include "inst-unit.h"
#include "inst.h"
#include "lib/closure.h"
#include "memory.h"
#include "reg.h"
#include "reorder-buffer.h"
#include "reservation-station.h"
#include "rv32i.h"

addr_t _inst_dispatch (inst_unit_t *unit,
                       reservation_station_t *rs,
                       reorder_buffer_t *rob,
                       inst_t inst);

void _inst_unit_tick (void *state, ...) {
  inst_unit_t *unit = (inst_unit_t *) state;
  if (rob_unit_full(unit->rob_unit)) return;
  addr_t pc = *rm_read(unit->pc, addr_t);
  word_t inst_bin = mem_get_inst(unit->mem, pc);
  inst_t inst = inst_decode(inst_bin);
  rs_type_t type = inst_type(inst);
  reservation_station_t *rs =
    rs_unit_acquire(unit->rs_unit, type);
  if (!rs) return;
  reorder_buffer_t *rob = rob_unit_acquire(unit->rob_unit);
  addr_t next = _inst_dispatch(unit, rs, rob, inst);
  rm_write(unit->pc, addr_t) = next;
}

inst_unit_t *inst_unit_create (memory_t *mem,
                               rs_unit_t *rs_unit,
                               rob_unit_t *rob_unit,
                               reg_file_t *reg_file,
                               reg_store_t *reg_store,
                               bus_t *cdb,
                               clk_t *clk) {
  inst_unit_t *unit =
    (inst_unit_t *) malloc(sizeof(inst_unit_t));
  unit->pc = reg_mut_create(sizeof(addr_t), clk);
  unit->mem = mem;
  unit->rs_unit = rs_unit;
  unit->rob_unit = rob_unit;
  unit->reg_file = reg_file;
  unit->reg_store = reg_store;
  unit->cdb = cdb;

  clk_add_callback(clk, closure_create(_inst_unit_tick, unit));

  return unit;
}
void inst_unit_free (inst_unit_t *unit) {
  reg_mut_free(unit->pc);
  free(unit);
}

addr_t _inst_dispatch (inst_unit_t *unit,
                       reservation_station_t *rs,
                       reorder_buffer_t *rob,
                       inst_t inst) {
  // TODO
  // TODO: see if cdb has some data we need
}
