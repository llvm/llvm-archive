#ifndef GRAPHDRAWER_H
#define GRAPHDRAWER_H

#include "wx/wx.h"

//===----------------------------------------------------------------------===//

// GraphDrawer abstract class

class GraphDrawer {
  wxImage *graphImage;
  virtual wxImage *drawGraphImage () = 0;

 public:
  GraphDrawer () : graphImage (0) { }
  wxImage *getGraphImage () {
    if (!graphImage)
      graphImage = drawGraphImage ();
    // FIXME -- should make sure that it worked
    return graphImage;
  }
};

#endif // GRAPHDRAWER_H
