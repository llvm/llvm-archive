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
        switch (code[i]) {
        case ACONST_NULL:
          THIS->do_aconst_null(curBC);
          break;
        case ICONST_M1:
        case ICONST_0:
        case ICONST_1:
        case ICONST_2:
        case ICONST_3:
        case ICONST_4:
        case ICONST_5:
          THIS->do_iconst(curBC, code[i]-ICONST_0);
          break;
        case LCONST_0:
        case LCONST_1:
          THIS->do_lconst(curBC, code[i]-LCONST_0);
          break;
        case FCONST_0:
        case FCONST_1:
        case FCONST_2:
          THIS->do_fconst(curBC, code[i]-FCONST_0);
          break;
        case DCONST_0:
        case DCONST_1:
          THIS->do_dconst(curBC, code[i]-DCONST_0);
          break;
        case BIPUSH:
          THIS->do_iconst(curBC, readSByte(code, i));
          break;
        case SIPUSH:
          THIS->do_iconst(curBC, readSShort(code, i));
          break;
        case LDC:
          THIS->do_ldc(curBC, readUByte(code, i));
          break;
        case LDC_W:
          THIS->do_ldc(curBC, readUShort(code, i));
          break;
        case LDC2_W:
          THIS->do_ldc(curBC, readUShort(code, i));
          break;
        case ILOAD:
          THIS->do_load(
            curBC, INT,
            wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case LLOAD:
          THIS->do_load(
            curBC, LONG,
            wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case FLOAD:
          THIS->do_load(
            curBC, FLOAT,
            wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case DLOAD:
          THIS->do_load(
            curBC, DOUBLE,
            wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ALOAD:
          THIS->do_load(
            curBC, REFERENCE,
            wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ILOAD_0:
        case ILOAD_1:
        case ILOAD_2:
        case ILOAD_3:
          THIS->do_load(curBC, INT, code[i]-ILOAD_0);
          break;
        case LLOAD_0:
        case LLOAD_1:
        case LLOAD_2:
        case LLOAD_3:
          THIS->do_load(curBC, LONG, code[i]-LLOAD_0);
          break;
        case FLOAD_0:
        case FLOAD_1:
        case FLOAD_2:
        case FLOAD_3:
          THIS->do_load(curBC, FLOAT, code[i]-FLOAD_0);
          break;
        case DLOAD_0:
        case DLOAD_1:
        case DLOAD_2:
        case DLOAD_3:
          THIS->do_load(curBC, DOUBLE, code[i]-DLOAD_0);
          break;
        case ALOAD_0:
        case ALOAD_1:
        case ALOAD_2:
        case ALOAD_3:
          THIS->do_load(curBC, REFERENCE, code[i]-ALOAD_0);
          break;
        case IALOAD:
          THIS->do_aload(curBC, INT);
          break;
        case LALOAD:
          THIS->do_aload(curBC, LONG);
          break;
        case FALOAD:
          THIS->do_aload(curBC, FLOAT);
          break;
        case DALOAD:
          THIS->do_aload(curBC, DOUBLE);
          break;
        case AALOAD:
          THIS->do_aload(curBC, REFERENCE);
          break;
        case BALOAD:
          THIS->do_aload(curBC, BYTE);
          break;
        case CALOAD:
          THIS->do_aload(curBC, CHAR);
          break;
        case SALOAD:
          THIS->do_aload(curBC, SHORT);
          break;
        case ISTORE:
          THIS->do_store(
            curBC, INT,
            wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case LSTORE:
          THIS->do_store(
            curBC, LONG,
            wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case FSTORE:
          THIS->do_store(
            curBC, FLOAT,
            wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case DSTORE:
          THIS->do_store(
            curBC, DOUBLE,
            wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ASTORE:
          THIS->do_store(
            curBC, REFERENCE,
            wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ISTORE_0:
        case ISTORE_1:
        case ISTORE_2:
        case ISTORE_3:
          THIS->do_store(curBC, INT, code[i]-ISTORE_0);
          break;
        case LSTORE_0:
        case LSTORE_1:
        case LSTORE_2:
        case LSTORE_3:
          THIS->do_store(curBC, LONG, code[i]-LSTORE_0);
          break;
        case FSTORE_0:
        case FSTORE_1:
        case FSTORE_2:
        case FSTORE_3:
          THIS->do_store(curBC, FLOAT, code[i]-FSTORE_0);
          break;
        case DSTORE_0:
        case DSTORE_1:
        case DSTORE_2:
        case DSTORE_3:
          THIS->do_store(curBC, DOUBLE, code[i]-DSTORE_0);
          break;
        case ASTORE_0:
        case ASTORE_1:
        case ASTORE_2:
        case ASTORE_3:
          THIS->do_store(curBC, REFERENCE, code[i]-ASTORE_0);
          break;
        case IASTORE:
          THIS->do_astore(curBC, INT);
          break;
        case LASTORE:
          THIS->do_astore(curBC, LONG);
          break;
        case FASTORE:
          THIS->do_astore(curBC, FLOAT);
          break;
        case DASTORE:
          THIS->do_astore(curBC, DOUBLE);
          break;
        case AASTORE:
          THIS->do_astore(curBC, REFERENCE);
          break;
        case BASTORE:
          THIS->do_astore(curBC, BYTE);
          break;
        case CASTORE:
          THIS->do_astore(curBC, CHAR);
          break;
        case SASTORE:
          THIS->do_astore(curBC, SHORT);
          break;
        case POP:
          THIS->do_pop(curBC);
          break;
        case POP2:
          THIS->do_pop2(curBC);
          break;
        case DUP:
          THIS->do_dup(curBC);
          break;
        case DUP_X1:
          THIS->do_dup_x1(curBC);
          break;
        case DUP_X2:
          THIS->do_dup_x2(curBC);
          break;
        case DUP2:
          THIS->do_dup2(curBC);
          break;
        case DUP2_X1:
          THIS->do_dup2_x1(curBC);
          break;
        case DUP2_X2:
          THIS->do_dup2_x2(curBC);
          break;
        case SWAP:
          THIS->do_swap(curBC);
          break;
        case IADD:
        case LADD:
        case FADD:
        case DADD:
          THIS->do_add(curBC);
          break;
        case ISUB:
        case LSUB:
        case FSUB:
        case DSUB:
          THIS->do_sub(curBC);
          break;
        case IMUL:
        case LMUL:
        case FMUL:
        case DMUL:
          THIS->do_mul(curBC);
          break;
        case IDIV:
        case LDIV:
        case FDIV:
        case DDIV:
          THIS->do_div(curBC);
          break;
        case IREM:
        case LREM:
        case FREM:
        case DREM:
          THIS->do_rem(curBC);
          break;
        case INEG:
        case LNEG:
        case FNEG:
        case DNEG:
          THIS->do_neg(curBC);
          break;
        case ISHL:
        case LSHL:
          THIS->do_shl(curBC);
          break;
        case ISHR:
        case LSHR:
          THIS->do_shr(curBC);
          break;
        case IUSHR:
        case LUSHR:
          THIS->do_ushr(curBC);
          break;
        case IAND:
        case LAND:
          THIS->do_and(curBC);
          break;
        case IOR:
        case LOR:
          THIS->do_or(curBC);
          break;
        case IXOR:
        case LXOR:
          THIS->do_xor(curBC);
          break;
        case IINC:
          THIS->do_iinc(
            curBC, readUByte(code, i), readSByte(code, i));
          break;
        case I2L:
          THIS->do_convert(curBC, LONG);
          break;
        case I2F:
          THIS->do_convert(curBC, FLOAT);
          break;
        case I2D:
          THIS->do_convert(curBC, DOUBLE);
          break;
        case L2I:
          THIS->do_convert(curBC, INT);
          break;
        case L2F:
          THIS->do_convert(curBC, FLOAT);
          break;
        case L2D:
          THIS->do_convert(curBC, DOUBLE);
          break;
        case F2I:
          THIS->do_convert(curBC, INT);
          break;
        case F2L:
          THIS->do_convert(curBC, LONG);
          break;
        case F2D:
          THIS->do_convert(curBC, DOUBLE);
          break;
        case D2I:
          THIS->do_convert(curBC, INT);
          break;
        case D2L:
          THIS->do_convert(curBC, LONG);
          break;
        case D2F:
          THIS->do_convert(curBC, FLOAT);
          break;
        case I2B:
          THIS->do_convert(curBC, BYTE);
          break;
        case I2C:
          THIS->do_convert(curBC, CHAR);
          break;
        case I2S:
          THIS->do_convert(curBC, SHORT);
          break;
        case LCMP:
          THIS->do_lcmp(curBC);
          break;
        case FCMPL:
          THIS->do_cmpl(curBC);
          break;
        case FCMPG:
          THIS->do_cmpg(curBC);
          break;
        case DCMPL:
          THIS->do_cmpl(curBC);
          break;
        case DCMPG:
          THIS->do_cmpg(curBC);
          break;
        case IFEQ: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(curBC, EQ, INT, t, i + 1);
          break;
        }
        case IFNE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(curBC, NE, INT, t, i + 1);
          break;
        }
        case IFLT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(curBC, LT, INT, t, i + 1);
          break;
        }
        case IFGE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(curBC, GE, INT, t, i + 1);
          break;
        }
        case IFGT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(curBC, GT, INT, t, i + 1);
          break;
        }
        case IFLE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if(curBC, LE, INT, t, i + 1);
          break;
        }
        case IF_ICMPEQ: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(curBC, EQ, t, i + 1);
          break;
        }
        case IF_ICMPNE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(curBC, NE, t, i + 1);
          break;
        }
        case IF_ICMPLT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(curBC, LT, t, i + 1);
          break;
        }
        case IF_ICMPGE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(curBC, GE, t, i + 1);
          break;
        }
        case IF_ICMPGT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(curBC, GT, t, i + 1);
          break;
        }
        case IF_ICMPLE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(curBC, LE, t, i + 1);
          break;
        }
        case IF_IACMPEQ: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(curBC, EQ, t, i + 1);
          break;
        }
        case IF_IACMPNE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifcmp(curBC, NE, t, i + 1);
          break;
        }
        case GOTO:
          THIS->do_goto(curBC, curBC + readSShort(code, i));
          break;
        case JSR:
          THIS->do_jsr(curBC, curBC + readSShort(code, i));
          break;
        case RET:
          THIS->do_ret(curBC, readUByte(code, i));
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
          THIS->do_switch(curBC, curBC + def, switchCases_);
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
          THIS->do_switch(curBC, curBC + def, switchCases_);
          break;
        }
        case IRETURN:
        case LRETURN:
        case FRETURN:
        case DRETURN:
        case ARETURN:
          THIS->do_return(curBC);
          break;
        case RETURN:
          THIS->do_return_void(curBC);
          break;
        case GETSTATIC:
          THIS->do_getstatic(curBC, readUShort(code, i));
          break;
        case PUTSTATIC:
          THIS->do_putstatic(curBC, readUShort(code, i));
          break;
        case GETFIELD:
          THIS->do_getfield(curBC, readUShort(code, i));
          break;
        case PUTFIELD:
          THIS->do_putfield(curBC, readUShort(code, i));
          break;
        case INVOKEVIRTUAL:
          THIS->do_invokevirtual(curBC, readUShort(code, i));
          break;
        case INVOKESPECIAL:
          THIS->do_invokespecial(curBC, readUShort(code, i));
          break;
        case INVOKESTATIC:
          THIS->do_invokestatic(curBC, readUShort(code, i));
          break;
        case INVOKEINTERFACE: {
          THIS->do_invokeinterface(curBC, readUShort(code, i));
          unsigned count = readUByte(code, i);
          unsigned zero = readUByte(code, i);
          break;
        }
        case XXXUNUSEDXXX:
          // FIXME: must throw something
          break;
        case NEW:
          THIS->do_new(curBC, readUShort(code, i));
          break;
        case NEWARRAY:
          THIS->do_newarray(curBC,
                            static_cast<JType>(readUByte(code, i)));
          break;
        case ANEWARRAY:
          THIS->do_anewarray(curBC, readUShort(code, i));
          break;
        case ARRAYLENGTH:
          THIS->do_arraylength(curBC);
          break;
        case ATHROW:
          THIS->do_athrow(curBC);
          break;
        case CHECKCAST:
          THIS->do_checkcast(curBC, readUShort(code, i));
          break;
        case INSTANCEOF:
          THIS->do_instanceof(curBC, readUShort(code, i));
          break;
        case MONITORENTER:
          THIS->do_monitorenter(curBC);
          break;
        case MONITOREXIT:
          THIS->do_monitorexit(curBC);
          break;
        case WIDE:
          // FIXME: must throw something
          break;
        case MULTIANEWARRAY:
          THIS->do_multianewarray(
            curBC, readUShort(code, i), readUByte(code, i));
          break;
        case IFNULL: {
          unsigned t = curBC + readUShort(code, i);
          THIS->do_if(curBC, EQ, REFERENCE, t, i + 1);
          break;
        }
        case IFNONNULL: {
          unsigned t = curBC + readUShort(code, i);
          THIS->do_if(curBC, NE, REFERENCE, t, i + 1);
          break;
        }
        case GOTO_W:
          THIS->do_goto(curBC, curBC + readSInt(code, i));
          break;
        case JSR_W:
          THIS->do_jsr(curBC, curBC + readSInt(code, i));
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

    /// @brief called on ACONST_NULL
    void do_aconst_null(unsigned bcI) { }
    /// @brief called on ICONST_<n>, SIPUSH and BIPUSH
    void do_iconst(unsigned bcI, int value) { }
    /// @brief called on LCONST_<n>
    void do_lconst(unsigned bcI, long long value) { }
    /// @brief called on FCONST_<n>
    void do_fconst(unsigned bcI, float value) { }
    /// @brief called on DCONST_<n>
    void do_dconst(unsigned bcI, double value) { }
    /// @brief called on LDC, LDC_W and LDC2_W
    void do_ldc(unsigned bcI, unsigned index) { }
    /// @brief called on ILOAD, LLOAD, FLOAD, DLOAD, ALOAD,
    /// ILOAD_<n>, LLOAD_<n>, FLOAD_<n>, DLOAD_<n>, and ALOAD_<n>
    void do_load(unsigned bcI, JType type, unsigned index) { }
    /// @brief called on IALOAD, LALOAD, FALOAD, DALOAD, AALOAD,
    /// BALOAD, CALOAD, and SALOAD
    void do_aload(unsigned bcI, JType type) { }
    /// @brief called on ISTORE, LSTORE, FSTORE, DSTORE, ASTORE,
    /// ISTORE_<n>, LSTORE_<n>, FSTORE_<n>, DSTORE_<n>, and
    /// ASTORE_<n>
    void do_store(unsigned bcI, JType type, unsigned index) { }
    /// @brief called on IASTORE, LASTORE, FASTORE, DASTORE, AASTORE,
    /// BASTORE, CASTORE, and SASTORE
    void do_astore(unsigned bcI, JType type) { }
    /// @brief called on POP
    void do_pop(unsigned bcI) { }
    /// @brief called on POP2
    void do_pop2(unsigned bcI) { }
    /// @brief called on DUP
    void do_dup(unsigned bcI) { }
    /// @brief called on DUP_X1
    void do_dup_x1(unsigned bcI) { }
    /// @brief called on DUP_X2
    void do_dup_x2(unsigned bcI) { }
    /// @brief called on DUP2
    void do_dup2(unsigned bcI) { }
    /// @brief called on DUP2_X1
    void do_dup2_x1(unsigned bcI) { }
    /// @brief called on DUP2_X2
    void do_dup2_x2(unsigned bcI) { }
    /// @brief called on SWAP
    void do_swap(unsigned bcI) { }
    /// @brief called on IADD, LADD, FADD, and DADD
    void do_add(unsigned bcI) { }
    /// @brief called on ISUB, LSUB, FSUB, and DSUB
    void do_sub(unsigned bcI) { }
    /// @brief called on IMUL, LMUL, FMUL, and DMUL
    void do_mul(unsigned bcI) { }
    /// @brief called on IDIV, LDIV, FDIV, and DDIV
    void do_div(unsigned bcI) { }
    /// @brief called on IREM, LREM, FREM, and DREM
    void do_rem(unsigned bcI) { }
    /// @brief called on INEG, LNEG, FNEG, and DNEG
    void do_neg(unsigned bcI) { }
    /// @brief called on ISHL and LSHL
    void do_shl(unsigned bcI) { }
    /// @brief called on ISHR and LSHR
    void do_shr(unsigned bcI) { }
    /// @brief called on IUSHR and LUSHR
    void do_ushr(unsigned bcI) { }
    /// @brief called on IAND and LAND
    void do_and(unsigned bcI) { }
    /// @brief called on IOR or LOR
    void do_or(unsigned bcI) { }
    /// @brief called on IXOR and LXOR
    void do_xor(unsigned bcI) { }
    /// @brief called on IINC
    void do_iinc(unsigned bcI, unsigned index, int amount) { }
    /// @brief called on I2L, I2F, I2D, L2I, L2F, L2D, F2I, F2L,
    /// F2D, D2I, D2L, D2F, I2B, I2C, and I2S
    void do_convert(unsigned bcI, JType to) { }
    /// @brief called on LCMP
    void do_lcmp(unsigned bcI) { }
    /// @brief called on FCMPL and DCMPL
    void do_cmpl(unsigned bcI) { }
    /// @brief called on FCMPG and DCMPG
    void do_cmpg(unsigned bcI) { }
    /// @brief called on IF<op>, IFNULL, and IFNONNULL
    void do_if(unsigned bcI, JSetCC cc, JType type,
               unsigned t, unsigned f) { }
    /// @brief called on IFCMP<op> and IFACMP<op>
    void do_ifcmp(unsigned bcI, JSetCC cc,
                  unsigned t, unsigned f) { }
    /// @brief called on GOTO and GOTO_W
    void do_goto(unsigned bcI, unsigned target) { }
    /// @brief called on JSR and JSR_W
    void do_jsr(unsigned bcI, unsigned target) { }
    /// @brief called on RET
    void do_ret(unsigned bcI, unsigned index) { }
    /// @brief called on TABLESWITCH and LOOKUPSWITCH
    void do_switch(unsigned bcI,
                   unsigned defTarget,
                   const SwitchCases& sw) { }
    /// @brief called on IRETURN, LRETURN, FRETURN, DRETURN and
    /// ARETURN
    void do_return(unsigned bcI) { }
    /// @brief called on RETURN
    void do_return_void(unsigned bcI) { }
    /// @brief called on GETSTATIC
    void do_getstatic(unsigned bcI, unsigned index) { }
    /// @brief called on PUTSTATIC
    void do_putstatic(unsigned bcI, unsigned index) { }
    /// @brief called on GETFIELD
    void do_getfield(unsigned bcI, unsigned index) { }
    /// @brief called on PUTFIELD
    void do_putfield(unsigned bcI, unsigned index) { }
    /// @brief called on INVOKEVIRTUAL
    void do_invokevirtual(unsigned bcI, unsigned index) { }
    /// @brief called on INVOKESPECIAL
    void do_invokespecial(unsigned bcI, unsigned index) { }
    /// @brief called on INVOKESTATIC
    void do_invokestatic(unsigned bcI, unsigned index) { }
    /// @brief called on INVOKEINTERFACE
    void do_invokeinterface(unsigned bcI, unsigned index) { }
    /// @brief called on NEW
    void do_new(unsigned bcI, unsigned index) { }
    /// @brief called on NEWARRAY
    void do_newarray(unsigned bcI, JType type) { }
    /// @brief called on ANEWARRAY
    void do_anewarray(unsigned bcI, unsigned index) { }
    /// @brief called on ARRAYLENGTH
    void do_arraylength(unsigned bcI) { }
    /// @brief called on ATHROW
    void do_athrow(unsigned bcI) { }
    /// @brief called on CHECKCAST
    void do_checkcast(unsigned bcI, unsigned index) { }
    /// @brief called on INSTANCEOF
    void do_instanceof(unsigned bcI, unsigned index) { }
    /// @brief called on MONITORENTER
    void do_monitorenter(unsigned bcI) { }
    /// @brief called on MONITOREXIT
    void do_monitorexit(unsigned bcI) { }
    /// @brief called on MULTIANEWARRAY
    void do_multianewarray(unsigned bcI,
                           unsigned index,
                           unsigned dims) { }
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_BYTECODEPARSER_H
