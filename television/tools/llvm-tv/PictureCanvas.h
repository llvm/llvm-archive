//===-- PictureCanvas.h - Host of graph images -------------------*- C++ -*-==//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// Visual pane for viewing analysis graphs
//
//===----------------------------------------------------------------------===//

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

  void imageChanged ();
 public:
  PictureCanvas ()
    : wxScrolledWindow (0), myImage (0), myBitmap (0) {
  }
  PictureCanvas (wxWindow *parent, wxImage *image = 0)
    : wxScrolledWindow (parent), myImage (image), myBitmap (0) {
    imageChanged ();
  }
  void SetImage (wxImage *img) { myImage = img; imageChanged (); }
  void OnDraw (wxDC &aDC);

  DECLARE_EVENT_TABLE ()
};

#endif // PICTURECANVAS_H
