//===-- BytecodeParser.h - Java bytecode parser -----------------*- C++ -*-===//
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

  private:
    SwitchCases switchCases_;

  protected:
#define THIS ((SubClass*)this)

    /// @brief parse code pointed to by \c code beginning at \c start
    /// bytecode and ending at \c end bytecode
    ///
    /// This function parses the code pointed to by \c code and calls
    /// the subclass's do_<bytecode> method appropriately. When this
    /// function returns all code in [start, end) is parsed.
    void parse(const uint8_t* code, unsigned start, unsigned end) {
      for (unsigned i = start; i < end; ++i) {
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
          THIS->do_ldc2(readUShort(code, i));
          break;
        case ILOAD:
          THIS->do_iload(wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case LLOAD:
          THIS->do_lload(wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case FLOAD:
          THIS->do_fload(wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case DLOAD:
          THIS->do_dload(wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ALOAD:
          THIS->do_aload(wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ILOAD_0:
        case ILOAD_1:
        case ILOAD_2:
        case ILOAD_3:
          THIS->do_iload(code[i]-ILOAD_0);
          break;
        case LLOAD_0:
        case LLOAD_1:
        case LLOAD_2:
        case LLOAD_3:
          THIS->do_lload(code[i]-LLOAD_0);
          break;
        case FLOAD_0:
        case FLOAD_1:
        case FLOAD_2:
        case FLOAD_3:
          THIS->do_fload(code[i]-FLOAD_0);
          break;
        case DLOAD_0:
        case DLOAD_1:
        case DLOAD_2:
        case DLOAD_3:
          THIS->do_dload(code[i]-DLOAD_0);
          break;
        case ALOAD_0:
        case ALOAD_1:
        case ALOAD_2:
        case ALOAD_3:
          THIS->do_aload(code[i]-ALOAD_0);
          break;
        case IALOAD:
          THIS->do_iaload();
          break;
        case LALOAD:
          THIS->do_laload();
          break;
        case FALOAD:
          THIS->do_faload();
          break;
        case DALOAD:
          THIS->do_daload();
          break;
        case AALOAD:
          THIS->do_aaload();
          break;
        case BALOAD:
          THIS->do_baload();
          break;
        case CALOAD:
          THIS->do_caload();
          break;
        case SALOAD:
          THIS->do_saload();
          break;
        case ISTORE:
          THIS->do_istore(wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case LSTORE:
          THIS->do_lstore(wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case FSTORE:
          THIS->do_fstore(wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case DSTORE:
          THIS->do_dstore(wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ASTORE:
          THIS->do_astore(wide ? readUShort(code, i) : readUByte(code, i));
          break;
        case ISTORE_0:
        case ISTORE_1:
        case ISTORE_2:
        case ISTORE_3:
          THIS->do_istore(code[i]-ISTORE_0);
          break;
        case LSTORE_0:
        case LSTORE_1:
        case LSTORE_2:
        case LSTORE_3:
          THIS->do_lstore(code[i]-LSTORE_0);
          break;
        case FSTORE_0:
        case FSTORE_1:
        case FSTORE_2:
        case FSTORE_3:
          THIS->do_fstore(code[i]-FSTORE_0);
          break;
        case DSTORE_0:
        case DSTORE_1:
        case DSTORE_2:
        case DSTORE_3:
          THIS->do_dstore(code[i]-DSTORE_0);
          break;
        case ASTORE_0:
        case ASTORE_1:
        case ASTORE_2:
        case ASTORE_3:
          THIS->do_astore(code[i]-ASTORE_0);
          break;
        case IASTORE:
          THIS->do_iastore();
          break;
        case LASTORE:
          THIS->do_lastore();
          break;
        case FASTORE:
          THIS->do_fastore();
          break;
        case DASTORE:
          THIS->do_dastore();
          break;
        case AASTORE:
          THIS->do_aastore();
          break;
        case BASTORE:
          THIS->do_bastore();
          break;
        case CASTORE:
          THIS->do_castore();
          break;
        case SASTORE:
          THIS->do_sastore();
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
          THIS->do_iadd();
          break;
        case LADD:
          THIS->do_ladd();
          break;
        case FADD:
          THIS->do_fadd();
          break;
        case DADD:
          THIS->do_dadd();
          break;
        case ISUB:
          THIS->do_isub();
          break;
        case LSUB:
          THIS->do_lsub();
          break;
        case FSUB:
          THIS->do_fsub();
          break;
        case DSUB:
          THIS->do_dsub();
          break;
        case IMUL:
          THIS->do_imul();
          break;
        case LMUL:
          THIS->do_lmul();
          break;
        case FMUL:
          THIS->do_fmul();
          break;
        case DMUL:
          THIS->do_dmul();
          break;
        case IDIV:
          THIS->do_idiv();
          break;
        case LDIV:
          THIS->do_ldiv();
          break;
        case FDIV:
          THIS->do_fdiv();
          break;
        case DDIV:
          THIS->do_ddiv();
          break;
        case IREM:
          THIS->do_irem();
          break;
        case LREM:
          THIS->do_lrem();
          break;
        case FREM:
          THIS->do_frem();
          break;
        case DREM:
          THIS->do_drem();
          break;
        case INEG:
          THIS->do_ineg();
          break;
        case LNEG:
          THIS->do_lneg();
          break;
        case FNEG:
          THIS->do_fneg();
          break;
        case DNEG:
          THIS->do_dneg();
          break;
        case ISHL:
          THIS->do_ishl();
          break;
        case LSHL:
          THIS->do_lshl();
          break;
        case ISHR:
          THIS->do_ishr();
          break;
        case LSHR:
          THIS->do_lshr();
          break;
        case IUSHR:
          THIS->do_iushr();
          break;
        case LUSHR:
          THIS->do_lushr();
          break;
        case IAND:
          THIS->do_iand();
          break;
        case LAND:
          THIS->do_land();
          break;
        case IOR:
          THIS->do_ior();
          break;
        case LOR:
          THIS->do_lor();
          break;
        case IXOR:
          THIS->do_ixor();
          break;
        case LXOR:
          THIS->do_lxor();
          break;
        case IINC: {
          unsigned index = wide ? readUShort(code, i) : readUByte(code, i);
          int amount = wide ? readSShort(code, i) : readSByte(code, i);
          THIS->do_iinc(index, amount);
          break;
        }
        case I2L:
          THIS->do_i2l();
          break;
        case I2F:
          THIS->do_i2f();
          break;
        case I2D:
          THIS->do_i2d();
          break;
        case L2I:
          THIS->do_l2i();
          break;
        case L2F:
          THIS->do_l2f();
          break;
        case L2D:
          THIS->do_l2d();
          break;
        case F2I:
          THIS->do_f2i();
          break;
        case F2L:
          THIS->do_f2l();
          break;
        case F2D:
          THIS->do_f2d();
          break;
        case D2I:
          THIS->do_d2i();
          break;
        case D2L:
          THIS->do_d2l();
          break;
        case D2F:
          THIS->do_d2f();
          break;
        case I2B:
          THIS->do_i2b();
          break;
        case I2C:
          THIS->do_i2c();
          break;
        case I2S:
          THIS->do_i2s();
          break;
        case LCMP:
          THIS->do_lcmp();
          break;
        case FCMPL:
          THIS->do_fcmpl();
          break;
        case FCMPG:
          THIS->do_fcmpg();
          break;
        case DCMPL:
          THIS->do_dcmpl();
          break;
        case DCMPG:
          THIS->do_dcmpg();
          break;
        case IFEQ: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifeq(t, i + 1);
          break;
        }
        case IFNE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifne(t, i + 1);
          break;
        }
        case IFLT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_iflt(t, i + 1);
          break;
        }
        case IFGE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifge(t, i + 1);
          break;
        }
        case IFGT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifgt(t, i + 1);
          break;
        }
        case IFLE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifle(t, i + 1);
          break;
        }
        case IF_ICMPEQ: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if_icmpeq(t, i + 1);
          break;
        }
        case IF_ICMPNE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if_icmpne(t, i + 1);
          break;
        }
        case IF_ICMPLT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if_icmplt(t, i + 1);
          break;
        }
        case IF_ICMPGE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if_icmpge(t, i + 1);
          break;
        }
        case IF_ICMPGT: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if_icmpgt(t, i + 1);
          break;
        }
        case IF_ICMPLE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if_icmple(t, i + 1);
          break;
        }
        case IF_IACMPEQ: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if_acmpeq(t, i + 1);
          break;
        }
        case IF_IACMPNE: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_if_acmpne(t, i + 1);
          break;
        }
        case GOTO:
          THIS->do_goto(curBC + readSShort(code, i));
          break;
        case JSR: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_jsr(t, i + 1);
          break;
        }
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
          THIS->do_ireturn();
          break;
        case LRETURN:
          THIS->do_lreturn();
          break;
        case FRETURN:
          THIS->do_freturn();
          break;
        case DRETURN:
          THIS->do_dreturn();
          break;
        case ARETURN:
          THIS->do_areturn();
          break;
        case RETURN:
          THIS->do_return();
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
        case MULTIANEWARRAY: {
          unsigned index = readUShort(code, i);
          unsigned dims = readUByte(code, i);
          THIS->do_multianewarray(index, dims);
          break;
        }
        case IFNULL: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifnull(t, i + 1);
          break;
        }
        case IFNONNULL: {
          unsigned t = curBC + readSShort(code, i);
          THIS->do_ifnonnull(t, i + 1);
          break;
        }
        case GOTO_W:
          THIS->do_goto(curBC + readSInt(code, i));
          break;
        case JSR_W: {
          unsigned t = curBC + readSInt(code, i);
          THIS->do_jsr(t, i + 1);
          break;
        }
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
    /// @brief called on LDC and LDC_W
    void do_ldc(unsigned index) { }
    /// @brief called on LDC2_W
    void do_ldc2(unsigned index) { }
    /// @brief called on ILOAD and ILOAD_<n>
    void do_iload(unsigned index) { }
    /// @brief called on LLOAD and LLOAD_<n>
    void do_lload(unsigned index) { }
    /// @brief called on FLOAD and FLOAD_<n>
    void do_fload(unsigned index) { }
    /// @brief called on DLOAD and DLOAD_<n>
    void do_dload(unsigned index) { }
    /// @brief called on ALOAD and ALOAD_<n>
    void do_aload(unsigned index) { }
    /// @brief called on IALOAD
    void do_iaload() { }
    /// @brief called on LALOAD
    void do_laload() { }
    /// @brief called on FALOAD
    void do_faload() { }
    /// @brief called on DALOAD
    void do_daload() { }
    /// @brief called on AALOAD
    void do_aaload() { }
    /// @brief called on BALOAD
    void do_baload() { }
    /// @brief called on CALOAD
    void do_caload() { }
    /// @brief called on SALOAD
    void do_saload() { }
    /// @brief called on ISTORE and ISTORE_<n>
    void do_istore(unsigned index) { }
    /// @brief called on LSTORE and LSTORE_<n>
    void do_lstore(unsigned index) { }
    /// @brief called on FSTORE and FSTORE_<n>
    void do_fstore(unsigned index) { }
    /// @brief called on DSTORE and DSTORE_<n>
    void do_dstore(unsigned index) { }
    /// @brief called on ASTORE and ASTORE_<n>
    void do_astore(unsigned index) { }
    /// @brief called on IASTORE
    void do_iastore() { }
    /// @brief called on LASTORE
    void do_lastore() { }
    /// @brief called on FASTORE
    void do_fastore() { }
    /// @brief called on DASTORE
    void do_dastore() { }
    /// @brief called on AASTORE
    void do_aastore() { }
    /// @brief called on BASTORE
    void do_bastore() { }
    /// @brief called on CASTORE
    void do_castore() { }
    /// @brief called on SASTORE
    void do_sastore() { }
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
    /// @brief called on IADD
    void do_iadd() { }
    /// @brief called on LADD
    void do_ladd() { }
    /// @brief called on FADD
    void do_fadd() { }
    /// @brief called on DADD
    void do_dadd() { }
    /// @brief called on ISUB
    void do_isub() { }
    /// @brief called on LSUB
    void do_lsub() { }
    /// @brief called on FSUB
    void do_fsub() { }
    /// @brief called on DSUB
    void do_dsub() { }
    /// @brief called on IMUL
    void do_imul() { }
    /// @brief called on LMUL
    void do_lmul() { }
    /// @brief called on FMUL
    void do_fmul() { }
    /// @brief called on DMUL
    void do_dmul() { }
    /// @brief called on IDIV
    void do_idiv() { }
    /// @brief called on LDIV
    void do_ldiv() { }
    /// @brief called on FDIV
    void do_fdiv() { }
    /// @brief called on DDIV
    void do_ddiv() { }
    /// @brief called on IREM
    void do_irem() { }
    /// @brief called on LREM
    void do_lrem() { }
    /// @brief called on FREM
    void do_frem() { }
    /// @brief called on DREM
    void do_drem() { }
    /// @brief called on INEG
    void do_ineg() { }
    /// @brief called on LNEG
    void do_lneg() { }
    /// @brief called on FNEG
    void do_fneg() { }
    /// @brief called on DNEG
    void do_dneg() { }
    /// @brief called on ISHL
    void do_ishl() { }
    /// @brief called on LSHL
    void do_lshl() { }
    /// @brief called on ISHR
    void do_ishr() { }
    /// @brief called on LSHR
    void do_lshr() { }
    /// @brief called on IUSHR
    void do_iushr() { }
    /// @brief called on LUSHR
    void do_lushr() { }
    /// @brief called on IAND
    void do_iand() { }
    /// @brief called on LAND
    void do_land() { }
    /// @brief called on IOR
    void do_ior() { }
    /// @brief called on LOR
    void do_lor() { }
    /// @brief called on IXOR
    void do_ixor() { }
    /// @brief called on LXOR
    void do_lxor() { }
    /// @brief called on IINC
    void do_iinc(unsigned index, int amount) { }
    /// @brief called on I2L
    void do_i2l() { }
    /// @brief called on I2F
    void do_i2f() { }
    /// @brief called on I2D
    void do_i2d() { }
    /// @brief called on L2I
    void do_l2i() { }
    /// @brief called on L2F
    void do_l2f() { }
    /// @brief called on L2D
    void do_l2d() { }
    /// @brief called on F2I
    void do_f2i() { }
    /// @brief called on F2L
    void do_f2l() { }
    /// @brief called on F2D
    void do_f2d() { }
    /// @brief called on D2I
    void do_d2i() { }
    /// @brief called on D2L
    void do_d2l() { }
    /// @brief called on D2F
    void do_d2f() { }
    /// @brief called on I2B
    void do_i2b() { }
    /// @brief called on I2C
    void do_i2c() { }
    /// @brief called on I2S
    void do_i2s() { }
    /// @brief called on LCMP
    void do_lcmp() { }
    /// @brief called on FCMPL
    void do_fcmpl() { }
    /// @brief called on DCMPL
    void do_dcmpl() { }
    /// @brief called on FCMPG
    void do_fcmpg() { }
    /// @brief called on DCMPG
    void do_dcmpg() { }
    /// @brief called on IFEQ
    void do_ifeq(unsigned t, unsigned f) { }
    /// @brief called on IFNE
    void do_ifne(unsigned t, unsigned f) { }
    /// @brief called on IFLT
    void do_iflt(unsigned t, unsigned f) { }
    /// @brief called on IFGE
    void do_ifge(unsigned t, unsigned f) { }
    /// @brief called on IFGT
    void do_ifgt(unsigned t, unsigned f) { }
    /// @brief called on IFLE
    void do_ifle(unsigned t, unsigned f) { }
    /// @brief called on IF_ICMPEQ
    void do_if_icmpeq(unsigned t, unsigned f) { }
    /// @brief called on IF_ICMPNE
    void do_if_icmpne(unsigned t, unsigned f) { }
    /// @brief called on IF_ICMPLT
    void do_if_icmplt(unsigned t, unsigned f) { }
    /// @brief called on IF_ICMPGE
    void do_if_icmpge(unsigned t, unsigned f) { }
    /// @brief called on IF_ICMPGT
    void do_if_icmpgt(unsigned t, unsigned f) { }
    /// @brief called on IF_ICMPLE
    void do_if_icmple(unsigned t, unsigned f) { }
    /// @brief called on IF_ACMPEQ
    void do_if_acmpeq(unsigned t, unsigned f) { }
    /// @brief called on IF_ACMPNE
    void do_if_acmpne(unsigned t, unsigned f) { }
    /// @brief called on GOTO and GOTO_W
    void do_goto(unsigned target) { }
    /// @brief called on JSR and JSR_W
    void do_jsr(unsigned target, unsigned retAddress) { }
    /// @brief called on RET
    void do_ret(unsigned index) { }
    /// @brief called on TABLESWITCH and LOOKUPSWITCH
    void do_switch(unsigned defTarget, const SwitchCases& sw) { }
    /// @brief called on IRETURN
    void do_ireturn() { }
    /// @brief called on LRETURN
    void do_lreturn() { }
    /// @brief called on FRETURN
    void do_freturn() { }
    /// @brief called on DRETURN
    void do_dreturn() { }
    /// @brief called on ARETURN
    void do_areturn() { }
    /// @brief called on RETURN
    void do_return() { }
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
    /// @brief called on IFNULL
    void do_ifnull(unsigned t, unsigned f) { }
    /// @brief called on IFNONNULL
    void do_ifnonnull(unsigned t, unsigned f) { }
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_BYTECODEPARSER_H
