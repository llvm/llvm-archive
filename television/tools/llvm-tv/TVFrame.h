//===-- TVFrame.h - Main window class for llvm-tv ----------------*- C++ -*-==//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVFRAME_H
#define TVFRAME_H

#include "wx/wx.h"
#include "wx/listctrl.h"
#include "wx/splitter.h"
#include "wx/treectrl.h"
#include "wx/textctrl.h"
#include "TVSnapshot.h"
#include <string>
#include <vector>


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

/// TVTreeItemData - Base class for LLVM TV Tree Data
///  
class TVTreeItemData : public wxTreeItemData {
public:
  TVTreeItemData(const wxString& desc) : m_desc(desc) { }
  
  void ShowInfo(wxTreeCtrl *tree);
  const wxChar *GetDesc() const { return m_desc.c_str(); }
  virtual void print(std::ostream&) {};
  virtual Module *getModule() { return 0; }
  virtual Function *getFunction() { return 0; }
private:
  wxString m_desc;
};


/// TVTreeModuleItem - Tree Item containing a Module
///  
class TVTreeModuleItem : public TVTreeItemData {
public:
  TVTreeModuleItem(const wxString& desc, Module *mod) : TVTreeItemData(desc), 
							     myModule(mod) {}
  
  void print(std::ostream &out) { myModule->print(out); }
  Module *getModule() { return myModule; }
private:
  Module *myModule;
};

/// TVTreeFunctionItem - Tree Item containing a Function
///  
class TVTreeFunctionItem : public TVTreeItemData {
public:
  TVTreeFunctionItem(const wxString& desc, Function *func) : TVTreeItemData(desc), 
							     myFunc(func) {}
  
  void print(std::ostream &out) { myFunc->print(out); }
  Module *getModule() { return myFunc->getParent (); }
  Function *getFunction() { return myFunc; }
private:
  Function *myFunc;
};


///==---------------------------------------------------------------------==///

/// TVTreeCtrl - A specialization of wxTreeCtrl that displays a list of LLVM
/// Modules and Functions from a snapshot
///

///==---------------------------------------------------------------------==///
class TVTreeCtrl : public wxTreeCtrl {
  
  enum
    {
      TreeCtrlIcon_File,
      TreeCtrlIcon_FileSelected,
      TreeCtrlIcon_Folder,
      TreeCtrlIcon_FolderSelected,
      TreeCtrlIcon_FolderOpened
    };

  void updateTextDisplayed();

public:
  TVTreeCtrl::TVTreeCtrl(wxWindow *parent, const wxWindowID id,
                       const wxPoint& pos, const wxSize& size,
			 long style);

  
  
  virtual ~TVTreeCtrl();
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
  LLVM_TV_SPLITTER_WINDOW,
  LLVM_TV_CALLGRAPHVIEW,
  LLVM_TV_CFGVIEW,
  LLVM_TV_CODEVIEW
};

class TVApplication;

/// TVFrame - The main application window for the demo, which displays
/// the list view, status bar, and menu bar.
///
class TVFrame : public wxFrame {
  TVApplication *myApp;
  TVTreeCtrl *myTreeCtrl;
  std::vector<TVSnapshot> mySnapshotList;
  std::string mySnapshotDirName;
  
  wxSplitterWindow *splitterWindow;
  wxTextCtrl *displayText;
  wxListCtrl *displayCode;

  void Resize();
 public:
  TVFrame (TVApplication *app, const char *title);
  void OnExit (wxCommandEvent &event);
  void CallGraphView (wxCommandEvent &event);
  void CFGView (wxCommandEvent &event);
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
