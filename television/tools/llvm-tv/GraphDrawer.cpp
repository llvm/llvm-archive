#include "wx/image.h"
#include "GraphDrawer.h"
#include "TVTreeItem.h"
#include "Support/FileUtilities.h"
#include <unistd.h>
#include <iostream>
using namespace llvm;

//===----------------------------------------------------------------------===//

// GraphDrawer shared implementation

extern void FatalErrorBox (const std::string msg);

wxImage *GraphDrawer::buildwxImageFromDotFile (const std::string filename) {
  if (!FileOpenable (filename))
    FatalErrorBox ("buildwxImageFromDotFile() got passed a bogus filename: '"
                   + filename + "'");

  // We have a dot file, turn it into something we can load.
  std::string cmd = "dot -Tpng " + filename + " -o image.png";
  if (system (cmd.c_str ()) != 0)
    FatalErrorBox ("buildwxImageFromDotFile() failed when calling dot");
  unlink (filename.c_str ());

  wxImage *img = new wxImage;
  if (!img->LoadFile ("image.png"))
    FatalErrorBox("buildwxImageFromDotFile() produced a non-loadable PNG file");

  unlink ("image.png");
  return img;
}

void GraphDrawer::displayItem (TVTreeItemData *item) {
  wxImage *graphImage = item->graphOn (this);
  if (!graphImage) {
    // If we're drawing into a window, don't leave behind an embarrassing
    // empty window AND an error message. (But don't destroy the window,
    // because TVApp will get around to destroying it later.)
    if (wxFrame *frame = dynamic_cast<wxFrame *>(myPictureCanvas->GetParent ()))
      frame->Show (false);
    std::string errMsg = "Sorry, you can't draw that kind of graph on "
                         + item->getTitle() + ".";
    wxMessageBox (errMsg.c_str (), "Error");
    return;
  }
  myPictureCanvas->SetImage (graphImage);
}
