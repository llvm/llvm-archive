//===-- PictureFrame.h - Window containing PictureCanvas ---------*- C++ -*-==//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// Window container for PictureCanvas objects.
//
//===----------------------------------------------------------------------===//

#ifndef PICTUREFRAME_H
#define PICTUREFRAME_H

#include "PictureCanvas.h"
#include "wx/wx.h"

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
