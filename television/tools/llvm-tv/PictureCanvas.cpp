#include "wx/image.h"
#include "PictureCanvas.h"

//===----------------------------------------------------------------------===//

// PictureCanvas implementation

void PictureCanvas::OnDraw (wxDC &aDC) {
  if (!myBitmap) return;
  aDC.DrawBitmap (*myBitmap, 0, 0, false);
}

void PictureCanvas::imageChanged () {
  if (myBitmap)
    delete myBitmap; // Don't leak memory.

  if (!myImage)
    return; // Maybe there isn't a new image.

  myBitmap = new wxBitmap (*myImage);
  SetVirtualSize (myImage->GetWidth (), myImage->GetHeight ());
  SetScrollRate (1, 1);
  GetParent ()->SetSizeHints (-1, -1, myImage->GetWidth (),
                             myImage->GetHeight ());
  GetParent ()->Refresh ();
}
