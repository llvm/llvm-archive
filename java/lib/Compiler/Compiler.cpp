//===-- Compiler.cpp - Java bytecode compiler -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains Java bytecode to LLVM bytecode compiler.
//
//===----------------------------------------------------------------------===//

#include <llvm/Java/Bytecode.h>
#include <llvm/Java/ClassFile.h>
#include <llvm/Java/Compiler.h>
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#include <llvm/Value.h>
#include <llvm/Type.h>
#include <Support/StringExtras.h>

using namespace llvm;
using namespace llvm::Java;

namespace {

    inline bool isTwoSlotValue(const Value* v) {
        return v->getType() == Type::LongTy | v->getType() == Type::DoubleTy;
    }

    inline bool isOneSlotValue(const Value* v) {
        return !isTwoSlotValue(v);
    }

    inline int readByteSigned(const uint8_t* code, unsigned& i) {
        return code[++i];
    }

    inline unsigned readByteUnsigned(const uint8_t* code, unsigned& i) {
        return code[++i];
    }

    inline int readShortSigned(const uint8_t* code, unsigned& i) {
        return (readByteSigned(code, i) << 8) | readByteUnsigned(code, i);
    }

    inline unsigned readShortUnsigned(const uint8_t* code, unsigned& i) {
        return (readByteUnsigned(code, i) << 8) | readByteUnsigned(code, i);
    }

    inline int readIntSigned(const uint8_t* code, unsigned& i) {
        return ((readByteUnsigned(code, i) << 24) |
                (readByteUnsigned(code, i) << 16) |
                (readByteUnsigned(code, i) << 8) |
                readByteUnsigned(code, i));
    }

    inline unsigned readIntUnsigned(const uint8_t* code, unsigned& i) {
        return readIntSigned(code, i);
    }

    inline void skipPadBytes(const uint8_t* code, unsigned& i) {
        while (((unsigned)&code[++i]) & 0XFF);
    }

} // namespace

void Compiler::compileMethodInit(Function& function,
                                 const CodeAttribute& codeAttr)
{
    while (!opStack_.empty())
        opStack_.pop();

    locals_.clear();
    locals_.assign(codeAttr.getMaxLocals(), NULL);

    bc2bbMap_.clear();
    bc2bbMap_.assign(codeAttr.getCodeSize(), NULL);
    bc2bbMap_[0] = new BasicBlock("entry", &function);

    const uint8_t* code = codeAttr.getCode();
    for (unsigned i = 0; i < codeAttr.getCodeSize(); ++i) {
        using namespace llvm::Java::Opcode;

        unsigned bcStart = i;
        bool wide = code[i] == WIDE;
        i += wide;
        switch (code[i]) {
        case BIPUSH:
        case LDC:
        case NEWARRAY:
            ++i;
            break;
        case ILOAD:
        case LLOAD:
        case FLOAD:
        case DLOAD:
        case ALOAD:
        case ISTORE:
        case LSTORE:
        case FSTORE:
        case DSTORE:
        case ASTORE:
        case RET:
            i += 1 + wide;
            break;
        case SIPUSH:
        case LDC_W:
        case LDC2_W:
        case GOTO:
        case JSR:
        case GETSTATIC:
        case PUTSTATIC:
        case GETFIELD:
        case PUTFIELD:
        case INVOKEVIRTUAL:
        case INVOKESPECIAL:
        case INVOKESTATIC:
        case INVOKEINTERFACE:
        case NEW:
        case ANEWARRAY:
        case ARRAYLENGTH:
        case ATHROW:
        case CHECKCAST:
        case INSTANCEOF:
            i += 2;
            break;
        case IINC:
            i += 2 * (1 + wide);
            break;
        case IFEQ:
        case IFNE:
        case IFLT:
        case IFGE:
        case IFGT:
        case IFLE:
        case IF_ICMPEQ:
        case IF_ICMPNE:
        case IF_ICMPLT:
        case IF_ICMPGE:
        case IF_ICMPGT:
        case IF_ICMPLE:
        case IF_ICMPACMPEQ:
        case IF_ICMPACMPNE:
        case IFNULL:
        case IFNONNULL: {
            unsigned index = readShortUnsigned(code, i);
            bc2bbMap_[bcStart] = new BasicBlock(
                std::string("bb@bc") + utostr(bcStart), &function);
            break;
        }
        case TABLESWITCH: {
            skipPadBytes(code, i);
            readIntSigned(code, i);
            int low = readIntSigned(code, i);
            int high = readIntSigned(code, i);
            unsigned offsetCount = high - low + 1;
            while (offsetCount--) {
                unsigned bcIndex = bcStart + readIntSigned(code, i);
                bc2bbMap_[bcIndex] = new BasicBlock(
                    std::string("bb@bc") + utostr(bcIndex), &function);
            }
            break;
        }
        case LOOKUPSWITCH: {
            skipPadBytes(code, i);
            unsigned pairCount = readIntUnsigned(code, i);
            while (pairCount--) {
                readIntSigned(code, i);
                unsigned bcIndex = bcStart + readIntSigned(code, i);
                bc2bbMap_[bcIndex] = new BasicBlock(
                    std::string("bb@bc") + utostr(bcIndex), &function);
            }
            break;
        }
        case XXXUNUSEDXXX:
            throw "FIXME: create new exception class";
        case MULTIANEWARRAY:
            i += 3;
            break;
        case GOTO_W:
        case JSR_W:
            i+= 4;
            break;
        default:
            break;
        }
    }

    unsigned i = 0;
    BasicBlock* bb = bc2bbMap_[i];
    while (++i < codeAttr.getCodeSize()) {
        if (bc2bbMap_[i])
            bc2bbMap_[i] = bb;
        else
            bb = bc2bbMap_[i];
    }
}

