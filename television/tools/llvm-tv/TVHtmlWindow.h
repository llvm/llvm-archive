#ifndef TVHTMLWINDOW_H
#define TVHTMLWINDOW_H

#include "wx/html/htmlwin.h"

class TVHtmlWindow : public wxHtmlWindow {
 public:
  TVHtmlWindow (wxWindow *_parent, wxWindowID _id, const wxString &_init = "")
     : wxHtmlWindow (_parent, _id) {
    AppendToPage (_init);
  }
};

#endif // TVHTMLWINDOW_H
