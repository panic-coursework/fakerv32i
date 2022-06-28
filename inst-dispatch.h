#pragma once

#include "inst.h"
#include "rv32i.h"

addr_t inst_dispatch (inst_unit_t *unit,
                      reservation_station_t *rs,
                      reorder_buffer_t *rob,
                      inst_t inst,
                      addr_t pc);
