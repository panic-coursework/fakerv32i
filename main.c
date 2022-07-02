#include <stdio.h>

#include "branch-predictor.h"
#include "clk.h"
#include "cpu.h"
#include "io.h"
#include "reorder-buffer.h"

#ifndef FAKE

int main () {
  cpu_t *cpu = cpu_create();
  read_to_memory(cpu->mem);
  while (!hcf) cpu_tick(cpu);
  write_result(cpu->reg_store);
  printf("commits = %ld\n", cpu->rob->commit_count);
  printf("cycles = %ld\n", clk_get(cpu->clk));
  branch_predictor_t *bp = cpu->branch_predictor;
  double rate = (double) bp->correct / bp->total;
  printf("bp total %ld, correct %ld, succ = %lf\n",
         bp->total, bp->correct, rate);
  double jalr_rate = (double) bp->jalr_correct / bp->jalr_total;
  printf("jalr total %ld, correct %ld, succ = %lf\n",
         bp->jalr_total, bp->jalr_correct, jalr_rate);
  cpu_free(cpu);
  return 0;
}

#else

#include <stdio.h>

#include "fakecpu.h"

int main () {
  fakecpu_t *cpu = fakecpu_create();
  read_to_memory(cpu->mem);
  while (!hcf) fakecpu_tick(cpu);
  printf("%d\n", cpu->regs[10] & 255u);
  fakecpu_free(cpu);
  return 0;
}

#endif
