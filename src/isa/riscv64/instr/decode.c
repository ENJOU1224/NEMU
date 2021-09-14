#include "../local-include/reg.h"
#include <cpu/ifetch.h>
#include <isa-all-instr.h>

def_all_THelper();

static uint32_t get_instr(Decode *s) {
  return s->isa.instr.val;
}

// decode operand helper
#define def_DopHelper(name) \
  void concat(decode_op_, name) (Decode *s, Operand *op, word_t val, bool flag)

#ifndef __ICS_EXPORT
#include "rvi/decode.h"
#include "rvf/decode.h"
#include "rvm/decode.h"
#include "rva/decode.h"
#include "rvc/decode.h"
#include "rvd/decode.h"
#include "priv/decode.h"

def_THelper(main) {
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00000 ??", I     , load);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00001 ??", fload , fload);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00011 ??", I     , mem_fence);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00100 ??", I     , op_imm);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00101 ??", auipc , auipc);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00110 ??", I     , op_imm32);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01000 ??", S     , store);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01001 ??", fstore, fstore);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01011 ??", R     , atomic);
  def_INSTR_IDTAB("0000001 ????? ????? ??? ????? 01100 ??", R     , rvm);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01100 ??", R     , op);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01101 ??", U     , lui);
  def_INSTR_IDTAB("0000001 ????? ????? ??? ????? 01110 ??", R     , rvm32);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01110 ??", R     , op32);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 10000 ??", R4    , fmadd_dispatch);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 10001 ??", R4    , fmadd_dispatch);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 10010 ??", R4    , fmadd_dispatch);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 10011 ??", R4    , fmadd_dispatch);
  def_INSTR_TAB  ("??????? ????? ????? ??? ????? 10100 ??",         op_fp);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11000 ??", B     , branch);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11001 ??", I     , jalr_dispatch);
  def_INSTR_TAB  ("??????? ????? ????? ??? ????? 11010 ??",         nemu_trap);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11011 ??", J     , jal_dispatch);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11100 ??", csr   , system);
  return table_inv(s);
};

int isa_fetch_decode(Decode *s) {
  int idx = EXEC_ID_inv;

  s->isa.instr.val = instr_fetch(&s->snpc, 2);
  if (s->isa.instr.r.opcode1_0 != 0x3) {
    // this is an RVC instruction
    idx = table_rvc(s);
  } else {
    // this is a 4-byte instruction, should fetch the MSB part
    // NOTE: The fetch here may cause IPF.
    // If it is the case, we should have mepc = xxxffe and mtval = yyy000.
    // Refer to `mtval` in the privileged manual for more details.
    uint32_t hi = instr_fetch(&s->snpc, 2);
    s->isa.instr.val |= (hi << 16);
    idx = table_main(s);
  }

  s->type = INSTR_TYPE_N;
  switch (idx) {
    case EXEC_ID_c_j: case EXEC_ID_p_jal: case EXEC_ID_jal:
      s->jnpc = id_src1->imm; s->type = INSTR_TYPE_J; break;

    case EXEC_ID_beq: case EXEC_ID_bne: case EXEC_ID_blt: case EXEC_ID_bge:
    case EXEC_ID_bltu: case EXEC_ID_bgeu:
    case EXEC_ID_c_beqz: case EXEC_ID_c_bnez:
    case EXEC_ID_p_bltz: case EXEC_ID_p_bgez: case EXEC_ID_p_blez: case EXEC_ID_p_bgtz:
      s->jnpc = id_dest->imm; s->type = INSTR_TYPE_B; break;

    case EXEC_ID_p_ret: case EXEC_ID_c_jr: case EXEC_ID_c_jalr: case EXEC_ID_jalr:
    IFDEF(CONFIG_ITRACE, case EXEC_ID_mret: case EXEC_ID_sret: case EXEC_ID_ecall:)
      s->type = INSTR_TYPE_I; break;

#ifndef CONFIG_ITRACE
    case EXEC_ID_system:
      if (s->isa.instr.i.funct3 == 0) {
        switch (s->isa.instr.csr.csr) {
          case 0:     // ecall
          case 0x102: // sret
          case 0x302: // mret
            s->type = INSTR_TYPE_I;
        }
      }
      break;
#endif
  }

  return idx;
}
#else
static inline def_DopHelper(i) {
  op->imm = val;
  print_Dop(op->str, OP_STR_SIZE, (flag ? "0x%lx" : "%ld"), op->imm);
}

static inline def_DopHelper(r) {
  bool is_write = flag;
  static word_t zero_null = 0;
  op->preg = (is_write && val == 0) ? &zero_null : &gpr(val);
  print_Dop(op->str, OP_STR_SIZE, "%s", reg_name(val, 4));
}

static inline def_DHelper(I) {
  decode_op_r(s, id_src1, s->isa.instr.i.rs1, false);
  decode_op_i(s, id_src2, (sword_t)s->isa.instr.i.simm11_0, false);
  decode_op_r(s, id_dest, s->isa.instr.i.rd, true);
}

static inline def_DHelper(U) {
  decode_op_i(s, id_src1, (sword_t)s->isa.instr.u.simm31_12 << 12, true);
  decode_op_r(s, id_dest, s->isa.instr.u.rd, true);
}

static inline def_DHelper(S) {
  decode_op_r(s, id_src1, s->isa.instr.s.rs1, false);
  sword_t simm = (s->isa.instr.s.simm11_5 << 5) | s->isa.instr.s.imm4_0;
  decode_op_i(s, id_src2, simm, false);
  decode_op_r(s, id_dest, s->isa.instr.s.rs2, false);
}

def_THelper(load) {
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??", ld);
  return EXEC_ID_inv;
}

def_THelper(store) {
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??", sd);
  return EXEC_ID_inv;
}

def_THelper(main) {
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00000 11", I     , load);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01000 11", S     , store);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00101 11", U     , auipc);
  def_INSTR_TAB  ("??????? ????? ????? ??? ????? 11010 11",         nemu_trap);
  return table_inv(s);
};

int isa_fetch_decode(Decode *s) {
  s->isa.instr.val = instr_fetch(&s->snpc, 4);
  int idx = table_main(s);
  return idx;
}
#endif