void Compiler::compileMethod(Module& module, const Java::Method& method) {
    using namespace llvm::Java::Opcode;

    Function* function =
        module.getOrInsertFunction(method.getName()->str(), Type::VoidTy);

    const Java::CodeAttribute* codeAttr =
        Java::getCodeAttribute(method.getAttributes());

    compileMethodInit(*function, *codeAttr);

    const uint8_t* code = codeAttr->getCode();
    for (unsigned i = 0; i < codeAttr->getCodeSize(); ++i) {
        unsigned bcStart = i;
        bool wide = code[i] == WIDE;
        i += wide;
        switch (code[i]) {
        case ACONST_NULL:
            // FIXME: should push a null pointer of type Object*
            opStack_.push(
                ConstantPointerNull::get(PointerType::get(Type::VoidTy)));
            break;
        case ICONST_M1:
        case ICONST_0:
        case ICONST_1:
        case ICONST_2:
        case ICONST_3:
        case ICONST_4:
        case ICONST_5:
            opStack_.push(ConstantInt::get(Type::IntTy, code[i]-ICONST_0));
            break;
        case LCONST_0:
        case LCONST_1:
            opStack_.push(ConstantInt::get(Type::LongTy, code[i]-LCONST_0));
            break;
        case FCONST_0:
        case FCONST_1:
        case FCONST_2:
            opStack_.push(ConstantFP::get(Type::FloatTy, code[i]-FCONST_0));
            break;
        case DCONST_0:
        case DCONST_1:
            opStack_.push(ConstantFP::get(Type::DoubleTy, code[i]-DCONST_0));
            break;
        case BIPUSH: {
            int imm = readByteSigned(code, i);
            opStack_.push(ConstantInt::get(Type::IntTy, imm));
            break;
        }
        case SIPUSH: {
            int imm = readShortSigned(code, i);
            opStack_.push(ConstantInt::get(Type::IntTy, imm));
            break;
        }
        case LDC: {
            unsigned index = readByteUnsigned(code, i);
            // FIXME: load constant from constant pool
        }
        case LDC_W: {
            unsigned index = readShortUnsigned(code, i);
            // FIXME: load constant from constant pool
        }
        case LDC2_W: {
            unsigned index = readShortUnsigned(code, i);
            // FIXME: load constant from constant pool
        }
        case ILOAD:
        case LLOAD:
        case FLOAD:
        case DLOAD:
        case ALOAD: {
            // FIXME: use opcodes to perform type checking
            unsigned index = readByteUnsigned(code, i);
            opStack_.push(locals_[index]);
            break;
        }
        case ILOAD_0:
        case ILOAD_1:
        case ILOAD_2:
        case ILOAD_3:
            opStack_.push(locals_[code[i]-ILOAD_0]);
            break;
        case LLOAD_0:
        case LLOAD_1:
        case LLOAD_2:
        case LLOAD_3:
            opStack_.push(locals_[code[i]-LLOAD_0]);
            break;
        case FLOAD_0:
        case FLOAD_1:
        case FLOAD_2:
        case FLOAD_3:
            opStack_.push(locals_[code[i]-FLOAD_0]);
            break;
        case DLOAD_0:
        case DLOAD_1:
        case DLOAD_2:
        case DLOAD_3:
            opStack_.push(locals_[code[i]-DLOAD_0]);
            break;
        case ALOAD_0:
        case ALOAD_1:
        case ALOAD_2:
        case ALOAD_3:
            opStack_.push(locals_[code[i]-ALOAD_0]);
            break;
        case IALOAD:
        case LALOAD:
        case FALOAD:
        case DALOAD:
        case AALOAD:
        case BALOAD:
        case CALOAD:
        case SALOAD:
        case ISTORE:
        case LSTORE:
        case FSTORE:
        case DSTORE:
        case ASTORE:
        case ISTORE_0:
        case ISTORE_1:
        case ISTORE_2:
        case ISTORE_3:
        case LSTORE_0:
        case LSTORE_1:
        case LSTORE_2:
        case LSTORE_3:
        case FSTORE_0:
        case FSTORE_1:
        case FSTORE_2:
        case FSTORE_3:
        case DSTORE_0:
        case DSTORE_1:
        case DSTORE_2:
        case DSTORE_3:
        case ASTORE_0:
        case ASTORE_1:
        case ASTORE_2:
        case ASTORE_3:
        case IASTORE:
        case LASTORE:
        case FASTORE:
        case DASTORE:
        case AASTORE:
        case BASTORE:
        case CASTORE:
        case SASTORE:
            assert(0 && "not implemented");
        case POP:
            opStack_.pop();
            break;
        case POP2:
            opStack_.pop();
            opStack_.pop();
            break;
        case DUP:
            opStack_.push(opStack_.top());
            break;
        case DUP_X1: {
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* v2 = opStack_.top(); opStack_.pop();
            opStack_.push(v1);
            opStack_.push(v2);
            opStack_.push(v1);
            break;
        }
        case DUP_X2: {
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* v2 = opStack_.top(); opStack_.pop();
            if (isOneSlotValue(v2)) {
                Value* v3 = opStack_.top(); opStack_.pop();
                opStack_.push(v1);
                opStack_.push(v3);
                opStack_.push(v2);
                opStack_.push(v1);
            }
            else {
                opStack_.push(v1);
                opStack_.push(v2);
                opStack_.push(v1);
            }
            break;
        }
        case DUP2: {
            Value* v1 = opStack_.top(); opStack_.pop();
            if (isOneSlotValue(v1)) {
                Value* v2 = opStack_.top(); opStack_.pop();
                opStack_.push(v2);
                opStack_.push(v1);
                opStack_.push(v2);
                opStack_.push(v1);
            }
            else {
                opStack_.push(v1);
                opStack_.push(v1);
            }
            break;
        }
        case DUP2_X1: {
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* v2 = opStack_.top(); opStack_.pop();
            if (isOneSlotValue(v1)) {
                Value* v3 = opStack_.top(); opStack_.pop();
                opStack_.push(v2);
                opStack_.push(v1);
                opStack_.push(v3);
                opStack_.push(v2);
                opStack_.push(v1);
            }
            else {
                opStack_.push(v1);
                opStack_.push(v2);
                opStack_.push(v1);
            }
            break;
        }
        case DUP2_X2: {
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* v2 = opStack_.top(); opStack_.pop();
            if (isOneSlotValue(v1)) {
                Value* v3 = opStack_.top(); opStack_.pop();
                if (isOneSlotValue(v3)) {
                    Value* v4 = opStack_.top(); opStack_.pop();
                    opStack_.push(v2);
                    opStack_.push(v1);
                    opStack_.push(v4);
                    opStack_.push(v3);
                    opStack_.push(v2);
                    opStack_.push(v1);
                }
                else {
                    opStack_.push(v2);
                    opStack_.push(v1);
                    opStack_.push(v3);
                    opStack_.push(v2);
                    opStack_.push(v1);
                }
            }
            else {
                if (isOneSlotValue(v2)) {
                    Value* v3 = opStack_.top(); opStack_.pop();
                    opStack_.push(v1);
                    opStack_.push(v3);
                    opStack_.push(v2);
                    opStack_.push(v1);
                }
                else {
                    opStack_.push(v1);
                    opStack_.push(v2);
                    opStack_.push(v1);
                }
            }
            break;
        }
        case SWAP: {
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* v2 = opStack_.top(); opStack_.pop();
            opStack_.push(v1);
            opStack_.push(v2);
            break;
        }
        case IADD:
        case LADD:
        case FADD:
        case DADD:
        case ISUB:
        case LSUB:
        case FSUB:
        case DSUB:
        case IMUL:
        case LMUL:
        case FMUL:
        case DMUL:
        case IDIV:
        case LDIV:
        case FDIV:
        case DDIV:
        case IREM:
        case LREM:
        case FREM:
        case DREM:
        case INEG:
        case LNEG:
        case FNEG:
        case DNEG:
        case ISHL:
        case LSHL:
        case ISHR:
        case LSHR:
        case IUSHR:
        case LUSHR:
        case IAND:
        case LAND:
        case IOR:
        case LOR:
        case IXOR:
        case LXOR:
        case IINC:
        case I2L:
        case I2F:
        case I2D:
        case L2I:
        case L2F:
        case L2D:
        case F2I:
        case F2L:
        case F2D:
        case D2I:
        case D2L:
        case D2F:
        case I2B:
        case I2C:
        case I2S:
        case LCMP:
        case FCMPL:
        case FCMPG:
        case DCMPL:
        case DCMPG:
        case IFEQ:
        case IFNE:
        case IFLT:
        case IFGE:
        case IFGT:
        case IFLE:
        case IF_ICMPEQ:
        case IF_ICMPNE:
        case IF_ICMPLT:
        case IF_ICMPGE:
        case IF_ICMPGT:
        case IF_ICMPLE:
        case IF_ICMPACMPEQ:
        case IF_ICMPACMPNE:
        case GOTO:
        case JSR:
        case RET:
        case TABLESWITCH:
        case LOOKUPSWITCH:
        case IRETURN:
        case LRETURN:
        case FRETURN:
        case DRETURN:
        case ARETURN:
        case RETURN:
        case GETSTATIC:
        case PUTSTATIC:
        case GETFIELD:
        case PUTFIELD:
        case INVOKEVIRTUAL:
        case INVOKESPECIAL:
        case INVOKESTATIC:
        case INVOKEINTERFACE:
        case XXXUNUSEDXXX:
        case NEW:
        case NEWARRAY:
        case ANEWARRAY:
        case ARRAYLENGTH:
        case ATHROW:
        case CHECKCAST:
        case INSTANCEOF:
        case MONITORENTER:
        case MONITOREXIT:
        case WIDE:
        case MULTIANEWARRAY:
        case IFNULL:
        case IFNONNULL:
        case GOTO_W:
        case JSR_W:
        case BREAKPOINT:
        case IMPDEP1:
        case IMPDEP2:
        case NOP:
            break;
        }
    }
}

Module* Compiler::compile(const ClassFile& cf)
{
    Module* module = new Module(cf.getThisClass()->getName()->str());

    const Java::Methods& methods = cf.getMethods();
    for (Java::Methods::const_iterator
             i = methods.begin(), e = methods.end(); i != e; ++i)
        compileMethod(*module, **i);

    return module;
}
