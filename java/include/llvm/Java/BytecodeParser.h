//===-- BytecodeParser.h - Java bytecode parser ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains Java bytecode parser baseclass: a class that
// helps writing Java bytecode parsers.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_JAVA_BYTECODEPARSER_H
#define LLVM_JAVA_BYTECODEPARSER_H

#include <llvm/Java/Bytecode.h>

namespace llvm { namespace Java {

  /// @brief This class provides a base class that eases bytecode
  /// parsing.
  ///
  /// By default all the do_* methods do nothing. A subclass can
  /// override at will to provide specific behaviour.
  template <typename SubClass>
  class BytecodeParser {
  protected:
    typedef std::vector<std::pair<int, unsigned> > SwitchCases;

    enum JSetCC { EQ, NE, LT, GE, GT, LE };

    enum JType {
      REFERENCE = 0, // this is not defined in the java spec
      BOOLEAN = 4,
      CHAR = 5,
      FLOAT = 6,
      DOUBLE = 7,
      BYTE = 8,
      SHORT = 9,
      INT = 10,
      LONG = 11,
    };

  private:
    SwitchCases switchCases_;

  protected:
#define THIS ((SubClass*)this)

    /// @brief parse code pointed to by \c code of size \c size
    ///
    /// This function parses the code pointed to by \c code and
    /// calls the subclass's do_<bytecode> method
    /// appropriately. When this function returns all code up to
    /// \c size is parsed.
    void parse(const uint8_t* code, unsigned size) {
      using namespace Opcode;

      for (unsigned i = 0; i < size; ++i) {
        unsigned curBC = i;
        bool wide = code[i] == WIDE;
        i += wide;
        THIS->pre_inst(curBC);
        switch (code[i]) {
        case ACONST_NULL:
          THIS->do_aconst_null();
          break;
        case ICONST_M1:
        case ICONST_0:
        case ICONST_1:
        case ICONST_2:
        case ICONST_3:
        case ICONST_4:
        case ICONST_5:
          THIS->do_iconst(code[i]-ICONST_0);
          break;
        case LCONST_0:
        case LCONST_1:
          THIS->do_lconst(code[i]-LCONST_0);
          break;
        case FCONST_0:
        case FCONST_1:
        case FCONST_2:
          THIS->do_fconst(code[i]-FCONST_0);
          break;
        case DCONST_0:
        case DCONST_1:
          THIS->do_dconst(code[i]-DCONST_0);
          break;
        case BIPUSH:
          THIS->do_iconst(readSByte(code, i));
          break;
        case SIPUSH:
          THIS->do_iconst(readSShort(code, i));
          break;
        case LDC:
          THIS->do_ldc(readUByte(code, i));
          break;
        case LDC_W:
          THIS->do_ldc(readUShort(code, i));
          break;
        case LDC2_W:
          THIS->do_ldc(readUShort(code, i));
          break;
        case ILOAD:
          THIS->do_load(
            INT, wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case LLOAD:
          THIS->do_load(
            LONG, wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case FLOAD:
          THIS->do_load(
            FLOAT, wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case DLOAD:
          THIS->do_load(
            DOUBLE, wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ALOAD:
          THIS->do_load(
            REFERENCE, wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ILOAD_0:
        case ILOAD_1:
        case ILOAD_2:
        case ILOAD_3:
          THIS->do_load(INT, code[i]-ILOAD_0);
          break;
        case LLOAD_0:
        case LLOAD_1:
        case LLOAD_2:
        case LLOAD_3:
          THIS->do_load(LONG, code[i]-LLOAD_0);
          break;
        case FLOAD_0:
        case FLOAD_1:
        case FLOAD_2:
        case FLOAD_3:
          THIS->do_load(FLOAT, code[i]-FLOAD_0);
          break;
        case DLOAD_0:
        case DLOAD_1:
        case DLOAD_2:
        case DLOAD_3:
          THIS->do_load(DOUBLE, code[i]-DLOAD_0);
          break;
        case ALOAD_0:
        case ALOAD_1:
        case ALOAD_2:
        case ALOAD_3:
          THIS->do_load(REFERENCE, code[i]-ALOAD_0);
          break;
        case IALOAD:
          THIS->do_aload(INT);
          break;
        case LALOAD:
          THIS->do_aload(LONG);
          break;
        case FALOAD:
          THIS->do_aload(FLOAT);
          break;
        case DALOAD:
          THIS->do_aload(DOUBLE);
          break;
        case AALOAD:
          THIS->do_aload(REFERENCE);
          break;
        case BALOAD:
          THIS->do_aload(BYTE);
          break;
        case CALOAD:
          THIS->do_aload(CHAR);
          break;
        case SALOAD:
          THIS->do_aload(SHORT);
          break;
        case ISTORE:
          THIS->do_store(
            INT, wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case LSTORE:
          THIS->do_store(
            LONG, wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case FSTORE:
          THIS->do_store(
            FLOAT, wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case DSTORE:
          THIS->do_store(
            DOUBLE, wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ASTORE:
          THIS->do_store(
            REFERENCE, wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ISTORE_0:
        case ISTORE_1:
        case ISTORE_2:
        case ISTORE_3:
          THIS->do_store(INT, code[i]-ISTORE_0);
          break;
        case LSTORE_0:
        case LSTORE_1:
        case LSTORE_2:
        case LSTORE_3:
          THIS->do_store(LONG, code[i]-LSTORE_0);
          break;
        case FSTORE_0:
        case FSTORE_1:
        case FSTORE_2:
        case FSTORE_3:
          THIS->do_store(FLOAT, code[i]-FSTORE_0);
          break;
        case DSTORE_0:
        case DSTORE_1:
        case DSTORE_2:
        case DSTORE_3:
          THIS->do_store(DOUBLE, code[i]-DSTORE_0);
          break;
        case ASTORE_0:
        case ASTORE_1:
        case ASTORE_2:
        case ASTORE_3:
          THIS->do_store(REFERENCE, code[i]-ASTORE_0);
          break;
        case IASTORE:
          THIS->do_astore(INT);
          break;
        case LASTORE:
          THIS->do_astore(LONG);
          break;
        case FASTORE:
          THIS->do_astore(FLOAT);
          break;
        case DASTORE:
          THIS->do_astore(DOUBLE);
          break;
        case AASTORE:
          THIS->do_astore(REFERENCE);
          break;
        case BASTORE:
          THIS->do_astore(BYTE);
          break;
        case CASTORE:
          THIS->do_astore(CHAR);
          break;
        case SASTORE:
          THIS->do_astore(SHORT);
          break;
        case POP:
          THIS->do_pop();
          break;
        case POP2:
          THIS->do_pop2();
          break;
        case DUP:
          THIS->do_dup();
          break;
        case DUP_X1:
          THIS->do_dup_x1();
          break;
        case DUP_X2:
          THIS->do_dup_x2();
          break;
        case DUP2:
          THIS->do_dup2();
          break;
        case DUP2_X1:
          THIS->do_dup2_x1();
          break;
        case DUP2_X2:
          THIS->do_dup2_x2();
          break;
        case SWAP:
          THIS->do_swap();
          break;
        case IADD:
        case LADD:
        case FADD:
        case DADD:
          THIS->do_add();
          break;
        case ISUB:
        case LSUB:
        case FSUB:
        case DSUB:
          THIS->do_sub();
          break;
        case IMUL:
        case LMUL:
        case FMUL:
        case DMUL:
          THIS->do_mul();
          break;
        case IDIV:
        case LDIV:
        case FDIV:
        case DDIV:
          THIS->do_div();
          break;
        case IREM:
        case LREM:
        case FREM:
        case DREM:
          THIS->do_rem();
          break;
        case INEG:
        case LNEG:
        case FNEG:
        case DNEG:
          THIS->do_neg();
          break;
        case ISHL:
        case LSHL:
          THIS->do_shl();
          break;
        case ISHR:
        case LSHR:
          THIS->do_shr();
          break;
        case IUSHR:
        case LUSHR:
          THIS->do_ushr();
          break;
        case IAND:
        case LAND:
          THIS->do_and();
          break;
        case IOR:
        case LOR:
          THIS->do_or();
          break;
        case IXOR:
        case LXOR:
          THIS->do_xor();
          break;
        case IINC:
          THIS->do_iinc(readUByte(code, i), readSByte(code, i));
          break;
        case I2L:
          THIS->do_convert(LONG);
          break;
        case I2F:
          THIS->do_convert(FLOAT);
          break;
        case I2D:
          THIS->do_convert(DOUBLE);
          break;
        case L2I:
          THIS->do_convert(INT);
          break;
        case L2F:
          THIS->do_convert(FLOAT);
          break;
        case L2D:
          THIS->do_convert(DOUBLE);
          break;
        case F2I:
          THIS->do_convert(INT);
          break;
        case F2L:
          THIS->do_convert(LONG);
          break;
        case F2D:
          THIS->do_convert(DOUBLE);
          break;
        case D2I:
          THIS->do_convert(INT);
          break;
        case D2L:
          THIS->do_convert(LONG);
          break;
        case D2F:
          THIS->do_convert(FLOAT);
          break;
        case I2B:
          THIS->do_convert(BYTE);
          break;
        case I2C:
          THIS->do_convert(CHAR);
          break;
        case I2S:
          THIS->do_convert(SHORT);
          break;
        case LCMP:
          THIS->do_lcmp();
          break;
        case FCMPL:
          THIS->do_cmpl();
          break;
        case FCMPG:
          THIS->do_cmpg();
          break;
        case DCMPL:
          THIS->do_cmpl();
          break;
        case DCMPG:
          THIS->do_cmpg();
          break;
        case IFEQ: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(EQ, INT, t, i + 1);
          break;
        }
        case IFNE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(NE, INT, t, i + 1);
          break;
        }
        case IFLT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(LT, INT, t, i + 1);
          break;
        }
        case IFGE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(GE, INT, t, i + 1);
          break;
        }
        case IFGT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(GT, INT, t, i + 1);
          break;
        }
        case IFLE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(LE, INT, t, i + 1);
          break;
        }
        case IF_ICMPEQ: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(EQ, t, i + 1);
          break;
        }
        case IF_ICMPNE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(NE, t, i + 1);
          break;
        }
        case IF_ICMPLT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(LT, t, i + 1);
          break;
        }
        case IF_ICMPGE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(GE, t, i + 1);
          break;
        }
        case IF_ICMPGT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(GT, t, i + 1);
          break;
        }
        case IF_ICMPLE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(LE, t, i + 1);
          break;
        }
        case IF_IACMPEQ: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(EQ, t, i + 1);
          break;
        }
        case IF_IACMPNE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(NE, t, i + 1);
          break;
        }
        case GOTO:
          THIS->do_goto(curBC + readSShort(code, i));
          break;
        case JSR:
          THIS->do_jsr(curBC + readSShort(code, i));
          break;
        case RET:
          THIS->do_ret(readUByte(code, i));
          break;
        case TABLESWITCH: {
          switchCases_.clear();
          skipPadBytes(i);
          int def = readSInt(code, i);
          int low = readSInt(code, i);
          int high = readSInt(code, i);
          switchCases_.reserve(high - low + 1);
          while (low <= high)
            switchCases_.push_back(
              std::make_pair(low++, curBC + readSInt(code, i)));
          THIS->do_switch(curBC + def, switchCases_);
          break;
        }
        case LOOKUPSWITCH: {
          switchCases_.clear();
          skipPadBytes(i);
          int def = readSInt(code, i);
          int pairCount = readSInt(code, i);
          switchCases_.reserve(pairCount);
          while (pairCount--) {
            int value = readSInt(code, i);
            switchCases_.push_back(
              std::make_pair(value, curBC + readSInt(code, i)));
          }
          THIS->do_switch(curBC + def, switchCases_);
          break;
        }
        case IRETURN:
        case LRETURN:
        case FRETURN:
        case DRETURN:
        case ARETURN:
          THIS->do_return();
          break;
        case RETURN:
          THIS->do_return_void();
          break;
        case GETSTATIC:
          THIS->do_getstatic(readUShort(code, i));
          break;
        case PUTSTATIC:
          THIS->do_putstatic(readUShort(code, i));
          break;
        case GETFIELD:
          THIS->do_getfield(readUShort(code, i));
          break;
        case PUTFIELD:
          THIS->do_putfield(readUShort(code, i));
          break;
        case INVOKEVIRTUAL:
          THIS->do_invokevirtual(readUShort(code, i));
          break;
        case INVOKESPECIAL:
          THIS->do_invokespecial(readUShort(code, i));
          break;
        case INVOKESTATIC:
          THIS->do_invokestatic(readUShort(code, i));
          break;
        case INVOKEINTERFACE: {
          THIS->do_invokeinterface(readUShort(code, i));
          unsigned count = readUByte(code, i);
          unsigned zero = readUByte(code, i);
          break;
        }
        case XXXUNUSEDXXX:
          // FIXME: must throw something
          break;
        case NEW:
          THIS->do_new(readUShort(code, i));
          break;
        case NEWARRAY:
          THIS->do_newarray(static_cast<JType>(readUByte(code, i)));
          break;
        case ANEWARRAY:
          THIS->do_anewarray(readUShort(code, i));
          break;
        case ARRAYLENGTH:
          THIS->do_arraylength();
          break;
        case ATHROW:
          THIS->do_athrow();
          break;
        case CHECKCAST:
          THIS->do_checkcast(readUShort(code, i));
          break;
        case INSTANCEOF:
          THIS->do_instanceof(readUShort(code, i));
          break;
        case MONITORENTER:
          THIS->do_monitorenter();
          break;
        case MONITOREXIT:
          THIS->do_monitorexit();
          break;
        case WIDE:
          // FIXME: must throw something
          break;
        case MULTIANEWARRAY:
          THIS->do_multianewarray(readUShort(code, i), readUByte(code, i));
          break;
        case IFNULL: {
          unsigned t = curBC + readUShort(code, i);
          THIS->do_if(EQ, REFERENCE, t, i + 1);
          break;
        }
        case IFNONNULL: {
          unsigned t = curBC + readUShort(code, i);
          THIS->do_if(NE, REFERENCE, t, i + 1);
          break;
        }
        case GOTO_W:
          THIS->do_goto(curBC + readSInt(code, i));
          break;
        case JSR_W:
          THIS->do_jsr(curBC + readSInt(code, i));
          break;
        case BREAKPOINT:
        case IMPDEP1:
        case IMPDEP2:
        case NOP:
          break;
        default:
          // FIXME: must throw something
          break;
        }
      }
    }

