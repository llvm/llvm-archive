#ifndef PICTUREFRAME_H
#define PICTUREFRAME_H

#include "wx/wx.h"
#include "PictureCanvas.h"

//===----------------------------------------------------------------------===//

// PictureFrame interface

class TVApplication;
/// PictureFrame - just a window that contains a PictureCanvas.
///
class PictureFrame : public wxFrame {
  TVApplication *myApp;
  PictureCanvas myPictureCanvas;
  void setupAppearance () {
    SetSize (wxRect (200, 200, 300, 300));
    Show (TRUE);
  }
 public:
  PictureFrame::PictureFrame (TVApplication *app, const char *filename)
    : wxFrame (NULL, -1, filename), myApp (app),
      myPictureCanvas (this, filename) {
    setupAppearance ();
  }
  PictureFrame::PictureFrame (TVApplication *app, const char *filename,
                              wxImage *image)
    : wxFrame (NULL, -1, filename), myApp (app),
      myPictureCanvas (this, image) {
    setupAppearance ();
  }
  bool OnClose (wxCloseEvent &event);
  DECLARE_EVENT_TABLE ()
};

#endif // PICTUREFRAME_H
