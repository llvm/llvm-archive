//===-- TVFrame.h - Main window class for llvm-tv ----------------*- C++ -*-==//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVFRAME_H
#define TVFRAME_H

#include "TVSnapshot.h"
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

  DECLARE_EVENT_TABLE();
};


/// Event IDs we use in the application
///
enum { 
  LLVM_TV_REFRESH = wxID_HIGHEST + 1,
  LLVM_TV_TREE_CTRL,
  LLVM_TV_TEXT_CTRL,
  LLVM_TV_HTML_WINDOW,
  LLVM_TV_SPLITTER_WINDOW,
  LLVM_TV_CALLGRAPHVIEW,
  LLVM_TV_CFGVIEW,
  LLVM_TV_BUDS_VIEW,
  LLVM_TV_TDDS_VIEW,
  LLVM_TV_LOCALDS_VIEW,
  LLVM_TV_CODEVIEW,
  LLVM_TV_CODEVIEW_LIST,
  LLVM_TV_NOTEBOOK
};

class TVApplication;

///==---------------------------------------------------------------------==///

/// TVFrame - The main application window for the demo, which displays
/// the tree view, status bar, and menu bar.
///
class TVFrame : public wxFrame {
  TVApplication *myApp;

  std::vector<TVSnapshot> mySnapshotList;
  std::string mySnapshotDirName;

  wxSplitterWindow *splitterWindow; // divides this into left & right sides
  TVTreeCtrl *myTreeCtrl;           // left side - displays tree view of module
  wxNotebook *notebook;             // right side - contains tab views
  ItemDisplayer *displayWidget;     // displays selected tree item inside a tab
  ItemDisplayer *displayWidget2;    // displays selected tree item inside a tab

  void Resize();
 public:
  TVFrame (TVApplication *app, const char *title);
  static ItemDisplayer *createDisplayWidget (wxWindow *parent, const wxString &init, unsigned nohtml);
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