#undef THIS

    /// @brief called before every bytecode
    void pre_inst(unsigned bcI) { }
    /// @brief called on ACONST_NULL
    void do_aconst_null() { }
    /// @brief called on ICONST_<n>, SIPUSH and BIPUSH
    void do_iconst(int value) { }
    /// @brief called on LCONST_<n>
    void do_lconst(long long value) { }
    /// @brief called on FCONST_<n>
    void do_fconst(float value) { }
    /// @brief called on DCONST_<n>
    void do_dconst(double value) { }
    /// @brief called on LDC, LDC_W and LDC2_W
    void do_ldc(unsigned index) { }
    /// @brief called on ILOAD, LLOAD, FLOAD, DLOAD, ALOAD,
    /// ILOAD_<n>, LLOAD_<n>, FLOAD_<n>, DLOAD_<n>, and ALOAD_<n>
    void do_load(JType type, unsigned index) { }
    /// @brief called on IALOAD, LALOAD, FALOAD, DALOAD, AALOAD,
    /// BALOAD, CALOAD, and SALOAD
    void do_aload(JType type) { }
    /// @brief called on ISTORE, LSTORE, FSTORE, DSTORE, ASTORE,
    /// ISTORE_<n>, LSTORE_<n>, FSTORE_<n>, DSTORE_<n>, and
    /// ASTORE_<n>
    void do_store(JType type, unsigned index) { }
    /// @brief called on IASTORE, LASTORE, FASTORE, DASTORE, AASTORE,
    /// BASTORE, CASTORE, and SASTORE
    void do_astore(JType type) { }
    /// @brief called on POP
    void do_pop() { }
    /// @brief called on POP2
    void do_pop2() { }
    /// @brief called on DUP
    void do_dup() { }
    /// @brief called on DUP_X1
    void do_dup_x1() { }
    /// @brief called on DUP_X2
    void do_dup_x2() { }
    /// @brief called on DUP2
    void do_dup2() { }
    /// @brief called on DUP2_X1
    void do_dup2_x1() { }
    /// @brief called on DUP2_X2
    void do_dup2_x2() { }
    /// @brief called on SWAP
    void do_swap() { }
    /// @brief called on IADD, LADD, FADD, and DADD
    void do_add() { }
    /// @brief called on ISUB, LSUB, FSUB, and DSUB
    void do_sub() { }
    /// @brief called on IMUL, LMUL, FMUL, and DMUL
    void do_mul() { }
    /// @brief called on IDIV, LDIV, FDIV, and DDIV
    void do_div() { }
    /// @brief called on IREM, LREM, FREM, and DREM
    void do_rem() { }
    /// @brief called on INEG, LNEG, FNEG, and DNEG
    void do_neg() { }
    /// @brief called on ISHL and LSHL
    void do_shl() { }
    /// @brief called on ISHR and LSHR
    void do_shr() { }
    /// @brief called on IUSHR and LUSHR
    void do_ushr() { }
    /// @brief called on IAND and LAND
    void do_and() { }
    /// @brief called on IOR or LOR
    void do_or() { }
    /// @brief called on IXOR and LXOR
    void do_xor() { }
    /// @brief called on IINC
    void do_iinc(unsigned index, int amount) { }
    /// @brief called on I2L, I2F, I2D, L2I, L2F, L2D, F2I, F2L,
    /// F2D, D2I, D2L, D2F, I2B, I2C, and I2S
    void do_convert(JType to) { }
    /// @brief called on LCMP
    void do_lcmp() { }
    /// @brief called on FCMPL and DCMPL
    void do_cmpl() { }
    /// @brief called on FCMPG and DCMPG
    void do_cmpg() { }
    /// @brief called on IF<op>, IFNULL, and IFNONNULL
    void do_if(JSetCC cc, JType type, unsigned t, unsigned f) { }
    /// @brief called on IFCMP<op> and IFACMP<op>
    void do_ifcmp(JSetCC cc, unsigned t, unsigned f) { }
    /// @brief called on GOTO and GOTO_W
    void do_goto(unsigned target) { }
    /// @brief called on JSR and JSR_W
    void do_jsr(unsigned target) { }
    /// @brief called on RET
    void do_ret(unsigned index) { }
    /// @brief called on TABLESWITCH and LOOKUPSWITCH
    void do_switch(unsigned defTarget, const SwitchCases& sw) { }
    /// @brief called on IRETURN, LRETURN, FRETURN, DRETURN and
    /// ARETURN
    void do_return() { }
    /// @brief called on RETURN
    void do_return_void() { }
    /// @brief called on GETSTATIC
    void do_getstatic(unsigned index) { }
    /// @brief called on PUTSTATIC
    void do_putstatic(unsigned index) { }
    /// @brief called on GETFIELD
    void do_getfield(unsigned index) { }
    /// @brief called on PUTFIELD
    void do_putfield(unsigned index) { }
    /// @brief called on INVOKEVIRTUAL
    void do_invokevirtual(unsigned index) { }
    /// @brief called on INVOKESPECIAL
    void do_invokespecial(unsigned index) { }
    /// @brief called on INVOKESTATIC
    void do_invokestatic(unsigned index) { }
    /// @brief called on INVOKEINTERFACE
    void do_invokeinterface(unsigned index) { }
    /// @brief called on NEW
    void do_new(unsigned index) { }
    /// @brief called on NEWARRAY
    void do_newarray(JType type) { }
    /// @brief called on ANEWARRAY
    void do_anewarray(unsigned index) { }
    /// @brief called on ARRAYLENGTH
    void do_arraylength() { }
    /// @brief called on ATHROW
    void do_athrow() { }
    /// @brief called on CHECKCAST
    void do_checkcast(unsigned index) { }
    /// @brief called on INSTANCEOF
    void do_instanceof(unsigned index) { }
    /// @brief called on MONITORENTER
    void do_monitorenter() { }
    /// @brief called on MONITOREXIT
    void do_monitorexit() { }
    /// @brief called on MULTIANEWARRAY
    void do_multianewarray(unsigned index, unsigned dims) { }
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_BYTECODEPARSER_H
