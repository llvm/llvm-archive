#include "PictureCanvas.h"

//===----------------------------------------------------------------------===//

// PictureCanvas implementation

void PictureCanvas::OnDraw (wxDC &aDC) {
  if (!myBitmap) return;
  aDC.DrawBitmap (*myBitmap, 0, 0, false);
}

void PictureCanvas::setupBitmap () {
  myBitmap = new wxBitmap (*myImage);
  SetVirtualSize (myImage->GetWidth (), myImage->GetHeight ());
  SetScrollRate (1, 1);
  myParent->SetSizeHints (-1, -1, myImage->GetWidth (), myImage->GetHeight ());
  myParent->Refresh ();
}

void PictureCanvas::loadPicture (const wxString filename) {
  if (!myImage)
    myImage = new wxImage;
  if (!myImage->LoadFile (filename)) {
    myParent->Close ();
    return;
  }
  setupBitmap ();
}

BEGIN_EVENT_TABLE (PictureCanvas, wxScrolledWindow)
END_EVENT_TABLE ()
