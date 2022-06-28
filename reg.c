#include "reg.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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
  reg->buf = calloc(1, size);
  reg->next = calloc(1, size);
  reg->update = update;

  clk_add_callback(clk, closure_create(_reg_tick, reg));

  return reg;
}

void reg_free (reg_t *reg) {
  closure_free(reg->update);
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
  memcpy(reg->buf, reg->next, reg->size);
  reg->write_count = 0;
}

reg_mut_t *reg_mut_create (size_t size, clk_t *clk) {
  reg_mut_t *reg = (reg_mut_t *) malloc(sizeof(reg_mut_t));
  reg->size = size;
  reg->buf = calloc(1, size);
  reg->next = calloc(1, size);
  reg->write_count = 0;
  reg->allow_multiwrite = false;

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
  if (++reg->write_count > 1 && !reg->allow_multiwrite) {
    debug_log("reg_mut_write called more than once!");
  }
  return reg->next;
}

void _reg_reduce_tick (void *_reg, ...) {
  reg_reduce_t *reg = (reg_reduce_t *) _reg;
  void *tmp = reg->buf;
  reg->buf = reg->next;
  reg->next = tmp;
  memcpy(reg->next, reg->init, reg->size);
}
reg_reduce_t *reg_reduce_create (size_t size,
  const void *init, closure_t *reducer, clk_t *clk) {
  reg_reduce_t *reg =
    (reg_reduce_t *) malloc(sizeof(reg_reduce_t));
  reg->size = size;
  reg->init = malloc(size);
  memcpy(reg->init, init, size);
  reg->buf = calloc(1, size);
  reg->next = calloc(1, size);
  reg->reducer = reducer;

  clk_add_callbefore(clk, closure_create(_reg_reduce_tick, reg));

  return reg;
}
void reg_reduce_free (reg_reduce_t *reg) {
  free(reg->init);
  free(reg->buf);
  free(reg->next);
  closure_free(reg->reducer);
  free(reg);
}

const void *reg_reduce_read (reg_reduce_t *reg) {
  return reg->buf;
}
void reg_reduce_write (reg_reduce_t *reg,
                       const void *update) {
  closure_call(reg->reducer, reg->next, update);
}

void _reg_or_reduce (void *state, ...) {
  reg_reduce_t *reg = (reg_reduce_t *) state;
  va_list args;
  va_start(args, state);
  bool *prev = va_arg(args, bool *);
  bool *curr = va_arg(args, bool *);
  va_end(args);

  *prev = *prev || *curr;
}
reg_reduce_t *reg_or_create (clk_t *clk) {
  bool f = false;
  reg_reduce_t *reg =  reg_reduce_create(sizeof(bool), &f,
    closure_create(_reg_or_reduce, NULL), clk);
  reg->reducer->data = reg;
  return reg;
}

void _busy_wait_tick (void *state, ...) {
  busy_wait_t *reg = (busy_wait_t *) state;
  if (!rr_read(reg->stall, bool)) return;
  if (!rm_read(reg->signal, bool)) busy_wait(reg);
}
busy_wait_t *busy_wait_create (clk_t *clk) {
  busy_wait_t *reg =
    (busy_wait_t *) calloc(1, sizeof(busy_wait_t));
  reg->stall = reg_or_create(clk);
  reg->signal = reg_mut_create(sizeof(bool), clk);
  clk_add_callback(clk, closure_create(_busy_wait_tick, reg));
  return reg;
}
void busy_wait_free (busy_wait_t *reg) {
  reg_reduce_free(reg->stall);
  reg_mut_free(reg->signal);
  free(reg);
}

void busy_wait (busy_wait_t *reg) {
  bool t = true;
  reg_reduce_write(reg->stall, &t);
}
bool busy_wait_should_stall (busy_wait_t *reg) {
  return rr_read(reg->stall, bool);
}
reg_mut_t *busy_wait_callback (busy_wait_t *reg) {
  return reg->signal;
}
