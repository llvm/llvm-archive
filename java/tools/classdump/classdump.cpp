//===-- classdump.cpp - classdump utility -----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is a sample class reader driver. It is used to drive class
// reader tests.
//
//===----------------------------------------------------------------------===//

#include <llvm/Java/ClassFile.h>
#include <llvm/Java/BytecodeParser.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/System/Signals.h>

#include <cstddef>
#include <iostream>

using namespace llvm;

static cl::opt<std::string>
InputClass(cl::Positional, cl::desc("<input class>"));

namespace {

  using namespace llvm::Java;

  class ClassDump : public BytecodeParser<ClassDump> {
    const ClassFile* CF;
    std::ostream& Out;

  public:
    ClassDump(const ClassFile* cf, std::ostream& out)
      : CF(cf), Out(out) {

      if (CF->isPublic())
        Out << "public ";
      if (CF->isFinal())
        Out << "final ";

      Out << "class " << CF->getThisClass()->getName()->str() << ' ';

      if (ConstantClass* super = CF->getSuperClass())
        Out << "extends " << super->getName()->str() << ' ';

      if (!CF->getInterfaces().empty()) {
        const Classes& interfaces = CF->getInterfaces();
        Out << "implements " << interfaces[0]->getName()->str();
        for (unsigned i = 1, e = interfaces.size(); i != e; ++i)
          Out << ", " << interfaces[i]->getName()->str();
        Out << ' ';
      }
      Out << "{\n";

      const Fields& fields = CF->getFields();
      // Dump static fields.
      for (unsigned i = 0, e = fields.size(); i != e; ++i)
        if (fields[i]->isStatic())
          dumpField(fields[i]);
      // Dump instance fields.
      for (unsigned i = 0, e = fields.size(); i != e; ++i)
        if (!fields[i]->isStatic())
          dumpField(fields[i]);

      const Methods& methods = CF->getMethods();
      for (unsigned i = 0, e = methods.size(); i != e; ++i)
        dumpMethod(methods[i]);

      Out << "\n}\n";
    }

    void dumpField(const Field* F) {
      Out << '\n';
      if (F->isPublic())
        Out << "public ";
      else if (F->isProtected())
        Out << "protected ";
      else if (F->isPrivate())
        Out << "private ";

      if (F->isStatic())
        Out << "static ";

      if (F->isFinal())
        Out << "final ";

      if (F->isTransient())
        Out << "transient ";

      Out << getPrettyString(F->getDescriptor()->str()) << ' '
          << F->getName()->str() << ";\n";
    }

    void dumpMethod(const Method* M) {
      Out << '\n';
      if (M->isPublic())
        Out << "public ";
      else if (M->isProtected())
        Out << "protected ";
      else if (M->isPrivate())
        Out << "private ";

      if (M->isStatic())
        Out << "static ";

      if (M->isAbstract())
        Out << "abstract ";

      if (M->isFinal())
        Out << "final ";

      if (M->isSynchronized())
        Out << "synchronized ";

      std::string Signature = getPrettyString(M->getDescriptor()->str());
      Signature.insert(Signature.find('('), M->getName()->str());

      Out << Signature << ";\n";
      if (CodeAttribute* CodeAttr = M->getCodeAttribute()) {
        Out << "\tCode:";
        parse(CodeAttr->getCode(), 0, CodeAttr->getCodeSize());
        Out << '\n';
      }
    }

    std::string getPrettyString(const std::string& Desc) {
      unsigned I = 0;
      std::string prettyString = getPrettyStringHelper(Desc, I);
      for (unsigned i = 0, e = prettyString.size(); i != e; ++i)
        if (prettyString[i] == '/')
          prettyString[i] = '.';
      return prettyString;
    }

    std::string getPrettyStringHelper(const std::string& Desc, unsigned& I) {
      if (Desc.size() == I)
        return "";

      switch (Desc[I++]) {
      case 'B': return "byte";
      case 'C': return "char";
      case 'D': return "double";
      case 'F': return "float";
      case 'I': return "int";
      case 'J': return "long";
      case 'S': return "short";
      case 'Z': return "boolean";
      case 'V': return "void";
      case 'L': {
        unsigned E = Desc.find(';', I);
        std::string ClassName = Desc.substr(I, E - I);
        I = E + 1;
        return ClassName;
      }
      case '[': {
        std::string ArrayPart;
        ArrayPart += "[]";
        while (Desc[I] == '[') {
          ArrayPart += "[]";
          ++I;
        }

        return getPrettyStringHelper(Desc, I) + ArrayPart;
      }
      case '(': {
        std::string Params;
        while (Desc[I] != ')') {
          if (Desc[I-1] != '(')
            Params += ',';
          Params += getPrettyStringHelper(Desc, I);
        }
        return getPrettyStringHelper(Desc, ++I) + " (" + Params + ')';
      }
      }

      return "";
    }

