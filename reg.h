#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "lib/closure.h"
#include "rv32i.h"

struct reg_t {
  size_t size;
  void *buf;
  closure_t *update;
};

reg_t *reg_create (size_t size, closure_t *update,
                   clk_t *clk);
void reg_free (reg_t *reg);

const void *reg_read (reg_t *reg);

#define r_read(reg, type) \
  ((const type *) reg_read(reg))

struct reg_mut_t {
  size_t size;
  void *buf;
  void *next;
  int write_count;
  bool clear;
};

reg_mut_t *reg_mut_create (size_t size, clk_t *clk);
void reg_mut_free (reg_mut_t *reg);

const void *reg_mut_read (reg_mut_t *reg);
void *reg_mut_write (reg_mut_t *reg);

#define rm_read(reg, type) \
  ((const type *) reg_mut_read(reg))
#define rm_write(reg, type) \
  (*(type *) reg_mut_write(reg))

struct reg_reduce_t {
  size_t size;
  void *init;
  void *buf;
  void *next;
  closure_t *reducer;
};

reg_reduce_t *reg_reduce_create (size_t size,
  const void *init, closure_t *reducer, clk_t *clk);
void reg_reduce_free (reg_reduce_t *reg);

const void *reg_reduce_read (reg_reduce_t *reg);
void reg_reduce_write (reg_reduce_t *reg,
                        const void *update);

#define rr_read(reg, type) \
  ((const type *) reg_reduce_read(reg))

reg_reduce_t *reg_or_create (clk_t *clk);

struct busy_wait_t {
  reg_reduce_t *stall;
  reg_mut_t *signal; // bool
};

busy_wait_t *busy_wait_create (clk_t *clk);
void busy_wait_free (busy_wait_t *reg);

void busy_wait (busy_wait_t *reg);
bool busy_wait_should_stall (busy_wait_t *reg);
reg_mut_t *busy_wait_callback (busy_wait_t *reg);
