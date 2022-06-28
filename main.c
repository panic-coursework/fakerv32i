#include "cpu.h"
#include "io.h"

int main () {
  cpu_t *cpu = cpu_create();
  read_to_memory(cpu->mem);
  while (!hcf) cpu_tick(cpu);
  write_result(cpu->reg_store);
  cpu_free(cpu);
  return 0;
}
