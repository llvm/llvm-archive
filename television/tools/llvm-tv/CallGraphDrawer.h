#ifndef CALLGRAPHDRAWER_H
#define CALLGRAPHDRAWER_H

#include "GraphDrawer.h"

namespace llvm {
  class Module;
};

//===----------------------------------------------------------------------===//

// CallGraphDrawer interface

class CallGraphDrawer : public GraphDrawer {
public:
  wxImage *drawModuleGraph (llvm::Module *M);
  CallGraphDrawer (wxWindow *parent) : GraphDrawer (parent) { }
  static std::string getDisplayTitle (TVTreeItemData *item);
};

#endif // CALLGRAPHDRAWER_H
