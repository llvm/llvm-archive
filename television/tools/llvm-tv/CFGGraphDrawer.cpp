#include "CFGGraphDrawer.h"
#include "llvm/ModuleProvider.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/PassManager.h"
using namespace llvm;

//===----------------------------------------------------------------------===//

// CFGGraphDrawer implementation

wxImage *CFGGraphDrawer::drawGraphImage () {
  ExistingModuleProvider MP (fn->getParent ());
  FunctionPassManager PM (&MP);
  PM.add (createCFGOnlyPrinterPass ());
  PM.run (*fn);
  return buildwxImageFromDotFile ("cfg." + fn->getName() + ".dot");
}
