#ifndef CALLGRAPHDRAWER_H
#define CALLGRAPHDRAWER_H

#include "GraphDrawer.h"
#include "TVSnapshot.h"

//===----------------------------------------------------------------------===//

// CallGraphDrawer interface

class CallGraphDrawer : public GraphDrawer {
  Module *module;
  wxImage *drawGraphImage ();
 public:
  CallGraphDrawer (Module *_mod) : GraphDrawer (), module (_mod) { }
};

#endif // CALLGRAPHDRAWER_H
