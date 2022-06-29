#include <assert.h>
#include <stdbool.h>

#include "branch-predictor.h"
#include "inst-dispatch.h"
#include "bus.h"
#include "common-data-bus.h"
#include "inst-unit.h"
#include "inst.h"
#include "lib/log.h"
#include "reg-store.h"
#include "reg.h"
#include "reorder-buffer.h"
#include "reservation-station.h"
#include "rv32i.h"
#include "rvmath.h"

void _inst_reg_set (inst_unit_t *unit, reg_id_t id,
                    rob_id_t *src, word_t *value);

addr_t inst_dispatch (inst_unit_t *unit,
                      reservation_station_t *rs,
                      reorder_buffer_t *rob,
                      inst_t inst,
                      addr_t pc) {
  debug_log("writing op %d to ROB #%02d, RS #%2d", inst.op,
            rob->id, rs->id);
  addr_t next_pc = pc + INST_SIZE;

  rm_write(rs->busy, bool) = true;
  rm_write(rob->ready, bool) = false;
  rob_payload_t *rob_data =
    &rm_write(rob->payload, rob_payload_t);
  rob_data->op = inst_rob_op(inst);
  rob_data->addr_ready = rob_data->value_ready = false;
  rob_data->rs = rs->id;
  rob_data->size = inst_ls_op(inst);
  rob_data->pc = pc;

  rs_payload_t *data = &rm_write(rs->payload, rs_payload_t);
  rob_id_t rob_id = rob->id;
  data->dest = rob_id;
  data->op_alu = inst_alu_op(inst);
  data->op_ls = inst_ls_op(inst);

  if (inst.format != IF_S && inst.format != IF_B) {
    rob_data->dest = inst.rd;
    reg_mut_t *reg =
      reg_store_rob_id(unit->reg_store, inst.rd);
    rm_write(reg, rob_id_t) = rob->id;
  } else {
    rob_data->dest = -1;
  }

  switch (inst.format) {
    case IF_I:
    if (inst.opcode == OPC_LOAD) {
      _inst_reg_set(unit, inst.rs1, &data->src1, &data->value1);
      data->addr = inst.immediate;
    } else if (inst.opcode == OPC_JALR) {
      addr_t curr = reg_store_get(unit->reg_store, inst.rs1);
      addr_t prediction = signed_add(curr, inst.immediate);
      prediction &= ~1u;
      rob_data->predicted_addr = prediction;
      rob_data->value = next_pc;
      next_pc = prediction;

      // addi rob, rs1, imm
      data->op_alu = ALU_ADD;
      _inst_reg_set(unit, inst.rs1, &data->src1, &data->value1);
      data->src2 = 0;
      data->value2 = inst.immediate;
    } else {
      assert(inst.opcode == OPC_IMM || inst.op == INST_HCF);
      data->value1 = inst.immediate;
      data->src1 = 0;
      _inst_reg_set(unit, inst.rs1, &data->src2, &data->value2);
    }
    break;

    case IF_U:
    // TODO(perf)
    // treat U-type as addi rd, x0, imm
    data->op_alu = ALU_ADD;
    data->src1 = data->src2 = 0;
    data->value2 = 0;
    switch (inst.op) {
      case OPC_LUI: data->value1 = inst.immediate; break;
      case OPC_AUIPC:
      data->value1 = signed_add(pc, inst.immediate);
      break;

      default:
      debug_log("unknown U-type inst %d", inst.op);
      assert(0);
    }
    break;

    case IF_R:
    _inst_reg_set(unit, inst.rs1, &data->src1, &data->value1);
    _inst_reg_set(unit, inst.rs2, &data->src2, &data->value2);
    break;

    case IF_J:
    assert(inst.op == INST_JAL);
    // addi rd, x0, pc + 4
    data->op_alu = ALU_ADD;
    data->src1 = data->src2 = 0;
    data->value1 = next_pc;
    data->value2 = 0;
    next_pc = signed_add(pc, inst.immediate);
    break;

    case IF_B:
    _inst_reg_set(unit, inst.rs1, &data->src1, &data->value1);
    _inst_reg_set(unit, inst.rs2, &data->src2, &data->value2);
    addr_t jump_target = signed_add(pc, inst.immediate);
    bool predicted = branch_predict(unit->branch_predictor);
    rob_data->predicted_take = predicted;
    rob_data->fallback = predicted ? jump_target : next_pc;
    if (predicted) next_pc = jump_target;
    break;

    case IF_S:
    data->addr = inst.immediate;
    _inst_reg_set(unit, inst.rs1, &data->src1, &data->value1);
    _inst_reg_set(unit, inst.rs2, &data->src2, &data->value2);
    break;

    default:
    debug_log("unknown inst format %d", inst.format);
    assert(0);
  }

  return next_pc;
}

void _inst_reg_set (inst_unit_t *unit, reg_id_t id,
                    rob_id_t *src, word_t *value) {
  // x0 is always 0
  if (id == 0) {
    *src = 0;
    *value = 0;
    return;
  }
  reg_mut_t *reg = reg_store_rob_id(unit->reg_store, id);
  rob_id_t rob = *rm_read(reg, rob_id_t);
  if (rob == 0) {
    *src = 0;
    *value = reg_store_get(unit->reg_store, id);
  } else {
    cdb_message_t *cdb =
      (cdb_message_t *) bus_get_data(unit->cdb);
    if (cdb && cdb->rob == rob) {
      *src = 0;
      *value = cdb->result;
    } else {
      *src = rob;
      *value = 0;
    }
  }
}
