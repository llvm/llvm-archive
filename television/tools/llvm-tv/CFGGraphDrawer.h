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
  llvm::Function *fn;
  wxImage *drawGraphImage ();
 public:
  CFGGraphDrawer (llvm::Function *_fn) : fn (_fn) { }
};

#endif // CFGGRAPHDRAWER_H
