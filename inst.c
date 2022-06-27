#include <assert.h>

#include "inst.h"
#include "lib/log.h"
#include "rv32i.h"

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
#define U_IMM_SHIFT  FUNCT3_SHIFT

#define FUNCT7_BIT_MASK 0b0100000
#define FUNCT7_BIT_SHIFT 5

#define _inst_get(inst, type) \
  ((inst & type##_MASK) >> type##_SHIFT)

word_t _inst_jalr_imm (word_t inst);
word_t _inst_branch_imm (word_t inst);
word_t _inst_store_imm (word_t inst);

inst_t inst_decode (word_t inst) {
  opcodes_t opcode = (opcodes_t) _inst_get(inst, OPCODE);
  word_t funct3 = _inst_get(inst, FUNCT3);
  word_t funct7 = _inst_get(inst, FUNCT7);
  word_t u_immediate = inst & U_IMM_MASK;

  inst_t res;
  res.rd = _inst_get(inst, RD);
  res.rs1 = _inst_get(inst, RS1);
  res.rs2 = _inst_get(inst, RS2);
  res.width = funct3;
  res.immediate = _inst_get(inst, IMM);

  word_t opc0 = opcode;
  word_t opc1 = opc0 | (funct3 << OPCODE_SIZE);
  word_t opc2 = opc1 | (_inst_get(funct7, FUNCT7_BIT) <<
    (OPCODE_SIZE + FUNCT3_SIZE));
  switch (opcode) {
    // I-type
    case OPC_IMM:
    case OPC_LOAD:
    case OPC_JALR:
    res.op = opc1;
    if (opc1 == INST_SLLI && opc1 == INST_SRLI) {
      res.immediate = res.rs2;
    }
    break;

    // U-type
    case OPC_LUI:
    case OPC_AUIPC:
    res.op = opc0;
    res.immediate = u_immediate;
    break;

    // R-type
    case OPC_OP: res.op = opc2; break;

    // J-type
    case OPC_JAL:
    res.op = opc0;
    res.immediate = _inst_jalr_imm(inst);
    break;

    // B-type
    case OPC_BRANCH:
    res.op = opc1;
    res.immediate = _inst_branch_imm(inst);
    break;

    // S-type
    case OPC_STORE:
    res.op = opc1;
    res.immediate = _inst_store_imm(inst);
    break;

    default:
    debug_log("unknown opcode %llx", inst);
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

#define JALR_0_MASK  0b10000000000000000000000000000000u
#define JALR_1_MASK  0b00000000000011111111000000000000u
#define JALR_2_MASK  0b01111111111000000000000000000000u
#define JALR_3_MASK  0b00000000000100000000000000000000u
#define JALR_0_SHIFT 11
#define JALR_1_SHIFT 0
#define JALR_2_SHIFT 20
#define JALR_3_SHIFT 9

word_t _inst_jalr_imm (word_t inst) {
  return _inst_get(inst, JALR_0) |
         _inst_get(inst, JALR_1) |
         _inst_get(inst, JALR_2) |
         _inst_get(inst, JALR_3) ;
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

word_t _inst_branch_imm (word_t inst) {
  return _inst_get(inst, BRANCH_0) |
        (_inst_get(inst, BRANCH_1) << BRANCH_1_LSHIFT) |
         _inst_get(inst, BRANCH_2) |
         _inst_get(inst, BRANCH_3) ;
}

word_t _inst_store_imm (word_t inst) {
  return (_inst_get(inst, FUNCT7) << RD_SIZE) |
          _inst_get(inst, RD);
}
