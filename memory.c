#include <assert.h>
#include <endian.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "memory.h"
#include "bus.h"
#include "common-data-bus.h"
#include "clk.h"
#include "lib/closure.h"
#include "lib/log.h"
#include "queue.h"
#include "reg.h"
#include "rv32i.h"
#include "rvmath.h"

#if BYTE_ORDER != LITTLE_ENDIAN
#error This program runs only on little endian architectures
#endif

word_t _mem_get_sz (memory_t *mem, addr_t addr,
                    ls_size_t size) {
  void *base = &mem->data[addr];
  switch (size) {
    case LS_BYTE: return sign_extend(*(__uint8_t *) base, 8);
    case LS_BYTEU: return *(__uint8_t *) base;
    case LS_HWORD: return sign_extend(*(__uint16_t *) base, 16);
    case LS_HWORDU: return *(__uint16_t *) base;
    case LS_WORD: return *(__uint32_t *) base;
    default:
    debug_log("invalid size %d", size);
    assert(0);
  }
}
void _mem_set_sz (memory_t *mem, addr_t addr,
                  word_t value, ls_size_t size) {
  void *base = &mem->data[addr];
  switch (size) {
    case LS_BYTE: *(__uint8_t *) base = value; break;
    case LS_HWORD: *(__uint16_t *) base = value; break;
    case LS_WORD: *(__uint32_t *) base = value; break;
    default:
    debug_log("invalid size %d", size);
    assert(0);
  }
}
void _mem_tick (void *state, va_list args) {
  memory_t *mem = (memory_t *) state;
  if (*rm_read(mem->clear, bool)) {
    for (int i = 0; i < MEM_TICKS; ++i) {
      bool busy = *rr_read(mem->store_requests_busy[i], bool);
      reg_reduce_write(mem->store_requests_busy[i], &busy);
    }
    return;
  }

  bool load_requests_busy[MEM_TICKS];
  bool store_requests_busy[MEM_TICKS];
  for (int i = 0; i < MEM_TICKS; ++i) {
    load_requests_busy[i] =
      *rr_read(mem->load_requests_busy[i], bool);
    store_requests_busy[i] =
      *rr_read(mem->store_requests_busy[i], bool);
  }

  bool cdb_sent = false;
  // load
  for (int i = 0; i < MEM_TICKS; ++i) {
    if (!load_requests_busy[i]) continue;
    const struct mem_load_request_t *req =
      rm_read(mem->load_requests[i],
              struct mem_load_request_t);
    struct mem_load_request_t *req_next =
      &rm_write(mem->load_requests[i],
                struct mem_load_request_t);
    int ticks_remaining = --req_next->ticks_remaining;
    if (!cdb_sent && ticks_remaining <= 0) {
      // the common data bus cannot handle two msgs at once.
      cdb_sent = true;
      reg_mut_t *reg = bh_acquire(mem->cdb_helper);
      if (!reg) continue;
      cdb_message_t msg = req->base_msg;
      msg.result = _mem_get_sz(mem, req->addr, req->size);
      debug_log("mem writing to ROB #%02d addr %08x value %08x",
                msg.rob, req->addr, msg.result);
      rm_write(reg, cdb_message_t) = msg;
      load_requests_busy[i] = false;
      continue;
    }
  }
  // store
  for (int i = 0; i < MEM_TICKS; ++i) {
    if (!store_requests_busy[i]) continue;
    const struct mem_store_request_t *req =
      rm_read(mem->store_requests[i],
              struct mem_store_request_t);
    struct mem_store_request_t *req_next =
      &rm_write(mem->store_requests[i],
                struct mem_store_request_t);
    req_next->ticks_remaining = req->ticks_remaining - 1;
    if (req_next->ticks_remaining == 0) {
      _mem_set_sz(mem, req->addr, req->value, req->size);
      debug_log("store addr %08x complete!", req->addr);
      store_requests_busy[i] = false;
      rm_write(req->callback, bool) = true;
    }
  }
  for (int i = 0; i < MEM_TICKS; ++i) {
    reg_reduce_write(mem->load_requests_busy[i],
      &load_requests_busy[i]);
    reg_reduce_write(mem->store_requests_busy[i],
      &store_requests_busy[i]);
  }
}
memory_t *mem_create (bus_t *cdb, clk_t *clk) {
  memory_t *mem = (memory_t *) malloc(sizeof(memory_t));
  memset(mem->data, 0, sizeof(mem->data));
  for (int i = 0; i < MEM_TICKS; ++i) {
    mem->load_requests[i] =
      reg_mut_create(sizeof(struct mem_load_request_t), clk);
    mem->store_requests[i] =
      reg_mut_create(sizeof(struct mem_store_request_t), clk);
    mem->load_requests_busy[i] = reg_or_create(clk);
    mem->store_requests_busy[i] = reg_or_create(clk);
  }
  mem->cdb = cdb;
  mem->cdb_helper = bh_create(cdb);
  mem->clear = reg_mut_create(sizeof(bool), clk);
  mem->clear->clear = true;
  clk_add_callback(clk, closure_create(_mem_tick, mem));
  return mem;
}
void mem_free (memory_t *mem) {
  for (int i = 0; i < MEM_TICKS; ++i) {
    reg_mut_free(mem->load_requests[i]);
    reg_mut_free(mem->store_requests[i]);
    reg_reduce_free(mem->load_requests_busy[i]);
    reg_reduce_free(mem->store_requests_busy[i]);
  }
  bh_free(mem->cdb_helper);
  free(mem);
}

