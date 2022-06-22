#include "reg-store.h"
#include "reg.h"
#include "rv32i.h"

#include <stdlib.h>

reg_store_t *reg_store_create (clk_t *clk) {
  reg_store_t *reg = (reg_store_t *) malloc(sizeof(reg_store_t));
  reg->pc = reg_mut_create(sizeof(addr_t), clk);
  for (int i = 0; i < REG_COUNT; ++i) {
    reg->registers[i] = reg_mut_create(sizeof(word_t), clk);
  }
  return reg;
}
void reg_store_free (reg_store_t *reg) {
  reg_mut_free(reg->pc);
  for (int i = 0; i < REG_COUNT; ++i) {
    reg_mut_free(reg->registers[i]);
  }
  free(reg);
}

addr_t reg_store_pc_get (reg_store_t *reg) {
  return *rm_read(reg->pc, addr_t);
}
void reg_store_pc_set (reg_store_t *reg, addr_t value) {
  rm_write(reg->pc, addr_t) = value;
}
word_t reg_store_get (reg_store_t *reg, int id) {
  if (id == 0) return 0;
  return *rm_read(reg->registers[id], word_t);
}
void reg_store_set (reg_store_t *reg, int id, word_t value) {
  if (id == 0) return;
  rm_write(reg->registers[id], word_t) = value;
}
