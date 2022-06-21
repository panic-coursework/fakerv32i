#include <stdlib.h>

#include "clk.h"
#include "rv32i.h"

struct reg_t {
  size_t size;
  void *buf;
  void *next;
  void (*update)(void *);
};

reg_t *reg_create (size_t size, void (*update)(void *),
                   clk_t *clk) {
  reg_t *reg = (reg_t *) malloc(sizeof(reg_t));
  reg->size = size;
  reg->buf = malloc(size);
  reg->next = malloc(size);
  reg->update = update;

  clk_add_reg(clk, reg);

  return reg;
}

void reg_destroy (reg_t *reg) {
  free(reg->buf);
  free(reg->next);
  free(reg);
}

void *reg_read (reg_t *reg) {
  return reg->buf;
}

void reg_tick (reg_t *reg) {
  void *tmp = reg->buf;
  reg->buf = reg->next;
  reg->next = tmp;
  reg->update(reg->next);
}
