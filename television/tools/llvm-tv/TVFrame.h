//===-- TVFrame.h - Main window class for llvm-tv ----------------*- C++ -*-==//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVFRAME_H
#define TVFRAME_H

#include "TVSnapshot.h"
#include "TVWindowIDs.h"
#include "wx/wx.h"
#include "wx/listctrl.h"
#include "wx/splitter.h"
#include "wx/treectrl.h"
#include "wx/notebook.h"
#include "ItemDisplayer.h"
#include <string>
#include <vector>

/// TVTreeCtrl - A specialization of wxTreeCtrl that displays a list of LLVM
/// Modules and Functions from a snapshot
///
class TVFrame;
class TVTreeCtrl : public wxTreeCtrl {
  TVFrame *myFrame;
public:
  TVTreeCtrl::TVTreeCtrl(wxWindow *parent, TVFrame *frame, const wxWindowID id,
                         const wxPoint& pos = wxDefaultPosition,
                         const wxSize& size = wxDefaultSize,
                         long style = wxTR_HIDE_ROOT | wxTR_DEFAULT_STYLE
                                    | wxSUNKEN_BORDER);
  virtual ~TVTreeCtrl() { }
  void AddSnapshotsToTree(std::vector<TVSnapshot>&);
  void updateSnapshotList(std::vector<TVSnapshot>&);
  void OnSelChanged(wxTreeEvent &event);
  TVTreeItemData *GetSelectedItemData ();

  DECLARE_EVENT_TABLE();
};

///==---------------------------------------------------------------------==///

/// TVNotebook - The class which is responsible for the "tab view"
/// containing visualizers, which appears on the right-hand side
/// of the main LLVM-TV window.
///
class TVNotebook : public wxNotebook {
  std::vector<ItemDisplayer *> displayers;
  TVTreeItemData *selectedItem;

  void displaySelectedItemOnPage (int page);
public:
  TVNotebook (wxWindow *_parent)
    : wxNotebook (_parent, LLVM_TV_NOTEBOOK) { }
  void OnSelChanged (wxNotebookEvent &event);
  bool AddItemDisplayer (ItemDisplayer *displayWidget);
  void SetSelectedItem (TVTreeItemData *newSelectedItem);
  DECLARE_EVENT_TABLE ()
};

///==---------------------------------------------------------------------==///

/// TVFrame - The main application window for LLVM-TV, which is responsible
/// for displaying the tree view, tab view, status bar, and menu bar.
///
class TVApplication;
class TVFrame : public wxFrame {
  TVApplication *myApp;

  std::vector<TVSnapshot> mySnapshotList;
  std::string mySnapshotDirName;

  wxSplitterWindow *splitterWindow; // divides this into left & right sides
  TVTreeCtrl *myTreeCtrl;           // left side - displays tree view of module
  TVNotebook *notebook;             // right side - tab views w/ item displayers

  void Resize();
 public:
  TVFrame (TVApplication *app, const char *title);
  void OnExit (wxCommandEvent &event);
  void CallGraphView (wxCommandEvent &event);
  void CFGView (wxCommandEvent &event);
  void BUDSView (wxCommandEvent &event);
  void TDDSView (wxCommandEvent &event);
  void LocalDSView (wxCommandEvent &event);
  void CodeView (wxCommandEvent &event);
  void OnAbout (wxCommandEvent &event);
  void OnHelp (wxCommandEvent &event);
  void OnOpen (wxCommandEvent &event);
  void OnRefresh (wxCommandEvent &event);
  void CreateTree(long style, std::vector<TVSnapshot>&);
  void refreshSnapshotList ();
  void initializeSnapshotListAndView (std::string directoryName);
  void updateDisplayedItem (TVTreeItemData *newlyDisplayedItem);

  DECLARE_EVENT_TABLE ();
};

#endif // TVFRAME_H
