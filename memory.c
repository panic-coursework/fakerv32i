#include "memory.h"
#include "rv32i.h"

#include <stdlib.h>
#include <string.h>

memory_t *mem_create (bus_t *cdb, clk_t *clk) {
  memory_t *mem = (memory_t *) malloc(sizeof(memory_t));
  memset(mem->data, 0, sizeof(mem->data));
  memset(mem->load_requests, 0, sizeof(mem->load_requests));
  memset(mem->store_requests, 0, sizeof(mem->store_requests));
  mem->cdb = cdb;
  // TODO: clk
  return mem;
}
void mem_free (memory_t *mem) {
  free(mem);
}

void mem_set (memory_t *mem, addr_t addr, byte_t value) {
  mem->data[addr] = value;
}
byte_t mem_get (memory_t *mem, addr_t addr) {
  return mem->data[addr];
}
