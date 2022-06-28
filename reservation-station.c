#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ls-queue.h"
#include "reservation-station.h"
#include "alu.h"
#include "bus.h"
#include "clk.h"
#include "common-data-bus.h"
#include "lib/closure.h"
#include "lib/log.h"
#include "lib/vector.h"
#include "queue.h"
#include "reg.h"
#include "reorder-buffer.h"
#include "rv32i.h"
#include "rvmath.h"

void _rs_onmsg (void *state, ...) {
  va_list args; va_start(args, state);
  cdb_message_t *msg = va_arg(args, cdb_message_t *);
  va_end(args);
  reservation_station_t *rs = (reservation_station_t *) state;
  if (!*rm_read(rs->busy, bool)) return;
  rs_payload_t data = *rm_read(rs->payload, rs_payload_t);
  bool modified = false;
  if (data.src1 == msg->rob) {
    modified = true;
    data.value1 = msg->result;
    data.src1 = 0;
  }
  if (data.src2 == msg->rob) {
    modified = true;
    data.value2 = msg->result;
    data.src2 = 0;
  }
  if (modified) rm_write(rs->payload, rs_payload_t) = data;
}
reservation_station_t *rs_create (rs_id_t id,
                                  rs_type_t type,
                                  bus_t *cdb,
                                  clk_t *clk) {
  reservation_station_t *rs = (reservation_station_t *)
    malloc(sizeof(reservation_station_t));
  rs->id = id;
  rs->type = type;
  rs->busy = reg_mut_create(sizeof(bool), clk);
  rs->payload = reg_mut_create(sizeof(rs_payload_t), clk);
  bus_listen(cdb, closure_create(_rs_onmsg, rs));
  return rs;
}
void rs_free (reservation_station_t *rs) {
  reg_mut_free(rs->busy);
  reg_mut_free(rs->payload);
  free(rs);
}

bool _rs_ready (reservation_station_t *rs) {
  rs_payload_t data = *rm_read(rs->payload, rs_payload_t);
  return data.src1 == 0 && data.src2 == 0;
}

void _rsu_check_alu (rs_unit_t *unit, vector_t *rss);
void _rsu_check_load (rs_unit_t *unit, vector_t *rss);
void _rsu_check_store (rs_unit_t *unit, vector_t *rss);
void _rsu_clear (vector_t *rss) {
  reservation_station_t *rs;
  vector_foreach (rss, i, rs) {
    rm_write(rs->busy, bool) = false;
  }
}
void _rs_unit_tick (void *state, ...) {
  rs_unit_t *unit = (rs_unit_t *) state;
  if (*rm_read(unit->clear, bool)) {
    rm_write(unit->clear, bool) = false;
    _rsu_clear(unit->alu_stations);
    _rsu_clear(unit->load_buffers);
    _rsu_clear(unit->store_buffers);
    return;
  }
  _rsu_check_alu(unit, unit->alu_stations);
  _rsu_check_load(unit, unit->load_buffers);
  _rsu_check_store(unit, unit->store_buffers);
}
rs_unit_t *rs_unit_create (alu_pool_t *alu_pool,
                           ls_queue_t *queue, clk_t *clk) {
  rs_unit_t *unit = (rs_unit_t *) malloc(sizeof(rs_unit_t));
  unit->alu_pool = alu_pool;
  unit->ls_queue = queue;
  unit->alu_stations = vector_create();
  unit->load_buffers = vector_create();
  unit->store_buffers = vector_create();
  unit->id_map = vector_create();
  unit->clear = reg_mut_create(sizeof(bool), clk);

  clk_add_callback(clk, closure_create(_rs_unit_tick, unit));

  return unit;
}
void _rs_vec_free (vector_t *vec) {
  void *ptr;
  vector_foreach (vec, i, ptr) free(ptr);
  vector_free(vec);
}
void rs_unit_free (rs_unit_t *rs) {
  _rs_vec_free(rs->alu_stations);
  _rs_vec_free(rs->load_buffers);
  _rs_vec_free(rs->store_buffers);
  _rs_vec_free(rs->id_map);
  reg_mut_free(rs->clear);
  free(rs);
}

vector_t *_rs_get_vec (rs_unit_t *unit, rs_type_t type) {
  vector_t *vec;
  switch (type) {
    case RS_RESERVATION_STATION: return unit->alu_stations;
    case RS_LOAD_BUFFER: return unit->load_buffers;
    case RS_STORE_BUFFER: return unit->store_buffers;
    default:
    debug_log("unknown rs type %d", type);
    assert(false);
  }
}
void rs_unit_add (rs_unit_t *unit,
                  reservation_station_t *rs) {
  vector_push(_rs_get_vec(unit, rs->type), rs);
}

