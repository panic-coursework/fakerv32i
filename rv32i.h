#pragma once

#include <sys/types.h>

typedef unsigned char byte_t;
typedef __uint32_t word_t;

typedef struct clk_t clk_t;
typedef struct reg_t reg_t;
typedef struct reg_mut_t reg_mut_t;

typedef struct memory_t memory_t;
typedef struct instr_queue_t instr_queue_t;
typedef struct reg_store_t reg_store_t;
typedef struct reg_file_t reg_file_t;
typedef struct reservation_station_t reservation_station_t;
typedef struct rs_unit_t rs_unit_t;
typedef struct load_buffer_t load_buffer_t;
typedef struct store_buffer_t store_buffer_t;
typedef struct reorder_buffer_t reorder_buffer_t;
typedef struct execution_unit_t execution_unit_t;
typedef struct common_data_bus_t common_data_bus_t;