    /// @brief called before every bytecode
    void pre_inst(unsigned bcI) { Out << "\n\t " << bcI << ":\t"; }
    /// @brief called on ACONST_NULL
    void do_aconst_null() { Out << "aconst_null"; }
    /// @brief called on ICONST_<n>, SIPUSH and BIPUSH
    void do_iconst(int value) {
      if (value == -1)
        Out << "iconst_m1";
      else if (value >=0 && value <= 5)
        Out << "iconst_" << value;
      else if (value >= -128 && value <= 127)
        Out << "bipush " << value;
      else
        Out << "sipush " << value;
    }
    /// @brief called on LCONST_<n>
    void do_lconst(long long value) {
      if (value == 0 || value == 1)
        Out << "lconst_" << value;
      else
        Out << "lconst " << value;
    }
    /// @brief called on FCONST_<n>
    void do_fconst(float value) {
      if (value >= 0 && value <= 2)
        Out << "fconst_" << value;
      else
        Out << "fconst " << value;
    }
    /// @brief called on DCONST_<n>
    void do_dconst(double value) {
      if (value == 0 || value == 1)
        Out << "dconst_" << value;
      else
        Out << "dconst " << value;
    }
    /// @brief called on LDC and LDC_W
    void do_ldc(unsigned index) {
      if (index <= 255)
        Out << "ldc";
      else
        Out << "ldc_w";
      Out <<  "\t#" << index << "; //" << *CF->getConstant(index);      
    }
    /// @brief called on LDC2_W
    void do_ldc2(unsigned index) {
      Out << "ldc2_w \t#" << index << "; //" << *CF->getConstant(index);
    }
    /// @brief called on ILOAD and ILOAD_<n>
    void do_iload(unsigned index) {
      if (index <= 3)
        Out << "iload_" << index;
      else
        Out << "iload " << index;
    }
    /// @brief called on LLOAD and LLOAD_<n>
    void do_lload(unsigned index) {
      if (index <= 3)
        Out << "lload_" << index;
      else
        Out << "lload " << index;
    }
    /// @brief called on FLOAD and FLOAD_<n>
    void do_fload(unsigned index) {
      if (index <= 3)
        Out << "fload_" << index;
      else
        Out << "fload " << index;
    }
    /// @brief called on DLOAD and DLOAD_<n>
    void do_dload(unsigned index) {
      if (index <= 3)
        Out << "dload_" << index;
      else
        Out << "dload " << index;
    }
    /// @brief called on ALOAD and ALOAD_<n>
    void do_aload(unsigned index) {
      if (index <= 3)
        Out << "aload_" << index;
      else
        Out << "aload " << index;
    }
    /// @brief called on IALOAD
    void do_iaload() { Out << "iaload"; }
    /// @brief called on LALOAD
    void do_laload() { Out << "laload"; }
    /// @brief called on FALOAD
    void do_faload() { Out << "faload"; }
    /// @brief called on DALOAD
    void do_daload() { Out << "daload"; }
    /// @brief called on AALOAD
    void do_aaload() { Out << "aaload"; }
    /// @brief called on BALOAD
    void do_baload() { Out << "baload"; }
    /// @brief called on CALOAD
    void do_caload() { Out << "caload"; }
    /// @brief called on SALOAD
    void do_saload() { Out << "saload"; }
    /// @brief called on ISTORE and ISTORE_<n>
    void do_istore(unsigned index) {
      if (index <= 3)
        Out << "istore_" << index;
      else
        Out << "istore " << index;
    }
    /// @brief called on LSTORE and LSTORE_<n>
    void do_lstore(unsigned index) {
      if (index <= 3)
        Out << "lstore_" << index;
      else
        Out << "lstore " << index;
    }
    /// @brief called on FSTORE and FSTORE_<n>
    void do_fstore(unsigned index) {
      if (index <= 3)
        Out << "fstore_" << index;
      else
        Out << "fstore " << index;
    }
    /// @brief called on DSTORE and DSTORE_<n>
    void do_dstore(unsigned index) {
      if (index <= 3)
        Out << "dstore_" << index;
      else
        Out << "dstore " << index;
    }
    /// @brief called on ASTORE and ASTORE_<n>
    void do_astore(unsigned index) {
      if (index <= 3)
        Out << "astore_" << index;
      else
        Out << "astore " << index;
    }
    /// @brief called on IASTORE
    void do_iastore() { Out << "iastore"; }
    /// @brief called on LASTORE
    void do_lastore() { Out << "lastore"; }
    /// @brief called on FASTORE
    void do_fastore() { Out << "fastore"; }
    /// @brief called on DASTORE
    void do_dastore() { Out << "dastore"; }
    /// @brief called on AASTORE
    void do_aastore() { Out << "aastore"; }
    /// @brief called on BASTORE
    void do_bastore() { Out << "bastore"; }
    /// @brief called on CASTORE
    void do_castore() { Out << "castore"; }
    /// @brief called on SASTORE
    void do_sastore() { Out << "sastore"; }
    /// @brief called on POP
    void do_pop() { Out << "pop"; }
    /// @brief called on POP2
    void do_pop2() { Out << "pop2"; }
    /// @brief called on DUP
    void do_dup() { Out << "dup"; }
    /// @brief called on DUP_X1
    void do_dup_x1() { Out << "dup_x1"; }
    /// @brief called on DUP_X2
    void do_dup_x2() { Out << "dup_x2"; }
    /// @brief called on DUP2
    void do_dup2() { Out << "dup2"; }
    /// @brief called on DUP2_X1
    void do_dup2_x1() { Out << "dup2_x1"; }
    /// @brief called on DUP2_X2
    void do_dup2_x2() { Out << "dup2_x2"; }
    /// @brief called on SWAP
    void do_swap() { Out << "swap"; }
    /// @brief called on IADD
    void do_iadd() { Out << "iadd"; }
    /// @brief called on LADD
    void do_ladd() { Out << "ladd"; }
    /// @brief called on FADD
    void do_fadd() { Out << "fadd"; }
    /// @brief called on DADD
    void do_dadd() { Out << "dadd"; }
    /// @brief called on ISUB
    void do_isub() { Out << "isub"; }
    /// @brief called on LSUB
    void do_lsub() { Out << "lsub"; }
    /// @brief called on FSUB
    void do_fsub() { Out << "fsub"; }
    /// @brief called on DSUB
    void do_dsub() { Out << "dsub"; }
    /// @brief called on IMUL
    void do_imul() { Out << "imul"; }
    /// @brief called on LMUL
    void do_lmul() { Out << "lmul"; }
    /// @brief called on FMUL
    void do_fmul() { Out << "fmul"; }
    /// @brief called on DMUL
    void do_dmul() { Out << "dmul"; }
    /// @brief called on IDIV
    void do_idiv() { Out << "idiv"; }
    /// @brief called on LDIV
    void do_ldiv() { Out << "ldiv"; }
    /// @brief called on FDIV
    void do_fdiv() { Out << "fdiv"; }
    /// @brief called on DDIV
    void do_ddiv() { Out << "ddiv"; }
    /// @brief called on IREM
    void do_irem() { Out << "irem"; }
    /// @brief called on LREM
    void do_lrem() { Out << "lrem"; }
    /// @brief called on FREM
    void do_frem() { Out << "frem"; }
    /// @brief called on DREM
    void do_drem() { Out << "drem"; }
    /// @brief called on INEG
    void do_ineg() { Out << "ineg"; }
    /// @brief called on LNEG
    void do_lneg() { Out << "lneg"; }
    /// @brief called on FNEG
    void do_fneg() { Out << "fneg"; }
    /// @brief called on DNEG
    void do_dneg() { Out << "dneg"; }
    /// @brief called on ISHL
    void do_ishl() { Out << "ishl"; }
    /// @brief called on LSHL
    void do_lshl() { Out << "lshl"; }
    /// @brief called on ISHR
    void do_ishr() { Out << "ishr"; }
    /// @brief called on LSHR
    void do_lshr() { Out << "lshr"; }
    /// @brief called on IUSHR
    void do_iushr() { Out << "iushr"; }
    /// @brief called on LUSHR
    void do_lushr() { Out << "lushr"; }
    /// @brief called on IAND
    void do_iand() { Out << "iand"; }
    /// @brief called on LAND
    void do_land() { Out << "land"; }
    /// @brief called on IOR
    void do_ior() { Out << "ior"; }
    /// @brief called on LOR
    void do_lor() { Out << "lor"; }
    /// @brief called on IXOR
    void do_ixor() { Out << "ixor"; }
    /// @brief called on LXOR
    void do_lxor() { Out << "lxor"; }
    /// @brief called on IINC
    void do_iinc(unsigned index, int amount) {
      Out << "iinc " << index << ", " << amount;
    }
    /// @brief called on I2L
    void do_i2l() { Out << "i2l"; }
    /// @brief called on I2F
    void do_i2f() { Out << "i2f"; }
    /// @brief called on I2D
    void do_i2d() { Out << "i2d"; }
    /// @brief called on L2I
    void do_l2i() { Out << "l2i"; }
    /// @brief called on L2F
    void do_l2f() { Out << "l2f"; }
    /// @brief called on L2D
    void do_l2d() { Out << "l2d"; }
    /// @brief called on F2I
    void do_f2i() { Out << "f2i"; }
    /// @brief called on F2L
    void do_f2l() { Out << "f2l"; }
    /// @brief called on F2D
    void do_f2d() { Out << "f2d"; }
    /// @brief called on D2I
    void do_d2i() { Out << "d2i"; }
    /// @brief called on D2L
    void do_d2l() { Out << "d2l"; }
    /// @brief called on D2F
    void do_d2f() { Out << "d2f"; }
    /// @brief called on I2B
    void do_i2b() { Out << "i2b"; }
    /// @brief called on I2C
    void do_i2c() { Out << "i2c"; }
    /// @brief called on I2S
    void do_i2s() { Out << "i2s"; }
    /// @brief called on LCMP
    void do_lcmp() { Out << "lcmp"; }
    /// @brief called on FCMPL
    void do_fcmpl() { Out << "fcmpl"; }
    /// @brief called on DCMPL
    void do_dcmpl() { Out << "dcmpl"; }
    /// @brief called on FCMPG
    void do_fcmpg() { Out << "fcmpg"; }
    /// @brief called on DCMPG
    void do_dcmpg() { Out << "dcmpg"; }
    /// @brief called on IFEQ
    void do_ifeq(unsigned t, unsigned f) { Out << "ifeq " << t; }
    /// @brief called on IFNE
    void do_ifne(unsigned t, unsigned f) { Out << "ifne " << t; }
    /// @brief called on IFLT
    void do_iflt(unsigned t, unsigned f) { Out << "iflt " << t; }
    /// @brief called on IFGE
    void do_ifge(unsigned t, unsigned f) { Out << "ifge " << t; }
    /// @brief called on IFGT
    void do_ifgt(unsigned t, unsigned f) { Out << "ifgt " << t; }
    /// @brief called on IFLE
    void do_ifle(unsigned t, unsigned f) { Out << "ifle " << t; }
    /// @brief called on IF_ICMPEQ
    void do_if_icmpeq(unsigned t, unsigned f) { Out << "if_icmpeq " << t; }
    /// @brief called on IF_ICMPNE
    void do_if_icmpne(unsigned t, unsigned f) { Out << "if_icmpne " << t; }
    /// @brief called on IF_ICMPLT
    void do_if_icmplt(unsigned t, unsigned f) { Out << "if_icmplt " << t; }
    /// @brief called on IF_ICMPGE
    void do_if_icmpge(unsigned t, unsigned f) { Out << "if_icmpge " << t; }
    /// @brief called on IF_ICMPGT
    void do_if_icmpgt(unsigned t, unsigned f) { Out << "if_icmpgt " << t; }
    /// @brief called on IF_ICMPLE
    void do_if_icmple(unsigned t, unsigned f) { Out << "if_icmple " << t; }
    /// @brief called on IF_ACMPEQ
    void do_if_acmpeq(unsigned t, unsigned f) { Out << "if_acmpeq " << t; }
    /// @brief called on IF_ACMPNE
    void do_if_acmpne(unsigned t, unsigned f) { Out << "if_acmpne " << t; }
    /// @brief called on GOTO and GOTO_W
    void do_goto(unsigned target) { Out << "goto " << target; }
    /// @brief called on JSR and JSR_W
    void do_jsr(unsigned target, unsigned retAddress) { abort(); }
    /// @brief called on RET
    void do_ret(unsigned index) { abort(); }
    /// @brief called on TABLESWITCH and LOOKUPSWITCH
    void do_switch(unsigned defTarget, const SwitchCases& sw) { abort(); }
    /// @brief called on IRETURN
    void do_ireturn() { Out << "ireturn"; }
    /// @brief called on LRETURN
    void do_lreturn() { Out << "lreturn"; }
    /// @brief called on FRETURN
    void do_freturn() { Out << "freturn"; }
    /// @brief called on DRETURN
    void do_dreturn() { Out << "dreturn"; }
    /// @brief called on ARETURN
    void do_areturn() { Out << "areturn"; }
    /// @brief called on RETURN
    void do_return() { Out << "return"; }
    /// @brief called on GETSTATIC
    void do_getstatic(unsigned index) {
      Out << "getstatic #" << index << "; //Field ";
      printMemberRef(index);
    }
    /// @brief called on PUTSTATIC
    void do_putstatic(unsigned index) {
      Out << "putstatic #" << index << "; //Field ";
      printMemberRef(index);
    }
    /// @brief called on GETFIELD
    void do_getfield(unsigned index) {
      Out << "getfield #" << index << "; //Field ";
      printMemberRef(index);
    }
    /// @brief called on PUTFIELD
    void do_putfield(unsigned index) {
      Out << "putfield #" << index << "; //Field ";
      printMemberRef(index);
    }
    /// @brief called on INVOKEVIRTUAL
    void do_invokevirtual(unsigned index) {
      Out << "invokevirtual #" << index << "; //Method ";
      printMemberRef(index);
    }
    /// @brief called on INVOKESPECIAL
    void do_invokespecial(unsigned index) {
      Out << "invokespecial #" << index << "; //Method ";
      printMemberRef(index);
    }
    /// @brief called on INVOKESTATIC
    void do_invokestatic(unsigned index) {
      Out << "invokestatic #" << index << "; //Method ";
      printMemberRef(index);
    }
    /// @brief called on INVOKEINTERFACE
    void do_invokeinterface(unsigned index) {
      Out << "invokeinterface #" << index << "; //InterfaceMethod ";
      printMemberRef(index);
    }
    /// @brief called on NEW
    void do_new(unsigned index) {
      Out << "new #" << index << "; //class ";
      printClassRef(index);
    }
    /// @brief called on NEWARRAY
    void do_newarray(JType type) {
      Out << "newarray ";
      switch (type) {
      case BOOLEAN: Out << "boolean"; break;
      case CHAR: Out << "char"; break;
      case FLOAT: Out << "float"; break;
      case DOUBLE: Out << "byte"; break;
      case SHORT: Out << "short"; break;
      case INT: Out << "int"; break;
      case LONG: Out << "long"; break;
      default: assert(0 && "Unknown type for newarray!");
      }
    }

