#include "HTMLPrinterUtils.h"
#include "llvm/Assembly/CachedWriter.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/Assembly/AsmAnnotationWriter.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/SymbolTable.h"
#include "llvm/Analysis/SlotCalculator.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Support/CFG.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/STLExtras.h"
#include <algorithm>
using namespace llvm;

const Module *getModuleFromVal(const Value *V) {
  if (const Argument *MA = dyn_cast<Argument>(V))
    return MA->getParent() ? MA->getParent()->getParent() : 0;
  else if (const BasicBlock *BB = dyn_cast<BasicBlock>(V))
    return BB->getParent() ? BB->getParent()->getParent() : 0;
  else if (const Instruction *I = dyn_cast<Instruction>(V)) {
    const Function *M = I->getParent() ? I->getParent()->getParent() : 0;
    return M ? M->getParent() : 0;
  } else if (const GlobalValue *GV = dyn_cast<GlobalValue>(V))
    return GV->getParent();
  return 0;
}

SlotCalculator *createSlotCalculator(const Value *V) {
  assert(!isa<Type>(V) && "Can't create an SC for a type!");
  if (const Argument *FA = dyn_cast<Argument>(V)) {
    return new SlotCalculator(FA->getParent());
  } else if (const Instruction *I = dyn_cast<Instruction>(V)) {
    return new SlotCalculator(I->getParent()->getParent());
  } else if (const BasicBlock *BB = dyn_cast<BasicBlock>(V)) {
    return new SlotCalculator(BB->getParent());
  } else if (const GlobalVariable *GV = dyn_cast<GlobalVariable>(V)){
    return new SlotCalculator(GV->getParent());
  } else if (const Function *Func = dyn_cast<Function>(V)) {
    return new SlotCalculator(Func);
  }
  return 0;
}

// getLLVMName - Turn the specified string into an 'LLVM name', which is either
// prefixed with % (if the string only contains simple characters) or is
// surrounded with ""'s (if it has special chars in it).
std::string getLLVMName(const std::string &Name) {
  assert(!Name.empty() && "Cannot get empty name!");

  // First character cannot start with a number...
  if (Name[0] >= '0' && Name[0] <= '9')
    return "\"" + Name + "\"";

  // Scan to see if we have any characters that are not on the "white list"
  for (unsigned i = 0, e = Name.size(); i != e; ++i) {
    char C = Name[i];
    assert(C != '"' && "Illegal character in LLVM value name!");
    if ((C < 'a' || C > 'z') && (C < 'A' || C > 'Z') && (C < '0' || C > '9') &&
        C != '-' && C != '.' && C != '_')
      return "\"" + Name + "\"";
  }
  
  // If we get here, then the identifier is legal to use as a "VarID".
  return "%"+Name;
}


/// fillTypeNameTable - If the module has a symbol table, take all global types
/// and stuff their names into the TypeNames map.
///
void fillTypeNameTable(const Module *M,
                       std::map<const Type *, std::string> &TypeNames) {
  if (!M) return;
  const SymbolTable &ST = M->getSymbolTable();
  SymbolTable::type_const_iterator TI = ST.type_begin();
  for (; TI != ST.type_end(); ++TI ) {
    // As a heuristic, don't insert pointer to primitive types, because
    // they are used too often to have a single useful name.
    //
    const Type *Ty = cast<Type>(TI->second);
    if (!isa<PointerType>(Ty) ||
        !cast<PointerType>(Ty)->getElementType()->isPrimitiveType() ||
        isa<OpaqueType>(cast<PointerType>(Ty)->getElementType()))
      TypeNames.insert(std::make_pair(Ty, getLLVMName(TI->first)));
  }
}


