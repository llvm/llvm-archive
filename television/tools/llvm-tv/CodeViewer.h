#ifndef INSTRVIEWER_H
#define INSTRVIEWER_H

#include "wx/wx.h"
#include <wx/listctrl.h>
#include <string>
#include <vector>

class TVApplication;

namespace llvm {
  class Function;
  class Value;
}


/// TVCodeItem
///
class TVCodeItem {
 private:
  llvm::Value *Val;
  std::string label;

 public:
  TVCodeItem(llvm::Value *V) : Val(V) {}
  const std::string& getLabel();
};


/// TVCodeViewer
///
class TVCodeViewer : public wxListCtrl {
  typedef std::vector<TVCodeItem*> Items;
  Items itemList;

  void refreshView();
 public:
  TVCodeViewer(wxWindow *_parent, llvm::Function *F);
  
};


/// CodeViewFrame
///
class CodeViewFrame : public wxFrame {
  TVApplication *myApp;
  TVCodeViewer *codeViewer;
  void setupAppearance () {
    SetSize (wxRect (200, 200, 300, 300));
    Show (TRUE);
  }
 public:
  CodeViewFrame(TVApplication *app, llvm::Function *F)
    : wxFrame (NULL, -1, "code viewer"), myApp (app) {
    codeViewer = new TVCodeViewer(this, F);
    setupAppearance();
  }

  ~CodeViewFrame() {
    delete codeViewer;
  }

  bool OnClose (wxCloseEvent &event);
  DECLARE_EVENT_TABLE ()
};


#endif
