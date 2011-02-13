/*
 * DetailsView.cpp
 */

#include "llbrowse/DetailsView.h"
#include "llbrowse/Formatting.h"

IMPLEMENT_DYNAMIC_CLASS(DetailsView, wxListCtrl)

DetailsView::DetailsView()
  : wxListCtrl(NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_NO_HEADER) {
  CreateColumns();
}

DetailsView::DetailsView(wxWindow *parent)
  : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_NO_HEADER) {
  CreateColumns();
}

void DetailsView::CreateColumns() {
  InsertColumn(0, _("Key"), wxLIST_FORMAT_RIGHT);
  InsertColumn(1, _("Val"), wxLIST_FORMAT_LEFT);
}

void DetailsView::Add(const wxString& key, const wxString& value) {
  AddImpl(key, value);
}

void DetailsView::Add(const wxString& key, const wxChar* value) {
  AddImpl(key, value);
}

void DetailsView::Add(const wxString& key, const llvm::StringRef& value) {
  AddImpl(key, toWxStr(value));
}

void DetailsView::Add(const wxString& key, bool value) {
  AddImpl(key, value ? _("true") : _("false"));
}

void DetailsView::Add(const wxString& key, unsigned value) {
  AddImpl(key, wxString::Format(wxT("%ld"), value));
}

void DetailsView::AddImpl(const wxString& key, const wxString& value) {
  long index = InsertItem(GetItemCount(), key);
  SetItem(index, 1, value);
}

