#include <stdlib.h>

#include "cpu.h"
#include "alu.h"
#include "branch-predictor.h"
#include "bus.h"
#include "common-data-bus.h"
#include "clk.h"
#include "dump.h"
#include "inst-unit.h"
#include "lib/closure.h"
#include "lib/log.h"
#include "ls-queue.h"
#include "memory.h"
#include "reg-store.h"
#include "reorder-buffer.h"
#include "reservation-station.h"
#include "rv32i.h"

bool hcf = false;

void _cpu_debug_cdb (void *state, va_list args) {
  cpu_t *cpu = (cpu_t *) state;
  cdb_message_t *msg = va_arg(args, cdb_message_t *);
  debug_log("cdb tick %ld: ROB #%02d received value %08x",
            clk_get(cpu->clk), msg->rob, msg->result);
}

void _cpu_debug (void *state, va_list args) {
  cpu_t *cpu = (cpu_t *) state;
  cpu_dump(cpu);
}

cpu_t *cpu_create () {
  cpu_t *cpu = (cpu_t *) malloc(sizeof(cpu_t));
  clk_t *clk = cpu->clk = clk_create();
  bus_t *cdb = cpu->cdb =
    bus_create(sizeof(cdb_message_t), clk);
  memory_t *mem = cpu->mem = mem_create(cdb, clk);
  cpu->reg_store = reg_store_create(clk);
  cpu->ls_queue = ls_queue_create(mem, clk);
  cpu->branch_predictor = bp_create(clk);
  cpu->alu = alu_pool_create(cdb);
  cpu->rs = rs_unit_create(cpu->alu, cpu->ls_queue, clk);
  cpu->inst_unit =
    inst_unit_create(mem, cpu->rs, cpu->reg_store,
                     cpu->branch_predictor, cdb, clk);
  cpu->rob = rob_unit_create(cpu->reg_store, cpu->ls_queue,
                             cpu->inst_unit, cpu->rs, mem,
                             cdb, clk);
  cpu->rs->rob_unit = cpu->inst_unit->rob_unit = cpu->rob;

  // add reservation stations
  int i = 1;
  for (; i < 8; ++i) {
    rs_unit_add(cpu->rs,
      rs_create(i, RS_RESERVATION_STATION, cdb, clk));
  }
  for (; i < 16; ++i) {
    rs_unit_add(cpu->rs,
      rs_create(i, RS_LOAD_BUFFER, cdb, clk));
  }
  for (; i < 24; ++i) {
    rs_unit_add(cpu->rs,
      rs_create(i, RS_STORE_BUFFER, cdb, clk));
  }

  // debug
  bus_listen(cdb, closure_create(_cpu_debug_cdb, cpu));
  clk_add_callback(clk, closure_create(_cpu_debug, cpu));

  return cpu;
}
void cpu_free (cpu_t *cpu) {
  rob_unit_free(cpu->rob);
  inst_unit_free(cpu->inst_unit);
  rs_unit_free(cpu->rs);
  alu_pool_free(cpu->alu);
  bp_free(cpu->branch_predictor);
  ls_queue_free(cpu->ls_queue);
  reg_store_free(cpu->reg_store);
  mem_free(cpu->mem);
  bus_free(cpu->cdb);
  clk_free(cpu->clk);
  free(cpu);
}

void cpu_tick (cpu_t *cpu) {
  clk_tick(cpu->clk);
}
