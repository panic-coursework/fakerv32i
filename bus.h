#pragma once

#include "lib/vector.h"
#include "rv32i.h"

#include <stdbool.h>

struct bus_arbitrator_t {
  vector_t *req; // request
  reg_t *gnt;    // grant
};

struct bus_t {
  struct bus_arbitrator_t arbitrator;
  reg_mut_t *data;
  size_t size;
  clk_t *clk;
};

bus_t *bus_create (size_t size, clk_t *clk);
void bus_free (bus_t *bus);

size_t bus_arb_add (bus_t *bus);

void bus_arb_req (bus_t *bus, size_t id);
bool bus_arb_status (bus_t *bus, size_t id);

#define BUS_TICKS 2

struct bus_helper_t {
  reg_mut_t *buffer[BUS_TICKS];
  reg_reduce_t *busy[BUS_TICKS];
  reg_mut_t *current;
  bus_t *bus;
  size_t id;
};

bus_helper_t *bh_create (bus_t *bus);
void bh_free (bus_helper_t *bh);

bool bh_should_stall (bus_helper_t *bh);
reg_mut_t *bh_acquire (bus_helper_t *bh);
