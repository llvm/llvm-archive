#include "wx/image.h"
#include "CFGGraphDrawer.h"
#include "llvm/ModuleProvider.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/PassManager.h"
#include <unistd.h>
using namespace llvm;

//===----------------------------------------------------------------------===//

// CFGGraphDrawer implementation

void CFGGraphDrawer::drawGraphImage () {
  ExistingModuleProvider MP (fn->getParent ());
  FunctionPassManager PM (&MP);
  PM.add (createCFGOnlyPrinterPass ());
  PM.run (*fn);
  // Ok, it made us a CFG dot graph file, turn it into something we can load
  std::string filename = "cfg." + fn->getName() + ".dot";
  std::string cmd = "dot -Tpng " + filename + " > cfg.png";
  system (cmd.c_str ());
  unlink (filename.c_str ());
  graphImage = new wxImage;
  if (!graphImage->LoadFile ("cfg.png")) {
     std::cerr << "drawGraphImage(): wxImage::LoadFile returned false\n";
     return;
  }
  unlink ("cfg.png");
}
