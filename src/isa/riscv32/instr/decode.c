#include "../local-include/rtl.h"
#include <cpu/ifetch.h>
#include <cpu/decode.h>
#include <isa-all-instr.h>

def_all_THelper();

__attribute__((always_inline))
static inline uint32_t get_instr(Decode *s) {
  return s->isa.instr.val;
}

// decode operand helper
#define def_DopHelper(name) \
  void concat(decode_op_, name) (Decode *s, Operand *op, uint32_t val, bool flag)

static inline def_DopHelper(i) {
  op->imm = val;
  print_Dop(op->str, OP_STR_SIZE, (flag ? "0x%x" : "%d"), op->imm);
}

static inline def_DopHelper(r) {
  bool load_val = flag;
  static word_t zero_null = 0;
  op->preg = (!load_val && val == 0) ? &zero_null : &reg_l(val);
  print_Dop(op->str, OP_STR_SIZE, "%s", reg_name(val, 4));
}

static inline def_DHelper(I) {
  decode_op_r(s, id_src1, s->isa.instr.i.rs1, true);
  decode_op_i(s, id_src2, s->isa.instr.i.simm11_0, false);
  decode_op_r(s, id_dest, s->isa.instr.i.rd, false);
}

static inline def_DHelper(U) {
  decode_op_i(s, id_src1, s->isa.instr.u.imm31_12 << 12, true);
  decode_op_r(s, id_dest, s->isa.instr.u.rd, false);
}

static inline def_DHelper(S) {
  decode_op_r(s, id_src1, s->isa.instr.s.rs1, true);
  sword_t simm = (s->isa.instr.s.simm11_5 << 5) | s->isa.instr.s.imm4_0;
  decode_op_i(s, id_src2, simm, false);
  decode_op_r(s, id_dest, s->isa.instr.s.rs2, true);
}
#ifndef __ICS_EXPORT

static inline def_DHelper(R) {
  decode_op_r(s, id_src1, s->isa.instr.r.rs1, true);
  decode_op_r(s, id_src2, s->isa.instr.r.rs2, true);
  decode_op_r(s, id_dest, s->isa.instr.r.rd, false);
}

static inline def_DHelper(J) {
  sword_t offset = (s->isa.instr.j.simm20 << 20) | (s->isa.instr.j.imm19_12 << 12) |
    (s->isa.instr.j.imm11 << 11) | (s->isa.instr.j.imm10_1 << 1);
  decode_op_i(s, id_src1, s->pc + offset, true);
  decode_op_r(s, id_dest, s->isa.instr.j.rd, false);
  id_src2->imm = s->snpc;
}

static inline def_DHelper(B) {
  sword_t offset = (s->isa.instr.b.simm12 << 12) | (s->isa.instr.b.imm11 << 11) |
    (s->isa.instr.b.imm10_5 << 5) | (s->isa.instr.b.imm4_1 << 1);
  decode_op_i(s, id_dest, s->pc + offset, true);
  decode_op_r(s, id_src1, s->isa.instr.b.rs1, true);
  decode_op_r(s, id_src2, s->isa.instr.b.rs2, true);
}

static inline def_DHelper(auipc) {
  decode_U(s, width);
  id_src1->imm += s->pc;
}

static inline def_DHelper(csr) {
  decode_op_r(s, id_src1, s->isa.instr.i.rs1, true);
  decode_op_i(s, id_src2, s->isa.instr.csr.csr, true);
  decode_op_r(s, id_dest, s->isa.instr.i.rd, false);
}
#endif

def_THelper(load) {
  print_Dop(id_src1->str, OP_STR_SIZE, "%d(%s)", id_src2->imm, reg_name(s->isa.instr.i.rs1, 4));
  int mmu_mode = isa_mmu_state();
  if (mmu_mode == MMU_DIRECT) {
    def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??", lb);
    def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??", lh);
    def_INSTR_TAB("??????? ????? ????? 010 ????? ????? ??", lw);
    def_INSTR_TAB("??????? ????? ????? 100 ????? ????? ??", lbu);
    def_INSTR_TAB("??????? ????? ????? 101 ????? ????? ??", lhu);
  } else if (mmu_mode == MMU_TRANSLATE) {
    def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??", lb_mmu);
    def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??", lh_mmu);
    def_INSTR_TAB("??????? ????? ????? 010 ????? ????? ??", lw_mmu);
    def_INSTR_TAB("??????? ????? ????? 100 ????? ????? ??", lbu_mmu);
    def_INSTR_TAB("??????? ????? ????? 101 ????? ????? ??", lhu_mmu);
  } else { assert(0); }
  return EXEC_ID_inv;
}