reservation_station_t *rs_unit_acquire (rs_unit_t *unit,
                                        rs_type_t type) {
  vector_t *rss = _rs_get_vec(unit, type);
  reservation_station_t *rs;
  vector_foreach (rss, i, rs) {
    if (!*rm_read(rs->busy, bool)) return rs;
  }
  return NULL;
}

void rs_unit_clear (rs_unit_t *unit) {
  rm_write(unit->clear, bool) = true;
}

addr_t _rs_calc_addr (rs_payload_t data) {
  return data.addr + data.value1;
}
void _rsu_check_alu (rs_unit_t *unit, vector_t *rss) {
  reservation_station_t *rs;
  int alu_id = 0;
  vector_foreach (rss, i, rs) {
    assert(rs->type == RS_RESERVATION_STATION);
    if (!*rm_read(rs->busy, bool)) continue;
    rs_payload_t data = *rm_read(rs->payload, rs_payload_t);
    if (data.src1 != 0 || data.src2 != 0) continue;
    rm_write(rs->busy, bool) = false;
    alu_task(unit->alu_pool->alus[alu_id++], (alu_task_t) {
      .busy = true,
      .base_msg = (cdb_message_t) {
        .rob = data.dest,
      },
      .op = data.op_alu,
      .value1 = data.value1,
      .value2 = data.value2,
    });
    if (alu_id >= ALU_COUNT) break;
  }
}
bool _rsu_overlap (addr_t addr1, ls_size_t size1,
                   addr_t addr2, ls_size_t size2) {
  if (addr1 == addr2) return true;
  if (addr1 > addr2) {
    swap(addr1, addr2, addr_t);
    swap(size1, size2, ls_size_t);
  }
  assert(addr1 < addr2);
  return addr1 + ls_size(size1) <= addr2;
}
void _rsu_check_load (rs_unit_t *unit, vector_t *rss) {
  if (ls_queue_full(unit->ls_queue)) return;
  reservation_station_t *rs;
  vector_foreach (rss, i, rs) {
    assert(rs->type == RS_LOAD_BUFFER);
    if (!*rm_read(rs->busy, bool)) continue;
    rs_payload_t data = *rm_read(rs->payload, rs_payload_t);
    if (data.src1 != 0 || data.src2 != 0) continue;
    addr_t addr = _rs_calc_addr(data);
    reg_mut_t *reg;
    bool blocking = false;
    queue_foreach (unit->rob_unit->robs, i, reg) {
      const reorder_buffer_t *rob =
        rm_read(reg, reorder_buffer_t);
      const rob_payload_t *rob_data =
        rm_read(rob->payload, rob_payload_t);
      if (rob_data->op == ROB_STORE) {
        blocking = !rob_data->addr_ready ||
          _rsu_overlap(addr, data.op_ls,
                       rob_data->addr, rob_data->size);
        if (blocking) break;
      }
    }
    if (blocking) continue;
    // not blocking, and ready to issue.
    // discard results as we are sure the queue isn't full.
    ls_queue_push(unit->ls_queue, (ls_queue_payload_t) {
      .addr = addr,
      .base_msg = (cdb_message_t) {
        .rob = data.dest,
      },
      .op = LS_LOAD,
      .size = data.op_ls,
    });
    // only write one command at a time.
    break;
  }
}
void _rsu_check_store (rs_unit_t *unit, vector_t *rss) {
  if (ls_queue_full(unit->ls_queue)) return;
  reservation_station_t *rs;
  vector_foreach (rss, i, rs) {
    assert(rs->type == RS_LOAD_BUFFER);
    if (!*rm_read(rs->busy, bool)) continue;
    rs_payload_t data = *rm_read(rs->payload, rs_payload_t);
    if (data.src1 == 0 || data.src2 == 0) {
      const reorder_buffer_t *rob =
        rob_unit_find(unit->rob_unit, data.dest);
      rob_payload_t old =
        *rm_read(rob->payload, rob_payload_t);
      if (old.addr_ready && old.value_ready) continue;
      if (!old.addr_ready && data.src1 == 0) {
        old.addr = _rs_calc_addr(data);
        old.addr_ready = true;
      }
      if (!old.value_ready && data.src2 == 0) {
        old.value = data.value2;
        old.value_ready = true;
      }
      rm_write(rob->payload, rob_payload_t) = old;
      if (data.src1 == 0 && data.src2 == 0) {
        rm_write(rs->busy, bool) = false;
      }
      // write one rob entry at a time
      break;
    }
  }
}
