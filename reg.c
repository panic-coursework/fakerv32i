#include "reg.h"

#include <stdlib.h>

#include "clk.h"
#include "lib/closure.h"
#include "lib/log.h"
#include "rv32i.h"

void _reg_tick (void *_reg, ...) {
  reg_t *reg = (reg_t *) _reg;
  void *tmp = reg->buf;
  reg->buf = reg->next;
  reg->next = tmp;
  closure_call(reg->update, reg->next);
}

reg_t *reg_create (size_t size, closure_t *update,
                   clk_t *clk) {
  reg_t *reg = (reg_t *) malloc(sizeof(reg_t));
  reg->size = size;
  reg->buf = malloc(size);
  reg->next = malloc(size);
  reg->update = update;

  clk_add_callback(clk, closure_create(_reg_tick, reg));

  return reg;
}

void reg_free (reg_t *reg) {
  free(reg->buf);
  free(reg->next);
  free(reg);
}

const void *reg_read (reg_t *reg) {
  return reg->buf;
}
void *reg_write (reg_t *reg) {
  return reg->buf;
}

void _reg_mut_tick (void *_reg, ...) {
  reg_mut_t *reg = (reg_mut_t *) _reg;
  void *tmp = reg->buf;
  reg->buf = reg->next;
  reg->next = tmp;
  reg->write_count = 0;
}

reg_mut_t *reg_mut_create (size_t size, clk_t *clk) {
  reg_mut_t *reg = (reg_mut_t *) malloc(sizeof(reg_t));
  reg->size = size;
  reg->buf = malloc(size);
  reg->next = malloc(size);
  reg->write_count = 0;

  clk_add_callbefore(clk, closure_create(_reg_mut_tick, reg));

  return reg;
}
void reg_mut_free (reg_mut_t *reg) {
  free(reg->buf);
  free(reg->next);
  free(reg);
}

const void *reg_mut_read (reg_mut_t *reg) {
  return reg->buf;
}
void *reg_mut_write (reg_mut_t *reg) {
  if (++reg->write_count > 1) {
    debug_log("reg_write called more than once!");
  }
  return reg->next;
}
