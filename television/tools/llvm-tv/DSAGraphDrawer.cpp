#include "DSAGraphDrawer.h"
#include "GraphPrinters.h"
#include "llvm/Function.h"
#include "llvm/ModuleProvider.h"
#include "llvm/PassManager.h"
#include "llvm/Target/TargetData.h"
using namespace llvm;

//===----------------------------------------------------------------------===//

// BUGraphDrawer implementation
wxImage *BUGraphDrawer::drawGraphImage() {
  if (M) {
    PassManager PM;
    PM.add(new TargetData("llvm-tv", M));
    PM.add(createBUDSModulePrinterPass());
    PM.run(*M);
    return buildwxImageFromDotFile("buds.dot");
  } else if (F) {
    ModuleProvider *MP = new ExistingModuleProvider(F->getParent());
    FunctionPassManager PM(MP);
    PM.add(new TargetData("llvm-tv", F->getParent()));
    PM.add(createBUDSFunctionPrinterPass());
    PM.run(*F);
    MP->releaseModule(); // Don't delete it when you go away, says I
    delete MP;
    return buildwxImageFromDotFile("buds." + F->getName() + ".dot");
  } else {
    std::cerr << "Both Function and Module are null!\n";
    return 0;
  }
}

//===----------------------------------------------------------------------===//

// TDGraphDrawer implementation
wxImage *TDGraphDrawer::drawGraphImage() {
  PassManager PM;
  PM.add(createTDDSPrinterPass());
  PM.run(*M);
  return buildwxImageFromDotFile ("tdds.dot");
}

//===----------------------------------------------------------------------===//

// LocalGraphDrawer implementation
wxImage *LocalGraphDrawer::drawGraphImage() {
  PassManager PM;
  PM.add(createLocalDSPrinterPass());
  PM.run(*M);
  return buildwxImageFromDotFile ("localds.dot");
}
