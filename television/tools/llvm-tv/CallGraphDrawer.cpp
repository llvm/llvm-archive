#include "CallGraphDrawer.h"
#include "GraphPrinters.h"
#include "llvm/PassManager.h"
using namespace llvm;

//===----------------------------------------------------------------------===//

// CallGraphDrawer implementation

wxImage *CallGraphDrawer::drawModuleGraph (Module *module) {
  PassManager PM;
  PM.add (createCallGraphPrinterPass ());
  PM.run (*module);
  return buildwxImageFromDotFile ("callgraph.dot");
}
