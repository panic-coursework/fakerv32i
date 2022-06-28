#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "bus.h"
#include "clk.h"
#include "lib/closure.h"
#include "lib/vector.h"
#include "reg.h"
#include "rv32i.h"

// TODO: improve arbitration algorithm
void _bus_arb (void *state, ...) {
  va_list args;
  va_start(args, state);
  size_t *gnt = va_arg(args, size_t *);
  *gnt = 0;

  bus_t *bus = (bus_t *) state;
  reg_mut_t *reg;
  vector_foreach(bus->arbitrator.req, i, reg) {
    if (*rm_read(reg, bool)) {
      *gnt = i + 1;
      break;
    }
  }
}
void _bus_fire (void *state, ...) {
  va_list args;
  va_start(args, state);
  bool *fire = va_arg(args, bool *);
  bus_t *bus = (bus_t *) state;
  *fire = *r_read(bus->arbitrator.gnt, size_t) != 0;
}

void _bus_tick (void *state, ...) {
  bus_t *bus = (bus_t *) state;
  if (*r_read(bus->arbitrator.fire, bool)) {
    closure_t *callback;
    const void *data = rm_read(bus->data, void);
    vector_foreach(bus->listeners, i, callback) {
      closure_call(callback, data);
    }
  }
}
bus_t *bus_create (size_t size, clk_t *clk) {
  bus_t *bus = (bus_t *) malloc(sizeof(bus_t));
  bus->arbitrator.req = vector_create();
  bus->arbitrator.gnt = reg_create(sizeof(size_t),
    closure_create(_bus_arb, bus), clk);
  bus->arbitrator.fire = reg_create(sizeof(bool),
    closure_create(_bus_fire, bus), clk);
  bus->data = reg_mut_create(size, clk);
  bus->size = size;
  bus->clk = clk;
  bus->listeners = vector_create();

  clk_add_callback(clk, closure_create(_bus_tick, bus));

  return bus;
}

void bus_free (bus_t *bus) {
  reg_mut_t *reg;
  vector_foreach (bus->arbitrator.req, i, reg) {
    reg_mut_free(reg);
  }
  vector_free(bus->arbitrator.req);
  reg_free(bus->arbitrator.gnt);
  reg_free(bus->arbitrator.fire);
  reg_mut_free(bus->data);
  closure_t *cb;
  vector_foreach (bus->listeners, i, cb) {
    closure_free(cb);
  }
  vector_free(bus->listeners);
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

const void *bus_get_data (bus_t *bus) {
  if (!*r_read(bus->arbitrator.fire, bool)) return NULL;
  return rm_read(bus->data, void);
}
void bus_listen (bus_t *bus, closure_t *callback) {
  vector_push(bus->listeners, callback);
}

// TODO: test those stuff
bool _bh_busy (bus_helper_t *bh, int id) {
  return *rr_read(bh->busy[id], bool);
}
void _bh_tick (void *state, ...) {
  bus_helper_t *bh = (bus_helper_t *) state;
  bool requested = false;
  int curr = *rm_read(bh->current, int);

  if (_bh_busy(bh, curr)) {
    bool ok = bus_arb_status(bh->bus, bh->id);
    if (ok) {
      void *buf = reg_mut_write(bh->bus->data);
      memcpy(buf, reg_mut_read(bh->buffer[curr]),
             bh->bus->size);
      bool f = false;
      reg_reduce_write(bh->busy[curr], &f);
    } else {
      bus_arb_req(bh->bus, bh->id);
      requested = true;
    }
  }
  if (!requested && _bh_busy(bh, !curr)) {
    bus_arb_req(bh->bus, bh->id);
    rm_write(bh->current, int) = !curr;
  }
}
int _bh_req_ix (bus_helper_t *bh) {
  int curr = *rm_read(bh->current, int);
  bool arb_last = *rr_read(bh->busy[curr], bool);
  if (arb_last && !bus_arb_status(bh->bus, bh->id)) {
    return curr;
  }
  if (*rr_read(bh->busy[!curr], bool)) return !curr;
  return -1;
}
bus_helper_t *bh_create (bus_t *bus) {
  bus_helper_t *bh =
    (bus_helper_t *) malloc(sizeof(bus_helper_t));
  bh->buffer[0] = reg_mut_create(bus->size, bus->clk);
  bh->buffer[1] = reg_mut_create(bus->size, bus->clk);
  bh->busy[0] = reg_or_create(bus->clk);
  bh->busy[1] = reg_or_create(bus->clk);
  bh->current = reg_mut_create(sizeof(int), bus->clk);
  bh->bus = bus;
  bh->id = bus_arb_add(bus);
  return bh;
}
void bh_free (bus_helper_t *bh) {
  reg_mut_free(bh->buffer[0]);
  reg_mut_free(bh->buffer[1]);
  reg_reduce_free(bh->busy[0]);
  reg_reduce_free(bh->busy[1]);
  reg_mut_free(bh->current);
  free(bh);
}

bool bh_should_stall (bus_helper_t *bh) {
  int ix = _bh_req_ix(bh);
  return ix >= 0 && *rr_read(bh->busy[!ix], bool);
}
reg_mut_t *bh_acquire (bus_helper_t *bh) {
  int req_ix = _bh_req_ix(bh);
  if (req_ix >= 0) {
    if (*rr_read(bh->busy[!req_ix], bool)) {
      return NULL;
    }
    bool t = true;
    reg_reduce_write(bh->busy[!req_ix], &t);
    return bh->buffer[!req_ix];
  }
  bus_arb_req(bh->bus, bh->id);
  int curr = *rm_read(bh->current, int);
  bool t = true;
  reg_reduce_write(bh->busy[curr], &t);
  return bh->buffer[curr];
}
