#include <stdlib.h>

#include "clk.h"
#include "inst-dispatch.h"
#include "inst-unit.h"
#include "inst.h"
#include "lib/closure.h"
#include "lib/log.h"
#include "memory.h"
#include "reg.h"
#include "reorder-buffer.h"
#include "reservation-station.h"
#include "rv32i.h"

struct _inst_unit_force_pc {
  bool busy;
  addr_t addr;
};

void _inst_unit_tick (void *state, va_list args) {
  inst_unit_t *unit = (inst_unit_t *) state;
  struct _inst_unit_force_pc rec =
    *rm_read(unit->force_pc, struct _inst_unit_force_pc);
  if (rec.busy) {
    rm_write(unit->force_pc, struct _inst_unit_force_pc)
      .busy = false;
    rm_write(unit->pc, addr_t) = rec.addr;
    debug_log("force PC: %08x", rec.addr);
    return;
  }
  if (rob_unit_full(unit->rob_unit)) {
    debug_log("instruction unit stall due to rob full!");
    return;
  }
  addr_t pc = *rm_read(unit->pc, addr_t);
  word_t inst_bin = mem_get_inst(unit->mem, pc);
  inst_t inst = inst_decode(inst_bin);
  debug_log("PC: %08x, inst: %08x, op: %08x", pc, inst_bin,
            inst.op);
  if (inst.op == INST_UNIMP) return;
  rs_type_t type = inst_type(inst);
  reservation_station_t *rs =
    rs_unit_acquire(unit->rs_unit, type);
  if (!rs) {
    debug_log("instruction unit stall due to rs full!");
    return;
  }
  reorder_buffer_t *rob = rob_unit_acquire(unit->rob_unit);
  addr_t next = inst_dispatch(unit, rs, rob, inst, pc);
  debug_log("next PC: %08x", next);
  rm_write(unit->pc, addr_t) = next;
}

inst_unit_t *inst_unit_create (memory_t *mem,
                               rs_unit_t *rs_unit,
                               reg_store_t *reg_store,
                               branch_predictor_t *bp,
                               bus_t *cdb,
                               clk_t *clk) {
  inst_unit_t *unit =
    (inst_unit_t *) malloc(sizeof(inst_unit_t));
  unit->pc = reg_mut_create(sizeof(addr_t), clk);
  unit->force_pc =
    reg_mut_create(sizeof(struct _inst_unit_force_pc), clk);
  unit->mem = mem;
  unit->rs_unit = rs_unit;
  unit->reg_store = reg_store;
  unit->branch_predictor = bp;
  unit->cdb = cdb;

  clk_add_callback(clk, closure_create(_inst_unit_tick, unit));

  return unit;
}
void inst_unit_free (inst_unit_t *unit) {
  reg_mut_free(unit->pc);
  reg_mut_free(unit->force_pc);
  free(unit);
}

void inst_unit_force_pc (inst_unit_t *unit, addr_t pc) {
  struct _inst_unit_force_pc *rec =
    &rm_write(unit->force_pc, struct _inst_unit_force_pc);
  rec->addr = pc;
  rec->busy = true;
}
