#pragma once

#include <sys/types.h>

#define REG_COUNT 32

typedef enum status_t {
  STATUS_FAIL = -1,
  STATUS_SUCCESS = 0,
} status_t;

typedef enum ls_size_t {
  LS_BYTE,
  LS_HWORD,
  LS_WORD, // 0b010
  LS_BYTEU = 0b100,
  LS_HWORDU = 0b101,
} ls_size_t;

typedef __uint8_t byte_t;
typedef __uint32_t word_t;
typedef word_t addr_t;
typedef int rs_id_t;
typedef int reg_id_t;
typedef int rob_id_t;

typedef struct clk_t clk_t;
typedef struct reg_t reg_t;
typedef struct reg_mut_t reg_mut_t;
typedef struct reg_reduce_t reg_reduce_t;

typedef struct inst_t inst_t;

typedef struct memory_t memory_t;
typedef struct inst_queue_t inst_queue_t;
typedef struct reg_store_t reg_store_t;
typedef struct reg_file_t reg_file_t;
typedef struct reservation_station_t reservation_station_t;
typedef struct rs_payload_t rs_payload_t;
typedef struct rs_unit_t rs_unit_t;
typedef struct load_buffer_t load_buffer_t;
typedef struct store_buffer_t store_buffer_t;
typedef struct reorder_buffer_t reorder_buffer_t;
typedef struct rob_unit_t rob_unit_t;
typedef struct alu_t alu_t;
typedef struct alu_task_t alu_task_t;
typedef struct alu_pool_t alu_pool_t;
typedef struct bus_t bus_t;
typedef struct bus_helper_t bus_helper_t;
typedef struct cdb_message_t cdb_message_t;

#define swap(x, y, type) \
  {                      \
    type _swap_tmp = x;  \
    x = y;               \
    y = _swap_tmp;       \
  }

enum alu_op_t {
  ALU_ADD,
  ALU_SLL,
  ALU_SLT,
  ALU_SLTU,
  ALU_XOR,
  ALU_SRL,
  ALU_OR,
  ALU_AND, // 0b111
  ALU_SUB = 0b1000,
  ALU_SRA = 0b1101,
};
typedef enum alu_op_t alu_op_t;

typedef enum rs_type_t {
  RS_RESERVATION_STATION,
  RS_LOAD_BUFFER,
  RS_STORE_BUFFER,
} rs_type_t;

typedef enum rob_op_t {
  ROB_REGISTER,
  ROB_STORE,
  ROB_BRANCH,
} rob_op_t;
