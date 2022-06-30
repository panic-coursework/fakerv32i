#include <assert.h>
#include <stdlib.h>

#include "alu-ops.h"
#include "bus.h"
#include "clk.h"
#include "common-data-bus.h"
#include "fakecpu.h"
#include "inst.h"
#include "lib/log.h"
#include "memory.h"
#include "rv32i.h"

fakecpu_t *fakecpu_create () {
  fakecpu_t *cpu = (fakecpu_t *) malloc(sizeof(fakecpu_t));
  cpu->clk = clk_create();
  cpu->cdb = cdb_create(cpu->clk);
  cpu->mem = mem_create(cpu->cdb, cpu->clk);
  for (int i = 0; i < 32; ++i) cpu->regs[i] = 0;
  cpu->pc = 0;
  return cpu;
}
void fakecpu_free (fakecpu_t *cpu) {
  mem_free(cpu->mem);
  bus_free(cpu->cdb);
  clk_free(cpu->clk);
  free(cpu);
}

word_t _cpu_get (fakecpu_t *cpu, reg_id_t reg) {
  if (reg == 0) return 0;
  return cpu->regs[reg];
}
void _cpu_set (fakecpu_t *cpu, reg_id_t reg, word_t value) {
  if (reg == 0) return;
  cpu->regs[reg] = value;
}

word_t _mem_get_sz (memory_t *mem, addr_t addr,
                    ls_size_t size);
void _mem_set_sz (memory_t *mem, addr_t addr,
                  word_t value, ls_size_t size);

void fakecpu_tick (fakecpu_t *cpu) {
  word_t inst_word = mem_get_inst(cpu->mem, cpu->pc);
  inst_t inst = inst_decode(inst_word);
  if (inst.op == INST_HCF) {
    hcf = true;
    return;
  }
  if (inst.op == INST_UNIMP) {
    debug_log("unimp op %08x at addr %08x!", inst_word, cpu->pc);
    assert(0);
  }
  addr_t next_pc = cpu->pc + 4;
  switch (inst.opcode) {
    case OPC_LOAD:
    _cpu_set(
      cpu,
      inst.rd,
      _mem_get_sz(cpu->mem, inst.immediate + _cpu_get(cpu, inst.rs1), inst.width)
    );
    break;

    case OPC_STORE:
    _mem_set_sz(
      cpu->mem,
      inst.immediate + _cpu_get(cpu, inst.rs1),
      _cpu_get(cpu, inst.rs2),
      inst.width
    );
    break;

    case OPC_IMM: {
      alu_op_t op = inst_alu_op(inst);
      word_t res = alu_execute(op, _cpu_get(cpu, inst.rs1), inst.immediate);
      _cpu_set(cpu, inst.rd, res);
      break;
    }

    case OPC_OP: {
      alu_op_t op = inst_alu_op(inst);
      word_t v1 = _cpu_get(cpu, inst.rs1);
      word_t v2 = _cpu_get(cpu, inst.rs2);
      word_t res = alu_execute(op, v1, v2);
      _cpu_set(cpu, inst.rd, res);
      break;
    }

    case OPC_AUIPC:
    _cpu_set(cpu, inst.rd, inst.immediate + cpu->pc);
    break;

    case OPC_LUI:
    _cpu_set(cpu, inst.rd, inst.immediate);
    break;

    case OPC_BRANCH: {
      alu_op_t op = inst_alu_op(inst);
      word_t v1 = _cpu_get(cpu, inst.rs1);
      word_t v2 = _cpu_get(cpu, inst.rs2);
      word_t res = alu_execute(op, v1, v2);
      if (res) next_pc = inst.immediate + cpu->pc;
      break;
    }

    case OPC_JALR: {
      addr_t addr = inst.immediate + _cpu_get(cpu, inst.rs1);
      addr &= ~1u;
      _cpu_set(cpu, inst.rd, next_pc);
      next_pc = addr;
      break;
    }

    case OPC_JAL:
    _cpu_set(cpu, inst.rd, next_pc);
    next_pc = inst.immediate + cpu->pc;
    break;

    default:
    debug_log("unknown opcode %08x", inst_word);
    assert(0);
  }
  cpu->pc = next_pc;
}
