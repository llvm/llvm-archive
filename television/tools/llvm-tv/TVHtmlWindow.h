#ifndef TVHTMLWINDOW_H
#define TVHTMLWINDOW_H

#include "ItemDisplayer.h"
#include "wx/html/htmlwin.h"

class TVHtmlWindow : public wxHtmlWindow, public ItemDisplayer {
 public:
  TVHtmlWindow (wxWindow *_parent, wxWindowID _id, const wxString &_init = "")
     : wxHtmlWindow (_parent, _id) {
    AppendToPage (_init);
  }
  void displayItem (TVTreeItemData *item);
};

#endif // TVHTMLWINDOW_H
