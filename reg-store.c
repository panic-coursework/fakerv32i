#include "reg-store.h"
#include "clk.h"
#include "lib/closure.h"
#include "reg.h"
#include "rv32i.h"

#include <stdlib.h>

void _reg_file_tick (void *state, va_list args) {
  reg_store_t *reg = (reg_store_t *) state;
  if (*rm_read(reg->clear, bool)) {
    for (int i = 0; i < REG_COUNT; ++i) {
      rm_write(reg->rob_id[i], rob_id_t) = 0;
    }
  }
}

reg_store_t *reg_store_create (clk_t *clk) {
  reg_store_t *reg = (reg_store_t *) malloc(sizeof(reg_store_t));
  for (int i = 0; i < REG_COUNT; ++i) {
    reg->registers[i] = reg_mut_create(sizeof(word_t), clk);
    reg->rob_id[i] = reg_mut_create(sizeof(rob_id_t), clk);
  }
  reg->clear = reg_mut_create(sizeof(bool), clk);
  reg->clear->clear = true;
  clk_add_callback(clk, closure_create(_reg_file_tick, reg));
  return reg;
}
void reg_store_free (reg_store_t *reg) {
  for (int i = 0; i < REG_COUNT; ++i) {
    reg_mut_free(reg->registers[i]);
    reg_mut_free(reg->rob_id[i]);
  }
  reg_mut_free(reg->clear);
  free(reg);
}

word_t reg_store_get (reg_store_t *reg, reg_id_t id) {
  if (id == 0) return 0;
  return *rm_read(reg->registers[id], word_t);
}
void reg_store_set (reg_store_t *reg, reg_id_t id,
                    word_t value) {
  if (id == 0) return;
  rm_write(reg->registers[id], word_t) = value;
}

reg_mut_t *reg_store_rob_id (reg_store_t *reg, reg_id_t id) {
  return reg->rob_id[id];
}
void reg_file_clear (reg_store_t *reg) {
  rm_write(reg->clear, bool) = true;
}
