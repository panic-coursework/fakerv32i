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
  clk_t *clk;
};

bus_t *bus_create(size_t size, clk_t *clk);
void bus_free(bus_t *bus);

size_t bus_arb_add(bus_t *bus);

void bus_arb_req(bus_t *bus, size_t id);
bool bus_arb_status(bus_t *bus, size_t id);
