#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#include "reservation-station.h"
#include "bus.h"
#include "clk.h"
#include "common-data-bus.h"
#include "lib/closure.h"
#include "lib/log.h"
#include "lib/vector.h"
#include "reg.h"
#include "rv32i.h"

void _rs_onmsg (void *state, ...) {
  va_list args; va_start(args, state);
  cdb_message_t *msg = va_arg(args, cdb_message_t *);
  va_end(args);
  reservation_station_t *rs = (reservation_station_t *) state;
  if (!*rm_read(rs->busy, bool)) return;
  if (rs->id == msg->rs) {
    rm_write(rs->busy, bool) = false;
    return;
  }
  rs_payload_t data = *rm_read(rs->payload, rs_payload_t);
  bool modified = false;
  if (data.src1 == rs->id) {
    modified = true;
    data.value1 = msg->result;
    data.src1 = 0;
  }
  if (data.src2 == rs->id) {
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
void _rs_unit_tick (void *state, ...) {
  rs_unit_t *unit = (rs_unit_t *) state;
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

void _rsu_check_alu (rs_unit_t *unit, vector_t *rss) {
  // TODO
}
void _rsu_check_load (rs_unit_t *unit, vector_t *rss) {
  // TODO
}
void _rsu_check_store (rs_unit_t *unit, vector_t *rss) {
  // TODO
}