std::string calcTypeName(const Type *Ty, 
                         std::vector<const Type *> &TypeStack,
                         std::map<const Type *, std::string> &TypeNames) {
  if (Ty->isPrimitiveType() && !isa<OpaqueType>(Ty))
    return Ty->getDescription();  // Base case

  // Check to see if the type is named.
  std::map<const Type *, std::string>::iterator I = TypeNames.find(Ty);
  if (I != TypeNames.end()) return I->second;

  if (isa<OpaqueType>(Ty))
    return "opaque";

  // Check to see if the Type is already on the stack...
  unsigned Slot = 0, CurSize = TypeStack.size();
  while (Slot < CurSize && TypeStack[Slot] != Ty) ++Slot; // Scan for type

  // This is another base case for the recursion.  In this case, we know 
  // that we have looped back to a type that we have previously visited.
  // Generate the appropriate upreference to handle this.
  if (Slot < CurSize)
    return "\\" + utostr(CurSize-Slot);       // Here's the upreference

  TypeStack.push_back(Ty);    // Recursive case: Add us to the stack..
  
  std::string Result;
  switch (Ty->getPrimitiveID()) {
  case Type::FunctionTyID: {
    const FunctionType *FTy = cast<FunctionType>(Ty);
    Result = calcTypeName(FTy->getReturnType(), TypeStack, TypeNames) + " (";
    for (FunctionType::param_iterator I = FTy->param_begin(),
           E = FTy->param_end(); I != E; ++I) {
      if (I != FTy->param_begin())
        Result += ", ";
      Result += calcTypeName(*I, TypeStack, TypeNames);
    }
    if (FTy->isVarArg()) {
      if (FTy->getNumParams()) Result += ", ";
      Result += "...";
    }
    Result += ")";
    break;
  }
  case Type::StructTyID: {
    const StructType *STy = cast<StructType>(Ty);
    Result = "{ ";
    for (StructType::element_iterator I = STy->element_begin(),
           E = STy->element_end(); I != E; ++I) {
      if (I != STy->element_begin())
        Result += ", ";
      Result += calcTypeName(*I, TypeStack, TypeNames);
    }
    Result += " }";
    break;
  }
  case Type::PointerTyID:
    Result = calcTypeName(cast<PointerType>(Ty)->getElementType(), 
                          TypeStack, TypeNames) + "*";
    break;
  case Type::ArrayTyID: {
    const ArrayType *ATy = cast<ArrayType>(Ty);
    Result = "[" + utostr(ATy->getNumElements()) + " x ";
    Result += calcTypeName(ATy->getElementType(), TypeStack, TypeNames) + "]";
    break;
  }
  case Type::OpaqueTyID:
    Result = "opaque";
    break;
  default:
    Result = "<unrecognized-type>";
  }

  TypeStack.pop_back();       // Remove self from stack...
  return Result;
}


/// printTypeInt - The internal guts of printing out a type that has a
/// potentially named portion.
///
std::ostream &printTypeInt(std::ostream &Out, const Type *Ty,
                           std::map<const Type *, std::string> &TypeNames) {
  // Primitive types always print out their description, regardless of whether
  // they have been named or not.
  //
  if (Ty->isPrimitiveType() && !isa<OpaqueType>(Ty))
    return Out << Ty->getDescription();

  // Check to see if the type is named.
  std::map<const Type *, std::string>::iterator I = TypeNames.find(Ty);
  if (I != TypeNames.end()) return Out << I->second;

  // Otherwise we have a type that has not been named but is a derived type.
  // Carefully recurse the type hierarchy to print out any contained symbolic
  // names.
  //
  std::vector<const Type *> TypeStack;
  std::string TypeName = calcTypeName(Ty, TypeStack, TypeNames);
  TypeNames.insert(std::make_pair(Ty, TypeName));//Cache type name for later use
  return Out << TypeName;
}


