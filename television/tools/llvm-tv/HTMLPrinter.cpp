#include "HTMLPrinter.h"
#include "HTMLPrinterUtils.h"
#include "HTMLMarkup.h"
#include "llvm/BasicBlock.h"
#include "llvm/Constant.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/SymbolTable.h"
#include "llvm/Type.h"
#include "llvm/Analysis/SlotCalculator.h"
#include <ostream>
using namespace llvm;


void HTMLPrinter::writeOperand(const Value *Operand, bool PrintType, 
                               bool PrintName) {
  if (PrintType) { os << ' '; printType(Operand->getType()); }
  WriteAsOperandInternal(os, Operand, PrintName, TypeNames, Table);
}

// printType - Go to extreme measures to attempt to print out a short,
// symbolic version of a type name.
//
void HTMLPrinter::printType(const Type *Ty) {
  html.typeBegin();
  printTypeInt(os, Ty, TypeNames);
  html.typeEnd();
}

void HTMLPrinter::printFunctionHeader(Function &F) {
  // print out function return type, name, and arguments
  if (F.isExternal())
    html.printKeyword("declare ");

  html.typeBegin();
  cw << F.getReturnType();
  html.typeEnd();

  os << " <tt>" << F.getName() << '(';
  for (Function::aiterator arg = F.abegin(), ae = F.aend(); arg != ae; ++arg){
    html.typeBegin();
    cw << arg->getType();
    html.typeEnd();
    if (arg->getName() != "")
      os << ' ' << arg->getName();
    Function::aiterator next = arg;
    ++next;
    if (next != F.aend()) 
      os << ", ";
  }

  if (F.getFunctionType()->isVarArg()) {
    if (F.getFunctionType()->getNumParams()) os << ", ";
    os << "...";  // Output varargs portion of signature!
  }

  os << ")</tt>";
}

void HTMLPrinter::visit(Module &M) {
  html.printHeader();

  Table = new SlotCalculator(&M, true);
  // If the module has a symbol table, take all global types and stuff their
  // names into the TypeNames map.
  fillTypeNameTable(&M, TypeNames);

  // Display target size (bits), endianness types
  html.printKeyword("target endian");
  os << " = ";
  html.printKeyword((M.getEndianness() ? "little" : "big"));
  os << "<br>";
  html.printKeyword("target pointersize");
  os << " = " << (M.getPointerSize() ? "32" : "64");
  os << "<br><br>";

  // Display globals
  for (Module::giterator G = M.gbegin(), Ge = M.gend(); G != Ge; ++G)
    visit(*G);
  
  os << "<br>";

  // Display function signatures
  for (Module::iterator F = M.begin(), Fe = M.end(); F != Fe; ++F) {
    printFunctionHeader(*F);
    os << "<br>";
  }

  html.printFooter();
}

void HTMLPrinter::visit(GlobalVariable &GV) {
  if (GV.hasName()) 
    os << getLLVMName(GV.getName()) << " = ";

  if (!GV.hasInitializer()) 
    html.printKeyword("external ");
  else
    switch (GV.getLinkage()) {
    case GlobalValue::InternalLinkage:  html.printKeyword("internal "); break;
    case GlobalValue::LinkOnceLinkage:  html.printKeyword("linkonce "); break;
    case GlobalValue::WeakLinkage:      html.printKeyword("weak "); break;
    case GlobalValue::AppendingLinkage: html.printKeyword("appending "); break;
    case GlobalValue::ExternalLinkage: break;
    }

  html.printKeyword((GV.isConstant() ? "constant " : "global "));
  printType(GV.getType()->getElementType());

  if (GV.hasInitializer())
    writeOperand(GV.getInitializer(), false, false);

  //printInfoComment(GV);
  os << "<br>";
}

void HTMLPrinter::visit(Function &F) {
  html.printHeader();

  Table = new SlotCalculator(F.getParent(), true);
  // If the module has a symbol table, take all global types and stuff their
  // names into the TypeNames map.
  fillTypeNameTable(F.getParent(), TypeNames);
  printFunctionHeader(F);

  os << "<tt>";
  if (!F.isExternal())
    os << " {<br>";

  visit(F.begin(), F.end());

  if (!F.isExternal())
    os << '}';
  os << "</tt><br>";

  html.printFooter();
}

void HTMLPrinter::visitBasicBlock(BasicBlock &BB) {
  html.printBB(BB.getName());
}

void HTMLPrinter::printName(Instruction &I) {
  os << " &nbsp; &nbsp;";

  // Print out name if it exists...
  if (I.hasName())
    os << getLLVMName(I.getName()) << " = ";

  // If this is a volatile load or store, print out the volatile marker
  if ((isa<LoadInst>(I)  && cast<LoadInst>(I).isVolatile()) ||
      (isa<StoreInst>(I) && cast<StoreInst>(I).isVolatile()))
      html.printKeyword("volatile ");

  // Print out the opcode...
  html.printKeyword(I.getOpcodeName());
  os << ' ';
}

void HTMLPrinter::printOperands(Instruction &I) {
  const Value *Operand = I.getNumOperands() ? I.getOperand(0) : 0;
  if (!Operand) return;
  
  // PrintAllTypes - Instructions who have operands of all the same type 
  // omit the type from all but the first operand.  If the instruction has
  // different type operands (for example br), then they are all printed.
  bool PrintAllTypes = false;
  const Type *TheType = Operand->getType();

  // Shift Left & Right print both types even for Ubyte LHS, and select prints
  // types even if all operands are bools.
  if (isa<ShiftInst>(I) || isa<SelectInst>(I)) {
    PrintAllTypes = true;
  } else {
    for (unsigned i = 1, E = I.getNumOperands(); i != E; ++i) {
      Operand = I.getOperand(i);
      if (Operand->getType() != TheType) {
        PrintAllTypes = true;    // We have differing types!  Print them all!
        break;
      }
    }
  }
    
  if (!PrintAllTypes) {
    os << ' ';
    printType(TheType);
  }

  for (unsigned i = 0, E = I.getNumOperands(); i != E; ++i) {
    if (i) os << ',';
    writeOperand(I.getOperand(i), PrintAllTypes);
  }
}

