//===-- TVApplication.h - Main application class for llvm-tv -----*- C++ -*--=//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVAPPLICATION_H
#define TVAPPLICATION_H

#include "wx/wx.h"
#include <string>
#include "TVFrame.h"

///==---------------------------------------------------------------------==///

/// TVApplication - This class shows a window containing a status bar and a menu
/// bar, and a list of files from a directory, that can be refreshed
/// using a menu item.
///
class TVApplication : public wxApp {
 public:
  virtual bool OnInit ();
};

DECLARE_APP (TVApplication)

#endif // TVAPPLICATION_H
