#include "DSAGraphDrawer.h"
#include "GraphPrinters.h"
#include "llvm/Function.h"
#include "llvm/ModuleProvider.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Target/TargetData.h"
using namespace llvm;

//===----------------------------------------------------------------------===//

// DSGraphDrawer implementation
wxImage *DSGraphDrawer::drawGraphImage() {
  if (M) {
    PassManager PM;
    PM.add(new TargetData("llvm-tv", M));
    PM.add(getModulePass());
    PM.run(*M);
    return buildwxImageFromDotFile(getFilename(M));
  } else if (F) {
    ModuleProvider *MP = new ExistingModuleProvider(F->getParent());
    FunctionPassManager PM(MP);
    PM.add(new TargetData("llvm-tv", F->getParent()));
    PM.add(getFunctionPass());
    PM.run(*F);
    MP->releaseModule(); // Don't delete it when you go away, says I
    delete MP;
    return buildwxImageFromDotFile(getFilename(F));
  } else {
    std::cerr << "BUDS: both Function and Module are null!\n";
    return 0;
  }
}

//===----------------------------------------------------------------------===//

// BUGraphDrawer implementation
FunctionPass *BUGraphDrawer::getFunctionPass() {
  return createBUDSFunctionPrinterPass();
}

Pass *BUGraphDrawer::getModulePass() {
  return createBUDSModulePrinterPass();
}

std::string BUGraphDrawer::getFilename(Function *F) {
  return "buds." + F->getName() + ".dot";
}

std::string BUGraphDrawer::getFilename(Module *M) {
  return "buds.dot";
}

//===----------------------------------------------------------------------===//

// TDGraphDrawer implementation
FunctionPass *TDGraphDrawer::getFunctionPass() {
  return createTDDSFunctionPrinterPass();
}

Pass *TDGraphDrawer::getModulePass() {
  return createTDDSModulePrinterPass();
}

std::string TDGraphDrawer::getFilename(Function *F) {
  return "tdds." + F->getName() + ".dot";
}

std::string TDGraphDrawer::getFilename(Module *M) {
  return "tdds.dot";
}

//===----------------------------------------------------------------------===//

// LocalGraphDrawer implementation
FunctionPass *LocalGraphDrawer::getFunctionPass() {
  return createLocalDSFunctionPrinterPass();
}

Pass *LocalGraphDrawer::getModulePass() {
  return createLocalDSModulePrinterPass();
}

std::string LocalGraphDrawer::getFilename(Function *F) {
  return "localds." + F->getName() + ".dot";
}

std::string LocalGraphDrawer::getFilename(Module *M) {
  return "localds.dot";
}
