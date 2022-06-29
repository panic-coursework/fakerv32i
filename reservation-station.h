#pragma once

#include <stdbool.h>

#include "lib/vector.h"
#include "rv32i.h"

// TODO: private or public?
struct rs_payload_t {
  alu_op_t op_alu;
  ls_size_t op_ls;
  // for LOAD, value1 = addr
  // for STORE, value1 = addr, value2 = value
  word_t value1, value2;
  rob_id_t src1, src2;
  rob_id_t dest;
  addr_t addr;
};

struct reservation_station_t {
  rs_id_t id;
  rs_type_t type;
  reg_mut_t *busy;
  reg_mut_t *payload;
  rs_unit_t *unit;
};

struct rs_unit_t {
  alu_pool_t *alu_pool;
  ls_queue_t *ls_queue;
  rob_unit_t *rob_unit; // need to be manually handled
  vector_t *alu_stations;
  vector_t *load_buffers;
  vector_t *store_buffers;
  // TODO: maintain it
  vector_t *id_map;
  reg_mut_t *clear;
};

reservation_station_t *rs_create (rs_id_t id,
                                  rs_type_t type,
                                  bus_t *cdb,
                                  clk_t *clk);
void rs_free (reservation_station_t *rs);

rs_unit_t *rs_unit_create (alu_pool_t *alu_pool,
                           ls_queue_t *queue, clk_t *clk);
void rs_unit_free (rs_unit_t *rs);

void rs_unit_add (rs_unit_t *unit,
                  reservation_station_t *rs);

reservation_station_t *rs_unit_acquire (rs_unit_t *unit,
                                        rs_type_t type);
// TODO: is this used?
reservation_station_t *rs_unit_find (rs_unit_t *unit,
                                     rs_id_t id);
void rs_unit_clear (rs_unit_t *unit);
