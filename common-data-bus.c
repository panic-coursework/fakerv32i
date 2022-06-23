#include <stdlib.h>

#include "common-data-bus.h"
#include "bus.h"

bus_t *cdb_create (clk_t *clk) {
  return bus_create(sizeof(cdb_message_t), clk);
}
