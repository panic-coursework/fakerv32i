#include "alu-ops.h"
#include "lib/log.h"
#include "rv32i.h"
#include <assert.h>

word_t _signed_lt (word_t value1, word_t value2) {
  signed_word_t v1 = value1, v2 = value2;
  return v1 < v2;
}
word_t _signed_rshift (word_t value1, word_t value2) {
  signed_word_t v1 = value1, v2 = value2;
  return v1 << v2;
}

word_t alu_execute (alu_op_t op, word_t value1,
                    word_t value2) {
  switch (op) {
    case ALU_ADD: return value1 + value2;
    case ALU_SLL: return value1 << value2;
    case ALU_SLT: return _signed_lt(value1, value2);
    case ALU_SLTU: return value1 < value2;
    case ALU_XOR: return value1 ^ value2;
    case ALU_SRL: return value1 >> value2;
    case ALU_OR: return value1 | value2;
    case ALU_AND: return value1 & value2;
    case ALU_SUB: return value1 - value2;
    case ALU_SRA: return _signed_rshift(value1, value2);
    case ALU_BEQ: return value1 == value2;
    case ALU_BNE: return value1 != value2;
    case ALU_BLT: return _signed_lt(value1, value2);
    case ALU_BGE: return !_signed_lt(value1, value2);
    case ALU_BLTU: return value1 < value2;
    case ALU_BGEU: return value1 >= value2;
    default:
    debug_log("unknown ALU op %d!", op);
    assert(0);
  }
  return 0;
}
