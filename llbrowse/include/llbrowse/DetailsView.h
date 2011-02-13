/*
 * DetailsView.h
 */

#ifndef LLBROWSE_DETAILSVIEW_H
#define LLBROWSE_DETAILSVIEW_H

#include "wx/listctrl.h"

namespace llvm {
class StringRef;
}

/// Details view class

class DetailsView : public wxListCtrl {
public:
  DetailsView();
  DetailsView(wxWindow *parent);

  DECLARE_DYNAMIC_CLASS(DetailsView)

  /** Add a row to the view. */
  void Add(const wxString& key, const wxString& value);
  void Add(const wxString& key, const wxChar* value);
  void Add(const wxString& key, const llvm::StringRef& value);
  void Add(const wxString& key, bool value);
  void Add(const wxString& key, unsigned value);

protected:
  void AddImpl(const wxString& key, const wxString& value);
  void CreateColumns();
};

#endif // LLBROWSE_DETAILSVIEW_H
