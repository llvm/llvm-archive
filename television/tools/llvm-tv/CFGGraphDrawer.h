#ifndef CFGGRAPHDRAWER_H
#define CFGGRAPHDRAWER_H

#include "GraphDrawer.h"
#include "TVSnapshot.h"

//===----------------------------------------------------------------------===//

// CFGGraphDrawer interface

class CFGGraphDrawer : public GraphDrawer {
  Function *fn;
  wxImage *drawGraphImage ();
 public:
  CFGGraphDrawer (Function *_fn) : fn (_fn) { }
};

#endif // CFGGRAPHDRAWER_H