    /// @brief called on ANEWARRAY
    void do_anewarray(unsigned index) {
      Out << "anewarray #" << index << "; //class ";
      printClassRef(index);
    }
    /// @brief called on ARRAYLENGTH
    void do_arraylength() { Out << "arraylength"; }
    /// @brief called on ATHROW
    void do_athrow() { Out << "athrow"; }
    /// @brief called on CHECKCAST
    void do_checkcast(unsigned index) {
      Out << "checkcast #" << index
          << "; //class ";
      printClassRef(index);
    }
    /// @brief called on INSTANCEOF
    void do_instanceof(unsigned index) {
      Out << "instanceof #" << index
          << "; //class ";
      printClassRef(index);
    }
    /// @brief called on MONITORENTER
    void do_monitorenter() { Out << "monitorenter"; }
    /// @brief called on MONITOREXIT
    void do_monitorexit() { Out << "monitorexit"; }
    /// @brief called on MULTIANEWARRAY
    void do_multianewarray(unsigned index, unsigned dims) { }
    /// @brief called on IFNULL
    void do_ifnull(unsigned t, unsigned f) { Out << "ifnull " << t; }
    /// @brief called on IFNONNULL
    void do_ifnonnull(unsigned t, unsigned f) { Out << "ifnonnull " << t; }

    void printMemberRef(unsigned index) {
      ConstantMemberRef* Ref = CF->getConstantMemberRef(index);
      ConstantClass* Class = Ref->getClass();
      if (Class != CF->getThisClass())
        Out << Class->getName()->str() << '.';
      Out << Ref->getNameAndType()->getName()->str()
          << ':'
          << Ref->getNameAndType()->getDescriptor()->str();
    }

    void printClassRef(unsigned index) {
      const std::string& FQCN = CF->getConstantClass(index)->getName()->str();
      Out << FQCN.substr(FQCN.rfind('/')+1);
    }
  };
}

int main(int argc, char* argv[])
{
  sys::PrintStackTraceOnErrorSignal();
  cl::ParseCommandLineOptions(argc, argv,
                              "class dump utility");

  try {
    const Java::ClassFile* cf(Java::ClassFile::get(InputClass));

    ClassDump(cf, std::cout);
  }
  catch (std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
