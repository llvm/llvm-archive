//===-- GraphDrawer.h - Graph viewing abstract class -------------*- C++ -*-==//
//
// Superclass for graph-drawing classes.
//
//===----------------------------------------------------------------------===//

#ifndef GRAPHDRAWER_H
#define GRAPHDRAWER_H

#include <string>

class wxImage;

//===----------------------------------------------------------------------===//

// GraphDrawer abstract class

class GraphDrawer {
  wxImage *graphImage;
  virtual wxImage *drawGraphImage () = 0;

 protected:
  static wxImage *buildwxImageFromDotFile (const std::string filename);

 public:
  GraphDrawer () : graphImage (0) { }
  wxImage *getGraphImage ();
};

#endif // GRAPHDRAWER_H