def_THelper(store) {
  print_Dop(id_src1->str, OP_STR_SIZE, "%d(%s)", id_src2->imm, reg_name(s->isa.instr.i.rs1, 4));
  int mmu_mode = isa_mmu_state();
  if (mmu_mode == MMU_DIRECT) {
    def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??", sb);
    def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??", sh);
    def_INSTR_TAB("??????? ????? ????? 010 ????? ????? ??", sw);
  } else if (mmu_mode == MMU_TRANSLATE) {
    def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??", sb_mmu);
    def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??", sh_mmu);
    def_INSTR_TAB("??????? ????? ????? 010 ????? ????? ??", sw_mmu);
  } else { assert(0); }
  return EXEC_ID_inv;
}

def_THelper(c_addi_dispatch) {
  if (id_src2->imm == 1) return table_p_inc(s);
  if (id_src2->imm == -1u) return table_p_dec(s);
  return table_c_addi(s);
}

def_THelper(addi_dispatch) {
  def_INSTR_TAB("0000000 00000 00000 ??? ????? ????? ??", p_li_0);
  def_INSTR_TAB("0000000 00001 00000 ??? ????? ????? ??", p_li_1);
  def_INSTR_TAB("??????? ????? 00000 ??? ????? ????? ??", c_li);
  def_INSTR_TAB("0000000 00000 ????? ??? ????? ????? ??", c_mv);
  def_INSTR_TAB("??????? ????? ????? ??? ????? ????? ??", addi);
  return EXEC_ID_inv;
}

def_THelper(op_imm) {
  if (s->isa.instr.i.rd == s->isa.instr.i.rs1) {
    def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??", c_addi_dispatch);
    def_INSTR_TAB("??????? ????? ????? 111 ????? ????? ??", c_andi);
    def_INSTR_TAB("0000000 ????? ????? 001 ????? ????? ??", c_slli);
    def_INSTR_TAB("0100000 ????? ????? 101 ????? ????? ??", c_srai);
    def_INSTR_TAB("0000000 ????? ????? 101 ????? ????? ??", c_srli);
  }
  def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??", addi_dispatch);
  def_INSTR_TAB("??????? ????? ????? 010 ????? ????? ??", slti);
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??", sltui);
  def_INSTR_TAB("??????? ????? ????? 100 ????? ????? ??", xori);
  def_INSTR_TAB("??????? ????? ????? 110 ????? ????? ??", ori);
  def_INSTR_TAB("??????? ????? ????? 111 ????? ????? ??", andi);
  def_INSTR_TAB("0000000 ????? ????? 001 ????? ????? ??", slli);
  def_INSTR_TAB("0000000 ????? ????? 101 ????? ????? ??", srli);
  def_INSTR_TAB("0100000 ????? ????? 101 ????? ????? ??", srai);
  return EXEC_ID_inv;
};

def_THelper(op) {
  if (s->isa.instr.r.rd == s->isa.instr.r.rs1) {
    def_INSTR_TAB("0000000 ????? ????? 000 ????? ????? ??", c_add);
    def_INSTR_TAB("0100000 ????? ????? 000 ????? ????? ??", c_sub);
    def_INSTR_TAB("0000000 ????? ????? 100 ????? ????? ??", c_xor);
    def_INSTR_TAB("0000000 ????? ????? 110 ????? ????? ??", c_or);
    def_INSTR_TAB("0000000 ????? ????? 111 ????? ????? ??", c_and);
  }
  def_INSTR_TAB("0000000 ????? ????? 000 ????? ????? ??", add);
  def_INSTR_TAB("0000000 ????? ????? 001 ????? ????? ??", sll);
  def_INSTR_TAB("0000000 ????? ????? 010 ????? ????? ??", slt);
  def_INSTR_TAB("0000000 ????? ????? 011 ????? ????? ??", sltu);
  def_INSTR_TAB("0000000 ????? ????? 100 ????? ????? ??", xor);
  def_INSTR_TAB("0000000 ????? ????? 101 ????? ????? ??", srl);
  def_INSTR_TAB("0000000 ????? ????? 110 ????? ????? ??", or);
  def_INSTR_TAB("0000000 ????? ????? 111 ????? ????? ??", and);
  def_INSTR_TAB("0100000 ????? ????? 000 ????? ????? ??", sub);
  def_INSTR_TAB("0100000 ????? ????? 101 ????? ????? ??", sra);
  def_INSTR_TAB("0000001 ????? ????? 000 ????? ????? ??", mul);
  def_INSTR_TAB("0000001 ????? ????? 001 ????? ????? ??", mulh);
  def_INSTR_TAB("0000001 ????? ????? 010 ????? ????? ??", mulhsu);
  def_INSTR_TAB("0000001 ????? ????? 011 ????? ????? ??", mulhu);
  def_INSTR_TAB("0000001 ????? ????? 100 ????? ????? ??", div);
  def_INSTR_TAB("0000001 ????? ????? 101 ????? ????? ??", divu);
  def_INSTR_TAB("0000001 ????? ????? 110 ????? ????? ??", rem);
  def_INSTR_TAB("0000001 ????? ????? 111 ????? ????? ??", remu);
  return EXEC_ID_inv;
}

