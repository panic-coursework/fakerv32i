#pragma once

#include <stddef.h>

#include "lib/closure.h"
#include "rv32i.h"

struct reg_t {
  size_t size;
  void *buf;
  void *next;
  closure_t *update;
};

reg_t *reg_create(size_t size, closure_t *update, clk_t *clk);
void reg_free(reg_t *reg);

const void *reg_read(reg_t *reg);

#define r_read(reg, type) \
  (const type *) reg_read(reg)

struct reg_mut_t {
  size_t size;
  void *buf;
  void *next;
  int write_count;
};

reg_mut_t *reg_mut_create(size_t size, clk_t *clk);
void reg_mut_free(reg_mut_t *reg);

const void *reg_mut_read(reg_mut_t *reg);
void *reg_mut_write(reg_mut_t *reg);

#define rm_read(reg, type) \
  (const type *) reg_mut_read(reg)
#define rm_write(reg, type) \
  *(type *) reg_mut_write(reg)
