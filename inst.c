#include <assert.h>

#include "inst.h"
#include "lib/log.h"
#include "rv32i.h"
#include "rvmath.h"

#define OPCODE_MASK 0b00000000000000000000000001111111u
#define RD_MASK     0b00000000000000000000111110000000u
#define FUNCT3_MASK 0b00000000000000000111000000000000u
#define RS1_MASK    0b00000000000011111000000000000000u
#define RS2_MASK    0b00000001111100000000000000000000u
#define FUNCT7_MASK 0b11111110000000000000000000000000u

#define IMM_MASK    (FUNCT7_MASK | RS2_MASK)
#define U_IMM_MASK  (IMM_MASK | RS1_MASK | FUNCT3_MASK)

#define OPCODE_SHIFT 0
#define OPCODE_SIZE  7
#define RD_SHIFT     7
#define RD_SIZE      5
#define FUNCT3_SHIFT 12
#define FUNCT3_SIZE  3
#define RS1_SHIFT    15
#define RS2_SHIFT    20
#define FUNCT7_SHIFT 25

#define IMM_SHIFT    RS2_SHIFT
#define IMM_SIZE     12
#define U_IMM_SHIFT  FUNCT3_SHIFT

#define FUNCT7_BIT_MASK 0b0100000
#define FUNCT7_BIT_SHIFT 5

#define _inst_get(inst, type) \
  ((inst & type##_MASK) >> type##_SHIFT)

word_t _inst_jal_imm (word_t inst);
word_t _inst_branch_imm (word_t inst);
word_t _inst_store_imm (word_t inst);

inst_t inst_decode (word_t inst) {
  opcodes_t opcode = (opcodes_t) _inst_get(inst, OPCODE);
  word_t funct3 = _inst_get(inst, FUNCT3);
  word_t funct7 = _inst_get(inst, FUNCT7);
  word_t imm = _inst_get(inst, IMM);
  word_t u_immediate = inst & U_IMM_MASK;

  inst_t res;
  res.rd = _inst_get(inst, RD);
  res.rs1 = _inst_get(inst, RS1);
  res.rs2 = _inst_get(inst, RS2);
  res.width = funct3;
  res.immediate = sign_extend(imm, IMM_SIZE);
  res.opcode = opcode;

  word_t opc0 = opcode;
  word_t opc1 = opc0 | (funct3 << OPCODE_SIZE);
  word_t opc2 = opc1 | (_inst_get(funct7, FUNCT7_BIT) <<
    (OPCODE_SIZE + FUNCT3_SIZE));
  switch (opcode) {
    // I-type
    case OPC_IMM:
    case OPC_LOAD:
    case OPC_JALR:
    res.format = IF_I;
    res.op = opc1;
    if (opc1 == INST_SLLI && opc1 == INST_SRLI) {
      res.immediate = res.rs2;
    } else if (opc1 == INST_SLTIU) {
      res.immediate = imm;
    }
    break;

    // U-type
    case OPC_LUI:
    case OPC_AUIPC:
    res.format = IF_U;
    res.op = opc0;
    res.immediate = u_immediate;
    break;

    // R-type
    case OPC_OP: res.format = IF_R; res.op = opc2; break;

    // J-type
    case OPC_JAL:
    res.format = IF_J;
    res.op = opc0;
    res.immediate = _inst_jal_imm(inst);
    break;

    // B-type
    case OPC_BRANCH:
    res.format = IF_B;
    res.op = opc1;
    res.immediate = _inst_branch_imm(inst);
    break;

    // S-type
    case OPC_STORE:
    res.format = IF_S;
    res.op = opc1;
    res.immediate = _inst_store_imm(inst);
    break;

    default:
    debug_log("unknown opcode %x", inst);
    assert(0);
  }
  return res;
}

rs_type_t inst_type (inst_t inst) {
  opcodes_t opcode = (opcodes_t) inst.op & OPCODE_MASK;
  switch (opcode) {
    case OPC_LOAD: return RS_LOAD_BUFFER;
    case OPC_STORE: return RS_STORE_BUFFER;
    default: return RS_RESERVATION_STATION;
  }
}

rob_op_t inst_rob_op (inst_t inst) {
  opcodes_t opcode = (opcodes_t) inst.op & OPCODE_MASK;
  switch (opcode) {
    case OPC_STORE: return ROB_STORE;
    case OPC_JAL: case OPC_BRANCH: return ROB_BRANCH;
    case OPC_JALR: return ROB_JALR;
    default: return ROB_REGISTER;
  }
}

#define BRANCH_MASK 0b10000
alu_op_t inst_alu_op (inst_t inst) {
  return inst.op >> OPCODE_SIZE |
        (inst.opcode == OPC_BRANCH ? BRANCH_MASK : 0);
}

ls_size_t inst_ls_op (inst_t inst) {
  return inst.op >> OPCODE_SIZE;
}

#define JAL_0_MASK  0b10000000000000000000000000000000u
#define JAL_1_MASK  0b00000000000011111111000000000000u
#define JAL_2_MASK  0b01111111111000000000000000000000u
#define JAL_3_MASK  0b00000000000100000000000000000000u
#define JAL_0_SHIFT 11
#define JAL_1_SHIFT 0
#define JAL_2_SHIFT 20
#define JAL_3_SHIFT 9
#define JAL_SIZE    21

word_t _inst_jal_imm (word_t inst) {
  word_t imm = _inst_get(inst, JAL_0) |
               _inst_get(inst, JAL_1) |
               _inst_get(inst, JAL_2) |
               _inst_get(inst, JAL_3) ;
  return sign_extend(imm, JAL_SIZE);
}

#define BRANCH_0_MASK  0b10000000000000000000000000000000u
#define BRANCH_1_MASK  0b00000000000000000000000010000000u
#define BRANCH_2_MASK  0b01111110000000000000000000000000u
#define BRANCH_3_MASK  0b00000000000000000000111100000000u
#define BRANCH_0_SHIFT 19
#define BRANCH_1_SHIFT 7 // We don't want negative shifts
#define BRANCH_1_LSHIFT 11
#define BRANCH_2_SHIFT 7
#define BRANCH_3_SHIFT 20
#define BRANCH_SIZE    13

word_t _inst_branch_imm (word_t inst) {
  word_t imm = _inst_get(inst, BRANCH_0) |
              (_inst_get(inst, BRANCH_1) << BRANCH_1_LSHIFT) |
               _inst_get(inst, BRANCH_2) |
               _inst_get(inst, BRANCH_3) ;
  return sign_extend(imm, BRANCH_SIZE);
}

#define STORE_SIZE 12

word_t _inst_store_imm (word_t inst) {
  word_t imm = (_inst_get(inst, FUNCT7) << RD_SIZE) |
                _inst_get(inst, RD);
  return sign_extend(imm, STORE_SIZE);
}
