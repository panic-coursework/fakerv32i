#pragma once

#include "rv32i.h"

struct cdb_message_t {
  rob_id_t rob;
  word_t result;
};

bus_t *cdb_create (clk_t *clk);
