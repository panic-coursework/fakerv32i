#pragma once

#include "common-data-bus.h"
#include "rv32i.h"

#include <stdbool.h>

#define MEM_SIZE 131072
#define MEM_TICKS 3

struct mem_load_request_t {
  addr_t addr;
  ls_size_t size;
  cdb_message_t base_msg;
  int ticks_remaining;
};
struct mem_store_request_t {
  addr_t addr;
  word_t value;
  reg_mut_t *callback;
  ls_size_t size;
  int ticks_remaining;
};

struct memory_t {
  byte_t data[MEM_SIZE];
  bus_t *cdb;
  bus_helper_t *cdb_helper;
  reg_mut_t *load_requests[MEM_TICKS];
  reg_reduce_t *load_requests_busy[MEM_TICKS];
  reg_mut_t *store_requests[MEM_TICKS];
  reg_reduce_t *store_requests_busy[MEM_TICKS];
  reg_mut_t *clear; // bool
};

memory_t *mem_create (bus_t *cdb, clk_t *clk);
void mem_free (memory_t *mem);

void mem_set (memory_t *mem, addr_t addr, byte_t value);
byte_t mem_get (memory_t *mem, addr_t addr);
word_t mem_get_inst (memory_t *mem, addr_t addr);

status_t mem_request_load (memory_t *mem, addr_t addr,
  ls_size_t size, cdb_message_t base_msg);
status_t mem_request_store (memory_t *mem, addr_t addr,
  word_t value, ls_size_t size, reg_mut_t *callback);

void mem_clear (memory_t *mem);
