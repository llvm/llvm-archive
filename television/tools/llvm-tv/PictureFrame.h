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
  void setupAppearance () {
    SetSize (wxRect (200, 200, 300, 300));
    Show (TRUE);
  }
 public:
  PictureFrame (TVApplication *app, const char *windowTitle = "")
    : wxFrame (NULL, -1, windowTitle), myApp (app) {
    setupAppearance ();
  }
  bool OnClose (wxCloseEvent &event);
  DECLARE_EVENT_TABLE ()
};

#endif // PICTUREFRAME_H
