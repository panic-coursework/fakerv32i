#pragma once

#include <stdbool.h>

#include "common-data-bus.h"
#include "lib/closure.h"
#include "rv32i.h"

struct alu_task_t {
  alu_op_t op;
  word_t value1, value2;
  cdb_message_t base_msg;
  bool busy;
};

struct alu_t {
  bus_t *cdb;
  bus_helper_t *cdb_helper;
  reg_mut_t *task;
  reg_mut_t *result_buf;
};

alu_t *alu_create (bus_t *cdb);
void alu_free (alu_t *alu);

status_t alu_task (alu_t *alu, alu_task_t task);

#define ALU_COUNT 2

struct alu_pool_t {
  alu_t *alus[ALU_COUNT];
};

alu_pool_t *alu_pool_create (bus_t *cdb);
void alu_pool_free (alu_pool_t *alu_pool);