void mem_set (memory_t *mem, addr_t addr, byte_t value) {
  mem->data[addr] = value;
}
byte_t mem_get (memory_t *mem, addr_t addr) {
  if (addr >= MEM_SIZE) {
    debug_log("bad mem access addr %08x", addr);
    return 0;
  }
  return mem->data[addr];
}
word_t mem_get_inst (memory_t *mem, addr_t addr) {
  return _mem_get_sz(mem, addr, LS_WORD);
}

int _mem_acquire_load_req (memory_t *mem) {
  for (int i = 0; i < MEM_SIZE; ++i) {
    if (!*rr_read(mem->load_requests_busy[i], bool)) {
      bool t = true;
      reg_reduce_write(mem->load_requests_busy[i], &t);
      return i;
    }
  }
  return -1;
}
int _mem_acquire_store_req (memory_t *mem) {
  for (int i = 0; i < MEM_SIZE; ++i) {
    if (!*rr_read(mem->store_requests_busy[i], bool)) {
      bool t = true;
      reg_reduce_write(mem->store_requests_busy[i], &t);
      return i;
    }
  }
  return -1;
}
status_t mem_request_load (memory_t *mem, addr_t addr,
  ls_size_t size, cdb_message_t base_msg) {
  int id = _mem_acquire_load_req(mem);
  if (id < 0) return STATUS_FAIL;
  struct mem_load_request_t *req =
    &rm_write(mem->load_requests[id],
              struct mem_load_request_t);
  req->addr = addr;
  req->size = size;
  req->ticks_remaining = MEM_TICKS - 1;
  req->base_msg = base_msg;
  return STATUS_SUCCESS;
}
status_t mem_request_store (memory_t *mem, addr_t addr,
  word_t value, ls_size_t size, reg_mut_t *callback) {
  int id = _mem_acquire_store_req(mem);
  if (id < 0) return STATUS_FAIL;
  struct mem_store_request_t *req =
    &rm_write(mem->store_requests[id],
              struct mem_store_request_t);
  req->addr = addr;
  req->value = value;
  req->callback = callback;
  req->size = size;
  req->ticks_remaining = MEM_TICKS - 1;
  return STATUS_SUCCESS;
}

void mem_clear (memory_t *mem) {
  rm_write(mem->clear, bool) = true;
}
