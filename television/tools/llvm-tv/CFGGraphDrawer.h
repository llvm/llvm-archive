//===-- CFGGraphDrawer.h - Creates an image representing a CFG ---*- C++ -*-==//
//
// Class for drawing CFG of a Function
//
//===----------------------------------------------------------------------===//

#ifndef CFGGRAPHDRAWER_H
#define CFGGRAPHDRAWER_H

#include "GraphDrawer.h"

namespace llvm {
  class Function;
}

//===----------------------------------------------------------------------===//

// CFGGraphDrawer interface

class CFGGraphDrawer : public GraphDrawer {
 public:
  wxImage *drawFunctionGraph (llvm::Function *fn);
  CFGGraphDrawer (wxWindow *parent) : GraphDrawer (parent) { }
};

#endif // CFGGRAPHDRAWER_H
