//===-- GraphDrawer.h - Graph viewing abstract class -------------*- C++ -*-==//
//
// Superclass for graph-drawing classes.
//
//===----------------------------------------------------------------------===//

#ifndef GRAPHDRAWER_H
#define GRAPHDRAWER_H

#include "ItemDisplayer.h"
#include "PictureCanvas.h"
#include "wx/wx.h"
#include <iostream>

namespace llvm {
  class Module;
  class Function;
};

class TVTreeItemData;

/// GraphDrawer abstract class
///
class GraphDrawer : public ItemDisplayer {
  PictureCanvas *myPictureCanvas;

protected:
  static wxImage *buildwxImageFromDotFile (const std::string filename);
  GraphDrawer (wxWindow *parent)
    : myPictureCanvas (new PictureCanvas (parent)) { }

public:
  virtual ~GraphDrawer () { delete myPictureCanvas; }
  wxWindow *getWindow () { return myPictureCanvas; }
  void displayItem (TVTreeItemData *item);

  /// drawModuleGraph, drawFunctionGraph - Subclasses should override
  /// these methods to return images if they support visualizing the
  /// corresponding kind of object.
  ///
  virtual wxImage *drawModuleGraph (llvm::Module *M) { return 0; }
  virtual wxImage *drawFunctionGraph (llvm::Function *F) { return 0; }
};

#endif // GRAPHDRAWER_H
