#ifndef DSAGRAPHDRAWER_H
#define DSAGRAPHDRAWER_H

#include "GraphDrawer.h"
#include <wx/wx.h>
#include <string>

namespace llvm {
  class Function;
  class Module;
}

//===----------------------------------------------------------------------===//

// GraphDrawer abstract class

class BUGraphDrawer : public GraphDrawer {
  llvm::Function *F;
  llvm::Module *M;
  wxImage *drawGraphImage ();
 public:
  BUGraphDrawer (llvm::Module *_M) : GraphDrawer(), F(0), M(_M) {}
  BUGraphDrawer (llvm::Function *_F) : GraphDrawer(), F(_F), M(0) {}
};

class TDGraphDrawer : public GraphDrawer {
  llvm::Module *M;
  wxImage *drawGraphImage ();
 public:
  TDGraphDrawer (llvm::Module *_M) : GraphDrawer (), M (_M) { }

};

class LocalGraphDrawer : public GraphDrawer {
  llvm::Module *M;
  wxImage *drawGraphImage ();
 public:
  LocalGraphDrawer (llvm::Module *_M) : GraphDrawer (), M (_M) { }

};

#endif // GRAPHDRAWER_H
