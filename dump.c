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
      reg_store_rob_id_get(cpu->reg_store, i);
    word_t value = reg_store_get(cpu->reg_store, i);
    if (rob_id == 0 && value == 0) continue;
    fprintf(stderr, "Reg %2d: ROB #%02d, Value %08x\n", i,
            rob_id, value);
  }
  fprintf(stderr, SEP);
}

void _dump_rs_vec (vector_t *vec, const char *type) {
  reservation_station_t *rs;
  vector_foreach (vec, i, rs) {
    if (*rm_read(rs->busy, bool)) {
      fprintf(stderr, "%s %d: ", type, rs->id);
      rs_payload_t data =
        *rm_read(rs->payload, rs_payload_t);
      fprintf(stderr, "dest ROB #%02d ", data.dest);
      if (data.src1 == 0) {
        fprintf(stderr, "value1 %08x ", data.value1);
      } else {
        fprintf(stderr, "src1 #%02d ", data.src1);
      }
      if (data.src2 == 0) {
        fprintf(stderr, "value2 %08x\n", data.value2);
      } else {
        fprintf(stderr, "src2 #%02d\n", data.src2);
      }
    }
  }
}
void _dump_rs (cpu_t *cpu) {
  _dump_rs_vec(cpu->rs->alu_stations, "RS");
  _dump_rs_vec(cpu->rs->load_buffers, "RSLB");
  _dump_rs_vec(cpu->rs->store_buffers, "RSSB");
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
      fprintf(stderr, " ROB #%02d\n", data->base_msg.rob);
    } else {
      fprintf(stderr, " value %08x\n", data->value);
    }
  }
  fprintf(stderr, SEP);
}
