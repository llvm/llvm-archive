/*
 * Formatting.cpp
 */

#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"

#include "llbrowse/Formatting.h"

using namespace llvm;

void printType(wxTextOutputStream & out, const Module* module,
    const Type* ty, uint32_t maxDepth) {
  std::string typeName = module->getTypeName(ty);
  if (!typeName.empty()) {
    out << wxString::From8BitData(&*typeName.begin(), typeName.size());
    return;
  }

  printTypeExpansion(out, module, ty, maxDepth);
}

void printTypeExpansion(wxTextOutputStream & out, const Module* module,
    const Type* ty, uint32_t maxDepth) {
  switch (ty->getTypeID()) {
    case Type::VoidTyID: out << _("void"); break;
    case Type::FloatTyID: out << _("float"); break;
    case Type::DoubleTyID: out << _("double"); break;
    case Type::X86_FP80TyID: out << _("x86_fp80"); break;
    case Type::FP128TyID: out << _("fp128"); break;
    case Type::PPC_FP128TyID: out << _("ppc_fp128"); break;
    #if HAVE_LLVM_TYPE_X86_FP80TY_ID
      case Type::X86_MMXTyID: out << _("x86mmx"); break;
    #endif
    case Type::MetadataTyID: out << _("metadata"); break;
    case Type::LabelTyID: out << _("label"); break;

    case Type::IntegerTyID: {
      const IntegerType* iTy = cast<IntegerType>(ty);
      out << _("int") << iTy->getBitWidth();
      break;
    }

    case Type::FunctionTyID: {
      const FunctionType* fTy = cast<FunctionType>(ty);
      printType(out, module, fTy->getReturnType(), maxDepth);
      out << _(" (");
      unsigned numParams = fTy->getNumParams();
      for (unsigned i = 0; i < numParams; ++i) {
        if (i != 0) {
          out << _(", ");
        }
        printType(out, module, fTy->getParamType(i), maxDepth);
      }
      out << _(")");
      break;
    }

    case Type::StructTyID: {
      const StructType* sTy = cast<StructType>(ty);
      if (maxDepth == 0) {
        out << _("{...}");
      } else {
        out << (sTy->isPacked() ? _("<{") : _("{"));
        for (int i = 0, ct = sTy->getNumContainedTypes(); i < ct; ++i) {
          if (i != 0) {
            out << _(", ");
          }

          if (maxDepth == 1 && i > 3) {
            out << _("...");
            break;
          }
          printType(out, module, sTy->getContainedType(i), maxDepth - 1);
        }
        out << (sTy->isPacked() ? _("}>") : _("}"));
      }
      (void) sTy;
      break;
    }

    case Type::ArrayTyID: {
      const ArrayType* aTy = cast<ArrayType>(ty);
      out << _("[") << int(aTy->getNumElements()) << _(" x ");
      printType(out, module, aTy->getContainedType(0), maxDepth);
      out << _("]");
      break;
    }

    case Type::PointerTyID: {
      const PointerType* pTy = cast<PointerType>(ty);
      printType(out, module, ty->getContainedType(0), maxDepth);
      if (pTy->getAddressSpace() != 0) {
        out << _(" addrspace (") << pTy->getAddressSpace() << _(")");
      }
      out << _("*");
      break;
    }

    case Type::VectorTyID: {
      const VectorType* vTy = cast<VectorType>(ty);
      out << _("<") << int(vTy->getNumElements()) << _(" x ");
      printType(out, module, vTy->getContainedType(0), maxDepth);
      out << _(">");
      break;
    }

    case Type::OpaqueTyID:
      out << _("opaque");
      break;

    default:
      out << _("???");
      break;
  }
}

