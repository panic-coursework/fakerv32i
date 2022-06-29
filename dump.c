#include <stdio.h>

#include "dump.h"
#include "clk.h"
#include "lib/vector.h"
#include "ls-queue.h"
#include "queue.h"
#include "reg.h"
#include "reg-store.h"
#include "reorder-buffer.h"
#include "reservation-station.h"
#include "rv32i.h"

#define SEP \
"+----------------------------------------------------+\n"

#define CLOCK \
SEP                                                        \
"|                     Cycle %6ld                   |\n"    \
SEP

void _dump_clk (cpu_t *cpu);
void _dump_reg (cpu_t *cpu);
void _dump_rs (cpu_t *cpu);
void _dump_rob (cpu_t *cpu);
void _dump_ls_queue (cpu_t *cpu);

void cpu_dump (cpu_t *cpu) {
  _dump_clk(cpu);
  _dump_reg(cpu);
  _dump_rs(cpu);
  _dump_rob(cpu);
  _dump_ls_queue(cpu);
}

void _dump_clk (cpu_t *cpu) {
  fprintf(stderr, CLOCK, clk_get(cpu->clk));
}

void _dump_reg (cpu_t *cpu) {
  for (int i = 0; i < REG_COUNT; ++i) {
    rob_id_t rob_id =
      *rm_read(reg_store_rob_id(cpu->reg_store, i), rob_id_t);
    word_t value = reg_store_get(cpu->reg_store, i);
    if (rob_id == 0 && value == 0) continue;
    fprintf(stderr, "Reg %2d: ROB #%02d, Value %08x\n", i,
            rob_id, value);
  }
  fprintf(stderr, SEP);
}

void _dump_rs (cpu_t *cpu) {
  reservation_station_t *rs;
  vector_foreach (cpu->rs->alu_stations, i, rs) {
    if (*rm_read(rs->busy, bool)) {
      fprintf(stderr, "RS %d busy\n", rs->id);
    }
  }
  vector_foreach (cpu->rs->load_buffers, i, rs) {
    if (*rm_read(rs->busy, bool)) {
      fprintf(stderr, "RSLB %d busy\n", rs->id);
    }
  }
  vector_foreach (cpu->rs->store_buffers, i, rs) {
    if (*rm_read(rs->busy, bool)) {
      fprintf(stderr, "RSSB %d busy\n", rs->id);
    }
  }
  fprintf(stderr, SEP);
}

void _dump_rob (cpu_t *cpu) {
  fprintf(stderr, "rob length %ld\n",
          queue_length(cpu->rob->robs));
  reg_mut_t *reg;
  queue_foreach (cpu->rob->robs, i, reg) {
    const reorder_buffer_t *rob =
      rm_read(reg, reorder_buffer_t);
    addr_t pc = rm_read(rob->payload, rob_payload_t)->pc;
    fprintf(stderr, "ROB %d pc %08x: ready? %d\n",
            rob->id, pc, (int)*rm_read(rob->ready, bool));
  }
  fprintf(stderr, SEP);
}

void _dump_ls_queue (cpu_t *cpu) {
  fprintf(stderr, "ls queue length %ld\n",
          queue_length(cpu->ls_queue->queue));
  reg_mut_t *reg;
  queue_foreach (cpu->ls_queue->queue, i, reg) {
    const ls_queue_payload_t *data =
      rm_read(reg, ls_queue_payload_t);
    fprintf(stderr, "%s addr %08x size %d",
            data->op == LS_LOAD ? "load" : "store",
            data->addr, data->size);
    if (data->op == LS_LOAD) {
      fprintf(stderr, " ROB #%02x\n", data->base_msg.rob);
    } else {
      fprintf(stderr, " value %08x\n", data->value);
    }
  }
  fprintf(stderr, SEP);
}
