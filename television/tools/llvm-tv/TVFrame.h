//===-- TVFrame.h - Main window class for llvm-tv ----------------*- C++ -*-==//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVFRAME_H
#define TVFRAME_H

#include "wx/wx.h"
#include "wx/listctrl.h"
#include <string>
#include <vector>

/// TVSnapshot - Wrapper class for snapshots.
///
class TVSnapshot {
 public:
  std::string itemName;
  TVSnapshot () : itemName ("") { }
  TVSnapshot (const std::string &_name) : itemName (_name) { }
  TVSnapshot (const char *_name) : itemName (_name) { }
  const char *label () { return itemName.c_str (); }
};

///==---------------------------------------------------------------------==///

/// TVListCtrl - A specialization of wxListCtrl that displays a list of TV
/// Snapshots. 
///
class TVListCtrl : public wxListCtrl {
  typedef std::vector<TVSnapshot> Items;
  Items &itemList;

 public:
  /// refreshView - Make sure the display is up-to-date with respect to
  /// the list.
  ///
  void refreshView ();

  TVListCtrl (wxWindow *_parent, Items &_itemList)
    : wxListCtrl (_parent, -1, wxDefaultPosition, wxDefaultSize, wxLC_LIST),
      itemList (_itemList) {
    refreshView ();
  }
};

///==---------------------------------------------------------------------==///

/// Event IDs we use in the application
///
enum { 
  LLVM_TV_REFRESH = wxID_HIGHEST + 1
};

/// TVFrame - The main application window for the demo, which displays
/// the list view, status bar, and menu bar.
///
class TVFrame : public wxFrame {
  TVListCtrl *myListCtrl;
  std::vector<TVSnapshot> mySnapshotList;
  std::string mySnapshotDirectoryName;

 public:
  TVFrame (const char *title);
  void OnExit (wxCommandEvent &event);
  void OnAbout (wxCommandEvent &event);
  void OnRefresh (wxCommandEvent &event);

  void refreshSnapshotList ();
  void initializeSnapshotListAndView (std::string directoryName);

  DECLARE_EVENT_TABLE ();
};

#endif // TVFRAME_H
