#include "cpu.h"

int main () {
  cpu_t *cpu = cpu_create();
  cpu_tick(cpu);
  cpu_free(cpu);
  return 0;
}
