#ifndef CALLGRAPHDRAWER_H
#define CALLGRAPHDRAWER_H

#include "wx/wx.h"
#include "TVSnapshot.h"

//===----------------------------------------------------------------------===//

// CallGraphDrawer interface

class CallGraphDrawer {
  Module *module;
  wxImage *graphImage;
 public:
  CallGraphDrawer (Module *_mod) : module (_mod), graphImage (0) { }
  void drawGraphImage ();
  wxImage *getGraphImage () {
    if (!graphImage)
      drawGraphImage ();
    return graphImage;
  }
};

#endif // CALLGRAPHDRAWER_H
