#ifndef TVHTMLWINDOW_H
#define TVHTMLWINDOW_H

#include "TVWindowIDs.h"
#include "ItemDisplayer.h"
#include "wx/html/htmlwin.h"

class TVHtmlWindow : public ItemDisplayer {
  wxHtmlWindow *myHtmlWindow;
 public:
  TVHtmlWindow (wxWindow *_parent, const wxString &_init = "") {
    myHtmlWindow = new wxHtmlWindow (_parent, LLVM_TV_HTML_WINDOW);
    myHtmlWindow->AppendToPage (_init);
  }
  void displayItem (TVTreeItemData *item);
  wxWindow *getWindow () { return myHtmlWindow; }
  std::string getDisplayTitle (TVTreeItemData *item) { return "HTML view"; }
};

#endif // TVHTMLWINDOW_H
