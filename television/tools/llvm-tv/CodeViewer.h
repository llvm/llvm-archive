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
#include "ItemDisplayer.h"

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

class TVCodeListCtrl : public wxListCtrl {
  typedef std::vector<TVCodeItem*> Items;
  Items itemList;
  std::map<llvm::Value*, TVCodeItem*> ValueToItem;
  std::map<TVCodeItem*, long> ItemToIndex;

  void refreshView();
  void changeItemTextAttrs (TVCodeItem *item, wxColour *newColor, int newWgt);

 public:
  void SetFunction (llvm::Function *F);
  TVCodeListCtrl(wxWindow *_parent, llvm::Function *F);
  void OnItemSelected(wxListEvent &event);
  void OnItemDeselected(wxListEvent &event);

  DECLARE_EVENT_TABLE ()
};

/// TVCodeViewer
///
class TVCodeViewer : public ItemDisplayer {
  TVCodeListCtrl *myListCtrl;
public:
  TVCodeViewer (wxWindow *_parent);
  void displayItem (TVTreeItemData *data);
  std::string getDisplayTitle (TVTreeItemData *data) { return "Code view"; }
  wxWindow *getWindow () { return myListCtrl; }
};

/// CodeViewFrame
///
class CodeViewFrame : public wxFrame {
  TVApplication *myApp;
  TVCodeListCtrl *codeViewer;
  void setupAppearance () {
    SetSize (wxRect (200, 200, 300, 300));
    Show (TRUE);
  }
 public:
  CodeViewFrame(TVApplication *app, llvm::Function *F)
    : wxFrame (NULL, -1, "code viewer"), myApp (app) {
    codeViewer = new TVCodeListCtrl(this, F);
    setupAppearance();
  }
  ~CodeViewFrame() { delete codeViewer; }
  void OnClose (wxCloseEvent &event);
  DECLARE_EVENT_TABLE ()
};


#endif
