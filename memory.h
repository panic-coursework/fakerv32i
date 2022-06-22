#pragma once

#include "rv32i.h"

#include <stdbool.h>

#define MEM_SIZE 131072
#define MEM_TICKS 3

enum mem_size_t {
  mem_byte,
  mem_hword,
  mem_word,
};

struct mem_load_request_t {
  bool busy;
  addr_t addr;
  enum mem_size_t size;
  int ticks_remaining;
};
struct mem_store_request_t {
  bool busy;
  addr_t addr;
  word_t value;
  enum mem_size_t size;
  int ticks_remaining;
};

struct memory_t {
  byte_t data[MEM_SIZE];
  bus_t *cdb;
  struct mem_load_request_t load_requests[MEM_TICKS];
  struct mem_store_request_t store_requests[MEM_TICKS];
};

// TODO: should we have mem_tick?
memory_t *mem_create (bus_t *cdb, clk_t *clk);
void mem_free (memory_t *mem);

void mem_set (memory_t *mem, addr_t addr, byte_t value);
byte_t mem_get (memory_t *mem, addr_t addr);

void mem_request_load (memory_t *mem, addr_t addr,
                       enum mem_size_t size);
void mem_request_store (memory_t *mem, addr_t addr,
                        word_t value, enum mem_size_t size);