/// Pretty-print an LLVM constant expression to a stream.
/// 'maxDepth' limits expansion of complex expressions.
void printConstant(wxTextOutputStream & out, const llvm::Module* module,
    const llvm::Constant* val, uint32_t maxDepth) {
  if (const ConstantInt* ci = dyn_cast<ConstantInt>(val)) {
    SmallVector<char, 16> intVal;
    ci->getValue().toString(intVal, 10, true);
    out << toWxStr(StringRef(intVal.data(), intVal.size()));
  } else if (const ConstantFP* cf = dyn_cast<ConstantFP>(val)) {
    // TODO: Implement
    (void) cf;
    out << _("FP?");
  } else if (const ConstantAggregateZero* cz =
      dyn_cast<ConstantAggregateZero>(val)) {
    (void) cz;
    out << _("constantzero");
  } else if (const ConstantArray* ca = dyn_cast<ConstantArray>(val)) {
    if (maxDepth > 0) {
      out << _("[");
      printConstantList(out, module, ca, maxDepth);
      out << _("]");
    } else {
      out << _("[...]");
    }
  } else if (const ConstantStruct* cs = dyn_cast<ConstantStruct>(val)) {
    if (maxDepth > 0) {
      out << _("{");
      printConstantList(out, module, cs, maxDepth);
      out << _("}");
    } else {
      out << _("{...}");
    }
  } else if (const ConstantVector* cv = dyn_cast<ConstantVector>(val)) {
    if (maxDepth > 0) {
      out << _("<");
      printConstantList(out, module, cv, maxDepth);
      out << _(">");
    } else {
      out << _("<...>");
    }
  } else if (isa<ConstantPointerNull>(val)) {
    printType(out, module, val->getType(), maxDepth);
    out << _(" null");
  } else if (const BlockAddress* ba = dyn_cast<BlockAddress>(val)) {
    // TODO: Implement
    (void) ba;
    out << _("BA?");
  } else if (const ConstantExpr* ce = dyn_cast<ConstantExpr>(val)) {
    out << wxString::From8BitData(ce->getOpcodeName());
    if (maxDepth > 0) {
      out << _("(");
      printConstantList(out, module, ce, maxDepth);
      out << _(")");
    } else {
      out << _("(...)");
    }
    (void) ce;
  } else if (isa<UndefValue>(val)) {
    out << _("undef");
  } else if (const GlobalValue* gv = dyn_cast<GlobalValue>(val)) {
    out << toWxStr(gv->getName());
  } else {
    out << _("???");
  }
}

void printConstantList(wxTextOutputStream & out, const llvm::Module* module,
    const llvm::Constant* parent, uint32_t maxDepth) {
  unsigned ct = parent->getNumOperands();
  for (unsigned i = 0; i < ct; ++i) {
    if (i != 0) {
      out << _(", ");
    }

    if (i > 3) {
      out << _("...");
      break;
    }

    printConstant(out, module, cast<Constant>(parent->getOperand(i)),
        maxDepth - 1);
  }
}

void printMetadata(wxTextOutputStream & out, const llvm::Value* nodeVal,
    uint32_t maxDepth) {
  if (const MDString* ms = dyn_cast<MDString>(nodeVal)) {
    out << _("\"") << toWxStr(ms->getName()) << _("\"");
  } else if (const MDNode* m = dyn_cast<MDNode>(nodeVal)) {
    if (maxDepth > 0) {
      out << _("{");
      for (unsigned i = 0; i < m->getNumOperands(); ++i) {
        if (i != 0) {
          out << _(", ");
        }
        if (maxDepth == 1 && i > 3) {
          out << _("...");
          break;
        }

        printMetadata(out, m->getOperand(i), maxDepth - 1);
      }
      out << _("}");
    } else {
      out << _("{...}");
    }
  } else if (const GlobalVariable* gv = dyn_cast<GlobalVariable>(nodeVal)) {
    out << toWxStr(gv->getName());
  } else if (const Function* fn = dyn_cast<Function>(nodeVal)) {
    out << toWxStr(fn->getName());
  } else if (const ConstantInt* ci = dyn_cast<ConstantInt>(nodeVal)) {
    SmallVector<char, 16> intVal;
    ci->getValue().toString(intVal, 10, true);
    out << toWxStr(StringRef(intVal.data(), intVal.size()));
  } else {
    out << _("???");
  }
}

