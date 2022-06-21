#pragma once

#include <stddef.h>

#include "rv32i.h"

struct reg_t {
  size_t size;
  void *buf;
  void *next;
  void (*update)(void *);
};

reg_t *reg_create (size_t size, void (*update)(void *));
void reg_destroy (reg_t *reg);

void reg_read (reg_t *reg, void *buf);
void reg_tick (reg_t *reg);