//===----------------------------------------------------------------------===//
// Instruction visit* methods

void HTMLPrinter::visitInstruction(Instruction &I) {
  printName(I);
  printOperands(I);
  os << "<br>";
}

void HTMLPrinter::visitSwitchInst(SwitchInst &SI) {
  const Value *Operand = SI.getNumOperands() ? SI.getOperand(0) : 0;
  printName(SI);

  // Special case switch statement to get formatting nice and correct...
  writeOperand(Operand         , true); os << ',';
  writeOperand(SI.getOperand(1), true); os << " [";

  for (unsigned op = 2, Eop = SI.getNumOperands(); op < Eop; op += 2) {
    os << "<br> &nbsp; &nbsp; ";
    writeOperand(SI.getOperand(op  ), true); os << ",";
    writeOperand(SI.getOperand(op+1), true);
  }
  os << "<br> &nbsp; &nbsp; ]<br>";
}

void HTMLPrinter::visitCallInst(CallInst &CI) {
  const Value    *Operand = CI.getNumOperands() ? CI.getOperand(0) : 0;
  const PointerType  *PTy = cast<PointerType>(Operand->getType());
  const FunctionType *FTy = cast<FunctionType>(PTy->getElementType());
  const Type       *RetTy = FTy->getReturnType();
  printName(CI);

  // If possible, print out the short form of the call instruction.  We can
  // only do this if the first argument is a pointer to a nonvararg function,
  // and if the return type is not a pointer to a function.
  //
  if (!FTy->isVarArg() &&
      (!isa<PointerType>(RetTy) || 
       !isa<FunctionType>(cast<PointerType>(RetTy)->getElementType()))) {
    os << ' '; printType(RetTy);
    writeOperand(Operand, false);
  } else {
    writeOperand(Operand, true);
  }
  os << '(';
  if (CI.getNumOperands() > 1) writeOperand(CI.getOperand(1), true);
  for (unsigned op = 2, Eop = CI.getNumOperands(); op < Eop; ++op) {
    os << ',';
    writeOperand(CI.getOperand(op), true);
  }

  os << " )<br>";
}

void HTMLPrinter::visitPHINode(PHINode &I) {
  printName(I);

  printType(I.getType());
  os << ' ';

  for (unsigned op = 0, Eop = I.getNumOperands(); op < Eop; op += 2) {
    if (op) os << ", ";
    os << '[';
    writeOperand(I.getOperand(op  ), false); os << ',';
    writeOperand(I.getOperand(op+1), false); os << " ]";
  }

  os << "<br>";
}

void HTMLPrinter::visitInvokeInst(InvokeInst &I) {
  printName(I);
  const Value    *Operand = I.getNumOperands() ? I.getOperand(0) : 0;
  const PointerType  *PTy = cast<PointerType>(Operand->getType());
  const FunctionType *FTy = cast<FunctionType>(PTy->getElementType());
  const Type       *RetTy = FTy->getReturnType();

  // If possible, print out the short form of the invoke instruction. We can
  // only do this if the first argument is a pointer to a nonvararg function,
  // and if the return type is not a pointer to a function.
  //
  if (!FTy->isVarArg() &&
      (!isa<PointerType>(RetTy) || 
       !isa<FunctionType>(cast<PointerType>(RetTy)->getElementType()))) {
    os << ' '; printType(RetTy);
    writeOperand(Operand, false);
  } else {
    writeOperand(Operand, true);
  }

  os << '(';
  if (I.getNumOperands() > 3) writeOperand(I.getOperand(3), true);
  for (unsigned op = 4, Eop = I.getNumOperands(); op < Eop; ++op) {
    os << ",";
    writeOperand(I.getOperand(op), true);
  }

  os << " )<br> &nbsp; &nbsp; &nbsp;";
  html.printKeyword("to");
  writeOperand(I.getNormalDest(), true);
  html.printKeyword(" unwind");
  writeOperand(I.getUnwindDest(), true);
  os << "<br>";
}

void HTMLPrinter::visitAllocationInst(AllocationInst &I) {
  printName(I);
  os << ' ';
  printType(I.getType()->getElementType());
  if (I.isArrayAllocation()) {
    os << ",";
    writeOperand(I.getArraySize(), true);
  }
  os << "<br>";
}

void HTMLPrinter::visitCastInst(CastInst &I) {
  printName(I);
  const Value *Operand = I.getNumOperands() ? I.getOperand(0) : 0;
  if (Operand) writeOperand(Operand, true);   // Work with broken code
  os << " to ";
  printType(I.getType());
  os << "<br>";
}

void HTMLPrinter::visitVANextInst(VANextInst &I) {
  printName(I);
  const Value *Operand = I.getNumOperands() ? I.getOperand(0) : 0;
  if (Operand) writeOperand(Operand, true);   // Work with broken code
  os << ", ";
  printType(I.getArgType());
  os << "<br>";
}

void HTMLPrinter::visitVAArgInst(VAArgInst &I) {
  printName(I);
  const Value *Operand = I.getNumOperands() ? I.getOperand(0) : 0;
  if (Operand) writeOperand(Operand, true);   // Work with broken code
  os << ", ";
  printType(I.getType());
  os << "<br>";
}