def_THelper(branch) {
  def_INSTR_TAB("??????? 00000 ????? 000 ????? ????? ??", c_beqz);
  def_INSTR_TAB("??????? 00000 ????? 001 ????? ????? ??", c_bnez);
  def_INSTR_TAB("??????? 00000 ????? 100 ????? ????? ??", p_bltz);
  def_INSTR_TAB("??????? 00000 ????? 101 ????? ????? ??", p_bgez);

  def_INSTR_TAB("??????? ????? 00000 100 ????? ????? ??", p_bgtz);
  def_INSTR_TAB("??????? ????? 00000 101 ????? ????? ??", p_blez);

  def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??", beq);
  def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??", bne);
  def_INSTR_TAB("??????? ????? ????? 100 ????? ????? ??", blt);
  def_INSTR_TAB("??????? ????? ????? 101 ????? ????? ??", bge);
  def_INSTR_TAB("??????? ????? ????? 110 ????? ????? ??", bltu);
  def_INSTR_TAB("??????? ????? ????? 111 ????? ????? ??", bgeu);
  return EXEC_ID_inv;
};

def_THelper(jal_dispatch) {
  def_INSTR_TAB("??????? ????? ????? ??? 00000 ????? ??", c_j);
  def_INSTR_TAB("??????? ????? ????? ??? 00001 ????? ??", c_jal);
  def_INSTR_TAB("??????? ????? ????? ??? ????? ????? ??", jal);
  return EXEC_ID_inv;
}

def_THelper(jalr_dispatch) {
  def_INSTR_TAB("0000000 00000 00001 ??? 00000 ????? ??", p_ret);
  def_INSTR_TAB("0000000 00000 ????? ??? 00000 ????? ??", c_jr);
  def_INSTR_TAB("??????? ????? ????? ??? ????? ????? ??", jalr);
  return EXEC_ID_inv;
}

def_THelper(system) {
  def_INSTR_TAB("000000000000 ????? 000 ????? ????? ??", ecall);
  def_INSTR_TAB("000100000010 ????? 000 ????? ????? ??", sret);
  def_INSTR_TAB("000100100000 ????? 000 ????? ????? ??", sfence_vma);
  def_INSTR_TAB("???????????? ????? 001 ????? ????? ??", csrrw);
  def_INSTR_TAB("???????????? ????? 010 ????? ????? ??", csrrs);
  return EXEC_ID_inv;
};

def_THelper(main) {
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00000 11", I     , load);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00100 11", I     , op_imm);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00101 11", auipc , auipc);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01000 11", S     , store);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01100 11", R     , op);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01101 11", U     , lui);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11000 11", B     , branch);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11001 11", I     , jalr_dispatch);
  def_INSTR_TAB  ("??????? ????? ????? ??? ????? 11010 11",         nemu_trap);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11011 11", J     , jal_dispatch);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11100 11", csr   , system);
  return table_inv(s);
};

int isa_fetch_decode(Decode *s) {
  s->isa.instr.val = instr_fetch(&s->snpc, 4);
  int idx = EXEC_ID_inv;
  if (s->isa.instr.i.opcode1_0 == 0x3) {
    idx = table_main(s);
  }

  s->type = INSTR_TYPE_N;
  switch (idx) {
    case EXEC_ID_c_j: case EXEC_ID_c_jal: case EXEC_ID_jal:
      s->jnpc = id_src1->imm; s->type = INSTR_TYPE_J; break;

    case EXEC_ID_beq: case EXEC_ID_bne: case EXEC_ID_blt: case EXEC_ID_bge:
    case EXEC_ID_bltu: case EXEC_ID_bgeu:
    case EXEC_ID_c_beqz: case EXEC_ID_c_bnez:
    case EXEC_ID_p_bltz: case EXEC_ID_p_bgez: case EXEC_ID_p_blez: case EXEC_ID_p_bgtz:
      s->jnpc = id_dest->imm; s->type = INSTR_TYPE_B; break;

    case EXEC_ID_p_ret: case EXEC_ID_c_jr: case EXEC_ID_jalr:
    case EXEC_ID_sret: case EXEC_ID_ecall:
      s->type = INSTR_TYPE_I;
  }

  return idx;
}
