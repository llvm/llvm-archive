//===-- CodeViewer.h - Interactive instruction stream viewer ----*- C++ -*-===//
//
// The interactive code explorer for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef CODEVIEWER_H
#define CODEVIEWER_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <map>
#include <string>
#include <vector>

class TVApplication;

namespace llvm {
  class Function;
  class Value;
}


/// TVCodeItem - contains an instruction, basic block, or function (which prints
/// out as the header)
///
class TVCodeItem : public wxListItem {
 private:
  llvm::Value *Val;
  std::string label;

  void SetLabel();
 public:
  TVCodeItem(llvm::Value *V) : Val(V) { SetLabel(); }
  llvm::Value* getValue() { return Val; }
};


/// TVCodeViewer
///
class TVCodeViewer : public wxListCtrl {
  typedef std::vector<TVCodeItem*> Items;
  Items itemList;

  std::map<llvm::Value*, TVCodeItem*> ValueToItem;
  std::map<TVCodeItem*, unsigned> ItemToIndex;

  void refreshView();
 public:
  TVCodeViewer(wxWindow *_parent, llvm::Function *F);
  void OnItemActivated(wxListEvent &event);  
  void OnItemSelected(wxListEvent &event);
  void OnItemDeselected(wxListEvent &event);

  DECLARE_EVENT_TABLE ()
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

  void OnClose (wxCloseEvent &event);
  DECLARE_EVENT_TABLE ()
};


#endif
