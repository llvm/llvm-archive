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
#include "wx/textctrl.h"
#include "wx/html/htmlwin.h"
#include <string>
#include <vector>

/// TVTreeCtrl - A specialization of wxTreeCtrl that displays a list of LLVM
/// Modules and Functions from a snapshot
///
class TVTreeCtrl : public wxTreeCtrl {
  
  enum {
    TreeCtrlIcon_File,
    TreeCtrlIcon_FileSelected,
    TreeCtrlIcon_Folder,
    TreeCtrlIcon_FolderSelected,
    TreeCtrlIcon_FolderOpened
  };

  void updateTextDisplayed();

public:
  TVTreeCtrl::TVTreeCtrl(wxWindow *parent, const wxWindowID id,
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
  LLVM_TV_CODEVIEW_LIST
};

class TVApplication;

///==---------------------------------------------------------------------==///

/// TVFrame - The main application window for the demo, which displays
/// the tree view, status bar, and menu bar.
///
class TVFrame : public wxFrame {
  TVApplication *myApp;
  TVTreeCtrl *myTreeCtrl;
  std::vector<TVSnapshot> mySnapshotList;
  std::string mySnapshotDirName;
  wxSplitterWindow *splitterWindow;
  wxWindow *displayWidget;

  void Resize();
 public:
  TVFrame (TVApplication *app, const char *title);
  static wxWindow *createDisplayWidget (wxWindow *parent, const wxString &init);
  void OnExit (wxCommandEvent &event);
  void CallGraphView (wxCommandEvent &event);
  void CFGView (wxCommandEvent &event);
  void BUDSView (wxCommandEvent &event);
  void TDDSView (wxCommandEvent &event);
  void LocalDSView (wxCommandEvent &event);
  void CodeView (wxCommandEvent &event);
  void OnAbout (wxCommandEvent &event);
  void OnHelp (wxCommandEvent &event);
  void OnRefresh (wxCommandEvent &event);
  void CreateTree(long style, std::vector<TVSnapshot>&);
  void refreshSnapshotList ();
  void initializeSnapshotListAndView (std::string directoryName);
  
  DECLARE_EVENT_TABLE ();
};

#endif // TVFRAME_H
