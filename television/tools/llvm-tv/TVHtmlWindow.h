#ifndef TVHTMLWINDOW_H
#define TVHTMLWINDOW_H

#include "ItemDisplayer.h"
#include "wx/html/htmlwin.h"

class TVHtmlWindow : public ItemDisplayer {
  wxHtmlWindow *myHtmlWindow;
 public:
  TVHtmlWindow (wxWindow *_parent, wxWindowID _id, const wxString &_init = "") {
    myHtmlWindow = new wxHtmlWindow (_parent, _id);
    myHtmlWindow->AppendToPage (_init);
  }
  void displayItem (TVTreeItemData *item);
  wxWindow *getWindow () { return myHtmlWindow; }
};

#endif // TVHTMLWINDOW_H
