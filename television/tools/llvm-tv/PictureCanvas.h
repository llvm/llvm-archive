#ifndef PICTURECANVAS_H
#define PICTURECANVAS_H

#include "wx/wx.h"
#include <iostream>
#include <functional>

//===----------------------------------------------------------------------===//

// PictureCanvas interface

class PictureCanvas : public wxScrolledWindow {
  wxImage *myImage;
  wxBitmap *myBitmap;

  void loadPicture (const wxString filename);
  void setupBitmap ();
 public:
  PictureCanvas ()
    : wxScrolledWindow (0), myImage (0), myBitmap (0) {
  }
  PictureCanvas (wxFrame *parent, const wxString filename)
    : wxScrolledWindow (parent), myImage (0), myBitmap (0) {
    loadPicture (filename);
  }
  PictureCanvas (wxFrame *parent, wxImage *image)
    : wxScrolledWindow (parent), myImage (image), myBitmap (0) {
    setupBitmap ();
  }
  void OnDraw (wxDC &aDC);
  DECLARE_EVENT_TABLE ()
};

#endif // PICTURECANVAS_H
