#include "wx/image.h"
#include "GraphDrawer.h"
#include "Support/FileUtilities.h"
#include <unistd.h>
#include <iostream>
using namespace llvm;

//===----------------------------------------------------------------------===//

// GraphDrawer shared implementation

extern void FatalErrorBox (const std::string msg);

wxImage *GraphDrawer::getGraphImage() {
  if (!graphImage) {
    graphImage = drawGraphImage ();
    // Make sure that it worked
    if (!graphImage)
      FatalErrorBox ("drawGraphImage failed to produce an image\n");
  }
  return graphImage;
}

wxImage *GraphDrawer::buildwxImageFromDotFile (const std::string filename) {
  if (!FileOpenable (filename))
    FatalErrorBox ("buildwxImageFromDotFile got passed a bogus filename: '"
                   + filename + "'");

  // We have a dot file, turn it into something we can load.
  std::string cmd = "dot -Tpng " + filename + " -o image.png";
  if (system (cmd.c_str ()) != 0)
    FatalErrorBox ("buildwxImageFromDotFile failed when calling dot");
  unlink (filename.c_str ());

  wxImage *img = new wxImage;
  if (!img->LoadFile ("image.png"))
    FatalErrorBox ("buildwxImageFromDotFile produced a non-loadable PNG file");

  unlink ("image.png");
  return img;
}