void WriteConstantInt(std::ostream &Out, const Constant *CV, 
                      bool PrintName,
                      std::map<const Type *, std::string> &TypeTable,
                      SlotCalculator *Table) {
  if (const ConstantBool *CB = dyn_cast<ConstantBool>(CV)) {
    Out << (CB == ConstantBool::True ? "true" : "false");
  } else if (const ConstantSInt *CI = dyn_cast<ConstantSInt>(CV)) {
    Out << CI->getValue();
  } else if (const ConstantUInt *CI = dyn_cast<ConstantUInt>(CV)) {
    Out << CI->getValue();
  } else if (const ConstantFP *CFP = dyn_cast<ConstantFP>(CV)) {
    // We would like to output the FP constant value in exponential notation,
    // but we cannot do this if doing so will lose precision.  Check here to
    // make sure that we only output it in exponential format if we can parse
    // the value back and get the same value.
    //
    std::string StrVal = ftostr(CFP->getValue());

    // Check to make sure that the stringized number is not some string like
    // "Inf" or NaN, that atof will accept, but the lexer will not.  Check that
    // the string matches the "[-+]?[0-9]" regex.
    //
    if ((StrVal[0] >= '0' && StrVal[0] <= '9') ||
        ((StrVal[0] == '-' || StrVal[0] == '+') &&
         (StrVal[1] >= '0' && StrVal[1] <= '9')))
      // Reparse stringized version!
      if (atof(StrVal.c_str()) == CFP->getValue()) {
        Out << StrVal; return;
      }
    
    // Otherwise we could not reparse it to exactly the same value, so we must
    // output the string in hexadecimal format!
    //
    // Behave nicely in the face of C TBAA rules... see:
    // http://www.nullstone.com/htmls/category/aliastyp.htm
    //
    double Val = CFP->getValue();
    char *Ptr = (char*)&Val;
    assert(sizeof(double) == sizeof(uint64_t) && sizeof(double) == 8 &&
           "assuming that double is 64 bits!");
    Out << "0x" << utohexstr(*(uint64_t*)Ptr);

  } else if (isa<ConstantAggregateZero>(CV)) {
    Out << "zeroinitializer";
  } else if (const ConstantArray *CA = dyn_cast<ConstantArray>(CV)) {
    // As a special case, print the array as a string if it is an array of
    // ubytes or an array of sbytes with positive values.
    // 
    const Type *ETy = CA->getType()->getElementType();
    bool isString = (ETy == Type::SByteTy || ETy == Type::UByteTy);

    if (ETy == Type::SByteTy)
      for (unsigned i = 0; i < CA->getNumOperands(); ++i)
        if (cast<ConstantSInt>(CA->getOperand(i))->getValue() < 0) {
          isString = false;
          break;
        }

    if (isString) {
      Out << "c\"";
      for (unsigned i = 0; i < CA->getNumOperands(); ++i) {
        unsigned char C = cast<ConstantInt>(CA->getOperand(i))->getRawValue();
        
        if (isprint(C) && C != '"' && C != '\\') {
          if (C == '<') Out << "&lt;";
          else if (C == '>') Out << "&gt;";
          else Out << C;
        } else {
          Out << '\\'
              << (char) ((C/16  < 10) ? ( C/16 +'0') : ( C/16 -10+'A'))
              << (char)(((C&15) < 10) ? ((C&15)+'0') : ((C&15)-10+'A'));
        }
      }
      Out << "\"";

    } else {                // Cannot output in string format...
      Out << '[';
      if (CA->getNumOperands()) {
        Out << " ";
        printTypeInt(Out, ETy, TypeTable);
        WriteAsOperandInternal(Out, CA->getOperand(0),
                               PrintName, TypeTable, Table);
        for (unsigned i = 1, e = CA->getNumOperands(); i != e; ++i) {
          Out << ", ";
          printTypeInt(Out, ETy, TypeTable);
          WriteAsOperandInternal(Out, CA->getOperand(i), PrintName,
                                 TypeTable, Table);
        }
      }
      Out << " ]";
    }
  } else if (const ConstantStruct *CS = dyn_cast<ConstantStruct>(CV)) {
    Out << '{';
    if (CS->getNumOperands()) {
      Out << ' ';
      printTypeInt(Out, CS->getOperand(0)->getType(), TypeTable);

      WriteAsOperandInternal(Out, CS->getOperand(0),
                             PrintName, TypeTable, Table);

      for (unsigned i = 1; i < CS->getNumOperands(); i++) {
        Out << ", ";
        printTypeInt(Out, CS->getOperand(i)->getType(), TypeTable);

        WriteAsOperandInternal(Out, CS->getOperand(i),
                               PrintName, TypeTable, Table);
      }
    }

    Out << " }";
  } else if (isa<ConstantPointerNull>(CV)) {
    Out << "null";

  } else if (const ConstantPointerRef *PR = dyn_cast<ConstantPointerRef>(CV)) {
    WriteAsOperandInternal(Out, PR->getValue(), true, TypeTable, Table);

  } else if (const ConstantExpr *CE = dyn_cast<ConstantExpr>(CV)) {
    Out << CE->getOpcodeName() << " (";
    
    for (User::const_op_iterator OI=CE->op_begin(); OI != CE->op_end(); ++OI) {
      printTypeInt(Out, (*OI)->getType(), TypeTable);
      WriteAsOperandInternal(Out, *OI, PrintName, TypeTable, Table);
      if (OI+1 != CE->op_end())
        Out << ", ";
    }
    
    if (CE->getOpcode() == Instruction::Cast) {
      Out << " to ";
      printTypeInt(Out, CE->getType(), TypeTable);
    }
    Out << ')';

  } else {
    Out << "<placeholder or erroneous Constant>";
  }
}


/// WriteAsOperand - Write the name of the specified value out to the specified
/// ostream.  This can be useful when you just want to print int %reg126, not
/// the whole instruction that generated it.
///
void WriteAsOperandInternal(std::ostream &Out, const Value *V, 
                            bool PrintName,
                            std::map<const Type*, std::string> &TypeTable,
                            SlotCalculator *Table) {
  Out << ' ';
  if (PrintName && V->hasName()) {
    Out << getLLVMName(V->getName());
  } else {
    if (const Constant *CV = dyn_cast<Constant>(V)) {
      WriteConstantInt(Out, CV, PrintName, TypeTable, Table);
    } else {
      int Slot;
      if (Table) {
	Slot = Table->getSlot(V);
      } else {
        if (const Type *Ty = dyn_cast<Type>(V)) {
          Out << Ty->getDescription();
          return;
        }

        Table = createSlotCalculator(V);
        if (Table == 0) { Out << "BAD VALUE TYPE!"; return; }

	Slot = Table->getSlot(V);
	delete Table;
      }
      if (Slot >= 0)  Out << '%' << Slot;
      else if (PrintName)
        if (V->hasName())
          Out << "<badref: " << getLLVMName(V->getName()) << ">";
        else
          Out << "<badref>";     // Not embedded into a location?
    }
  }
}


