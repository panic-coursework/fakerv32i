#include "cpu.h"
#include "io.h"

#define FAKE
#ifndef FAKE

int main () {
  cpu_t *cpu = cpu_create();
  read_to_memory(cpu->mem);
  while (!hcf) cpu_tick(cpu);
  write_result(cpu->reg_store);
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
