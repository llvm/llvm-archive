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
  class Module;
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
  TVCodeListCtrl(wxWindow *_parent);
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
  TVCodeViewer (wxWindow *_parent) {
    myListCtrl = new TVCodeListCtrl (_parent);
  }
  virtual ~TVCodeViewer () { delete myListCtrl; }
  void displayItem (TVTreeItemData *data);
  void viewFunctionCode (llvm::Function *F);
  void viewModuleCode (llvm::Module *M);
  std::string getDisplayTitle (TVTreeItemData *data) { return "Code view"; }
  wxWindow *getWindow () { return myListCtrl; }
};

#endif
