#ifndef CFGGRAPHDRAWER_H
#define CFGGRAPHDRAWER_H

#include "wx/wx.h"
#include "TVSnapshot.h"

//===----------------------------------------------------------------------===//

// CFGGraphDrawer interface

class CFGGraphDrawer {
  Function *fn;
  wxImage *graphImage;
 public:
  CFGGraphDrawer (Function *_fn) : fn (_fn), graphImage (0) { }
  void drawGraphImage ();
  wxImage *getGraphImage () {
    if (!graphImage)
      drawGraphImage ();
    return graphImage;
  }
};

#endif // CFGGRAPHDRAWER_H
