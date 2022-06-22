#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "bus.h"
#include "lib/closure.h"
#include "lib/vector.h"
#include "reg.h"
#include "rv32i.h"

void _bus_arb (void *state, ...) {
  va_list args;
  va_start(args, state);
  size_t *gnt = va_arg(args, size_t *);
  *gnt = 0;

  bus_t *bus = (bus_t *) state;
  reg_mut_t *reg;
  vector_foreach(bus->arbitrator.req, i, reg) {
    if (*rm_read(reg, bool)) {
      *gnt = i;
      break;
    }
  }
}

bus_t *bus_create (size_t size, clk_t *clk) {
  bus_t *bus = (bus_t *) malloc(sizeof(bus_t));
  bus->arbitrator.req = vector_create();
  bus->arbitrator.gnt = reg_create(sizeof(size_t),
    closure_create(_bus_arb, bus), clk);
  bus->data = reg_mut_create(size, clk);
  bus->clk = clk;
  return bus;
}

void bus_free (bus_t *bus) {
  vector_free(bus->arbitrator.req);
  reg_free(bus->arbitrator.gnt);
  reg_mut_free(bus->data);
  free(bus);
}

size_t bus_arb_add (bus_t *bus) {
  reg_mut_t *req = reg_mut_create(sizeof(bool), bus->clk);
  size_t id = vector_push(bus->arbitrator.req, req);
  return id;
}

void bus_arb_req (bus_t *bus, size_t id) {
  reg_mut_t *req = &v_read(bus->arbitrator.req, id,
                           reg_mut_t);
  rm_write(req, bool) = true;
}
bool bus_arb_status (bus_t *bus, size_t id) {
  return *r_read(bus->arbitrator.gnt, size_t) == id;
}
