#include "CallGraphDrawer.h"
#include "GraphPrinters.h"
#include "llvm/PassManager.h"
#include <unistd.h>
using namespace llvm;

//===----------------------------------------------------------------------===//

// CallGraphDrawer implementation

void CallGraphDrawer::drawGraphImage () {
  PassManager PM;
  PM.add (createCallGraphPrinterPass ());
  PM.run (*module);
  // Ok, it made us a callgraph.dot file, turn it into something we can load
  system ("dot -Tpng callgraph.dot > callgraph.png");
  unlink ("callgraph.dot");
  graphImage = new wxImage;
  if (!graphImage->LoadFile ("callgraph.png")) {
    std::cerr << "drawGraphImage(): wxImage::LoadFile returned false\n";
    return;
  }
  unlink ("callgraph.png");
}
