#ifndef TVTEXTCTRL_H
#define TVTEXTCTRL_H

#include "wx/textctrl.h"

class TVTextCtrl : public wxTextCtrl {
 public:
  TVTextCtrl (wxWindow *_parent, wxWindowID _id, const wxString &_value = "")
     : wxTextCtrl (_parent, _id, _value, wxDefaultPosition, wxDefaultSize,
                   wxTE_READONLY | wxTE_MULTILINE | wxHSCROLL) { }
};

#endif // TVTEXTCTRL_H
