//===-- GraphDrawer.h - Graph viewing abstract class -------------*- C++ -*-==//
//
// Superclass for graph-drawing classes.
//
//===----------------------------------------------------------------------===//

#ifndef GRAPHDRAWER_H
#define GRAPHDRAWER_H

#include "PictureCanvas.h"
#include "ItemDisplayer.h"
#include "wx/wx.h"
#include <iostream>

namespace llvm {
  class Module;
  class Function;
};

class TVTreeItemData;

//===----------------------------------------------------------------------===//

// GraphDrawer abstract class

class GraphDrawer : public ItemDisplayer {
  PictureCanvas *myPictureCanvas;

protected:
  static wxImage *buildwxImageFromDotFile (const std::string filename);

public:
  GraphDrawer (wxWindow *parent)
    : myPictureCanvas (new PictureCanvas (parent)) { }
  virtual ~GraphDrawer () { std::cerr << "GraphDrawer is being destroyed\n"; delete myPictureCanvas; }
  wxWindow *getWindow () { return myPictureCanvas; }
  void displayItem (TVTreeItemData *item);

  /// Subclasses should override these methods to return images if they
  /// support visualizing the corresponding kind of object.
  ///
  virtual wxImage *drawModuleGraph (llvm::Module *M) { return 0; }
  virtual wxImage *drawFunctionGraph (llvm::Function *F) { return 0; }
};

#endif // GRAPHDRAWER_H
