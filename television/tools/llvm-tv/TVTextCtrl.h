#ifndef TVTEXTCTRL_H
#define TVTEXTCTRL_H

#include "ItemDisplayer.h"
#include "wx/textctrl.h"

class TVTextCtrl : public ItemDisplayer {
  wxTextCtrl *myTextCtrl;
 public:
  TVTextCtrl (wxWindow *_parent, wxWindowID _id, const wxString &_value = "") {
    myTextCtrl = new wxTextCtrl (_parent, _id, _value, wxDefaultPosition,
      wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxHSCROLL);
  }
  void displayItem (TVTreeItemData *item);
  wxWindow *getWindow () { return myTextCtrl; }
};

#endif // TVTEXTCTRL_H
