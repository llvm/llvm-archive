//===-- TVApplication.h - Main application class for llvm-tv -----*- C++ -*--=//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVAPPLICATION_H
#define TVAPPLICATION_H

#include "wx/wx.h"
#include <string>
#include <vector>
#include "llvm/Module.h"

///==---------------------------------------------------------------------==///

class TVFrame;

/// TVApplication - This class shows a window containing a status bar and a menu
/// bar, and a list of files from a directory, that can be refreshed
/// using a menu item.
///
class TVApplication : public wxApp {
  TVFrame *myFrame;
  std::vector<wxWindow *> allMyWindows;
public:
  bool OnInit ();
  void GoodbyeFrom (wxWindow *dyingWindow);
  void ReceivedSignal ();
  void OpenCallGraphView (llvm::Module *M);
  void OpenCFGView (llvm::Function *F);
  void OpenCodeView (llvm::Function *F);
  void Quit ();
};

DECLARE_APP (TVApplication)

#endif // TVAPPLICATION_H
