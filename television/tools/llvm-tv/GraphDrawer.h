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
  virtual ~GraphDrawer () { delete myPictureCanvas; }
  wxWindow *getWindow () { return myPictureCanvas; }
  void displayItem (TVTreeItemData *item);

  /// drawModuleGraph, drawFunctionGraph - Subclasses should override
  /// these methods to return images if they support visualizing the
  /// corresponding kind of object.
  ///
  virtual wxImage *drawModuleGraph (llvm::Module *M) { return 0; }
  virtual wxImage *drawFunctionGraph (llvm::Function *F) { return 0; }

  /// getDisplayTitle - What should it be called when a subclass displays a
  /// graph of the given item? If you want the graph to have a descriptive
  /// title, like "Spiffy graph of foo()", override this method.
  ///
  static std::string getDisplayTitle (TVTreeItemData *item);
};

#endif // GRAPHDRAWER_H
