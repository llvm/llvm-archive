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

///==---------------------------------------------------------------------==///

namespace llvm {
  class Function;
  class Module;
}

class TVFrame;
class TVTreeItemData;
class ItemDisplayer;

/// FatalErrorBox - pop up an error message and quit.
///
void FatalErrorBox (const std::string msg);

/// TVApplication - This class shows a window containing a status bar and a menu
/// bar, and a list of files from a directory, that can be refreshed
/// using a menu item.
///
class TVApplication : public wxApp {
  TVFrame *myFrame;
  std::vector<wxWindow *> allMyWindows;
  std::vector<ItemDisplayer *> allMyDisplayers;
public:
  bool OnInit ();
  void GoodbyeFrom (wxWindow *dyingWindow);
  void ReceivedSignal ();
  void OpenCallGraphView (TVTreeItemData *M);
  void OpenCFGView (TVTreeItemData *F);
  void OpenBUDSView (TVTreeItemData *M);
  void OpenTDDSView (TVTreeItemData *F);
  void OpenLocalDSView (TVTreeItemData *F);
  void OpenCodeView (llvm::Function *F);
  void Quit ();
};

DECLARE_APP (TVApplication)

#endif // TVAPPLICATION_H
