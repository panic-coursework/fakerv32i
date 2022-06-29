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

  // Halt and Catch Fire
  INST_HCF  = 0xfee1dead,

  // Invalid instruction
  INST_UNIMP = 0xbada991e,
};
typedef enum inst_op_t inst_op_t;

enum inst_format_t {
  IF_I,
  IF_U,
  IF_R,
  IF_J,
  IF_B,
  IF_S,
  IF_UNIMP,
};
typedef enum inst_format_t inst_format_t;

// opcodes
enum opcodes_t {
  OPC_LOAD   = 0b0000011,
  OPC_STORE  = 0b0100011,
  OPC_IMM    = 0b0010011,
  OPC_OP     = 0b0110011,
  OPC_AUIPC  = 0b0010111,
  OPC_LUI    = 0b0110111,
  OPC_BRANCH = 0b1100011,
  OPC_JALR   = 0b1100111,
  OPC_JAL    = 0b1101111,
};
typedef enum opcodes_t opcodes_t;

struct inst_t {
  inst_op_t op;
  opcodes_t opcode;
  inst_format_t format;
  word_t immediate;
  reg_id_t rs1, rs2;
  reg_id_t rd;
  ls_size_t width;
};

inst_t inst_decode (word_t inst);
rs_type_t inst_type (inst_t inst);
rob_op_t inst_rob_op (inst_t inst);
alu_op_t inst_alu_op (inst_t inst);
ls_size_t inst_ls_op (inst_t inst);
