#pragma once

#include <stdbool.h>

#include "lib/vector.h"
#include "rv32i.h"

// TODO: private or public?
struct rs_payload_t {
  alu_op_t op_alu;
  ls_size_t op_ls;
  word_t value1, value2;
  rs_id_t src1, src2;
  rob_id_t dest;
  addr_t addr;
};

struct reservation_station_t {
  rs_id_t id;
  rs_type_t type;
  reg_mut_t *busy;
  reg_mut_t *payload;
};

struct rs_unit_t {
  alu_pool_t *alu_pool;
  ls_queue_t *ls_queue;
  rob_unit_t *rob;
  vector_t *alu_stations;
  vector_t *load_buffers;
  vector_t *store_buffers;
  vector_t *id_map;
};

// TODO: _rs_tick
reservation_station_t *rs_create (rs_id_t id,
  rs_type_t type, clk_t *clk);
void rs_free (reservation_station_t *rs);

// TODO: _rs_unit_tick
rs_unit_t *rs_unit_create (alu_pool_t *alu_pool,
                           ls_queue_t *queue, clk_t *clk);
void rs_unit_free (rs_unit_t *rs);

void rs_unit_add_alu_station (rs_unit_t *unit,
                              reservation_station_t *rs);
void rs_unit_add_load_buffer (rs_unit_t *unit,
                              reservation_station_t *rs);
void rs_unit_add_store_buffer (rs_unit_t *unit,
                               reservation_station_t *rs);
