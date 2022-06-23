#pragma once

#include "rv32i.h"

enum inst_op_t {
  // load
  INST_LB  = 0b000'0000011,
  INST_LH  = 0b001'0000011,
  INST_LW  = 0b010'0000011,
  INST_LBU = 0b100'0000011,
  INST_LHU = 0b101'0000011,

  // store
  INST_SB  = 0b000'0100011,
  INST_SH  = 0b001'0100011,
  INST_SW  = 0b010'0100011,

  // immediate alu ops
  INST_ADDI  = 0b0'000'0010011,
  INST_SLLI  = 0b0'001'0010011,
  INST_SLTI  = 0b0'010'0010011,
  INST_SLTIU = 0b0'011'0010011,
  INST_XORI  = 0b0'100'0010011,
  INST_SRLI  = 0b0'101'0010011,
  INST_SRAI  = 0b1'101'0010011,
  INST_ORI   = 0b0'110'0010011,
  INST_ANDI  = 0b0'111'0010011,

  INST_AUIPC = 0b0010111,

  // register alu ops
  INST_ADD  = 0b0'000'0110011,
  INST_SUB  = 0b1'000'0110011,
  INST_SLL  = 0b0'001'0110011,
  INST_SLT  = 0b0'010'0110011,
  INST_SLTU = 0b0'011'0110011,
  INST_XOR  = 0b0'100'0110011,
  INST_SRL  = 0b0'101'0110011,
  INST_SRA  = 0b1'101'0110011,
  INST_OR   = 0b0'110'0110011,
  INST_AND  = 0b0'111'0110011,

  INST_LUI = 0b0110111,

  // branch
  INST_BEQ  = 0b000'1100011,
  INST_BNE  = 0b001'1100011,
  INST_BLT  = 0b100'1100011,
  INST_BGE  = 0b101'1100011,
  INST_BLTU = 0b110'1100011,
  INST_BGEU = 0b111'1100011,

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
