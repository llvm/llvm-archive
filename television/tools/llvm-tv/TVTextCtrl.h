#ifndef TVTEXTCTRL_H
#define TVTEXTCTRL_H

#include "TVWindowIDs.h"
#include "ItemDisplayer.h"
#include "wx/textctrl.h"

class TVTextCtrl : public ItemDisplayer {
  wxTextCtrl *myTextCtrl;
 public:
  TVTextCtrl (wxWindow *_parent, const wxString &_value = "") {
    myTextCtrl = new wxTextCtrl (_parent, LLVM_TV_TEXT_CTRL, _value,
      wxDefaultPosition, wxDefaultSize,
      wxTE_READONLY | wxTE_MULTILINE | wxHSCROLL);
  }
  void displayItem (TVTreeItemData *item);
  wxWindow *getWindow () { return myTextCtrl; }
  std::string getDisplayTitle (TVTreeItemData *item) { return "Text view"; }
};

#endif // TVTEXTCTRL_H
