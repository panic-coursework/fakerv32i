#pragma once

#include "rv32i.h"

struct cdb_message_t {
  rs_id_t rs;
  word_t result;
};

bus_t *cdb_create (clk_t *clk);
