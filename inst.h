#pragma once

#include "rv32i.h"

#define INST_SIZE 4

enum inst_op_t {
  // load
  INST_LB  = 0b0000000011,
  INST_LH  = 0b0010000011,
  INST_LW  = 0b0100000011,
  INST_LBU = 0b1000000011,
  INST_LHU = 0b1010000011,

  // store
  INST_SB  = 0b0000100011,
  INST_SH  = 0b0010100011,
  INST_SW  = 0b0100100011,

  // immediate alu ops
  INST_ADDI  = 0b00000010011,
  INST_SLLI  = 0b00010010011,
  INST_SLTI  = 0b00100010011,
  INST_SLTIU = 0b00110010011,
  INST_XORI  = 0b01000010011,
  INST_SRLI  = 0b01010010011,
  INST_SRAI  = 0b11010010011,
  INST_ORI   = 0b01100010011,
  INST_ANDI  = 0b01110010011,

  INST_AUIPC = 0b0010111,

  // register alu ops
  INST_ADD  = 0b00000110011,
  INST_SUB  = 0b10000110011,
  INST_SLL  = 0b00010110011,
  INST_SLT  = 0b00100110011,
  INST_SLTU = 0b00110110011,
  INST_XOR  = 0b01000110011,
  INST_SRL  = 0b01010110011,
  INST_SRA  = 0b11010110011,
  INST_OR   = 0b01100110011,
  INST_AND  = 0b01110110011,

  INST_LUI = 0b0110111,

  // branch
  INST_BEQ  = 0b0001100011,
  INST_BNE  = 0b0011100011,
  INST_BLT  = 0b1001100011,
  INST_BGE  = 0b1011100011,
  INST_BLTU = 0b1101100011,
  INST_BGEU = 0b1111100011,

  INST_JALR = 0b1100111,
  INST_JAL  = 0b1101111,
};
typedef enum inst_op_t inst_op_t;

// opcodes
enum opcodes_t {
  OPC_LOAD   = 0b0000011,
  OPC_STORE  = 0b0100011,
  OPC_ALUI   = 0b0010011,
  OPC_ALUR   = 0b0110011,
  OPC_AUIPC  = 0b0010111,
  OPC_LUI    = 0b0110111,
  OPC_BRANCH = 0b1100011,
  OPC_JALR   = 0b1100111,
  OPC_JAL    = 0b1101111,
};
typedef enum opcodes_t opcodes_t;

struct inst_t {
  inst_op_t op;
  word_t immediate;
  reg_id_t rs1, rs2;
  reg_id_t rd;
};

inst_t inst_decode (word_t inst);
rs_type_t inst_type (inst_t inst);
