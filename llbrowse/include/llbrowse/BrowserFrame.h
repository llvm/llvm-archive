/*
 * BrowserFrame.h
 */

#ifndef LLBROWSE_BROWSERFRAME_H
#define LLBROWSE_BROWSERFRAME_H

#ifndef _WX_WX_H_
#include "wx/wx.h"
#endif

#ifndef _WX_TREECTRL_H_BASE_
#include "wx/treectrl.h"
#endif

#ifndef _WX_LISTCTRL_H_BASE_
#include "wx/listctrl.h"
#endif

#ifndef _WX_SPLITTER_H_BASE_
#include "wx/splitter.h"
#endif

#ifndef _WX_HTMLWIN_H_
#include "wx/html/htmlwin.h"
#endif

#include <auto_ptr.h>

namespace llvm {
class LLVMContext;
class Module;
}

class TreeView;
class TreeItemBase;
class DetailsView;

class BrowserFrame : public wxFrame {
public:
  enum MenuID {
    ID_SORT_FUNCTION_NAMES = 0x100,
    ID_SORT_VARIABLE_NAMES
  };

  BrowserFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
  ~BrowserFrame();

  // Event handlers

  void OnQuit(wxCommandEvent& event);
  void OnOpen(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);

  void OnTreeItemExpanding(wxTreeEvent& event);
  void OnTreeSelChanged(wxTreeEvent& event);

  void SetCurrentDir(wxString& dir);
  wxString GetCurrentDir();

private:
  wxFileDialog* fileDialog_;
  wxSplitterWindow* splitter_;
  TreeView* tree_;
  DetailsView* details_;
  wxString lastLoadDir_;
  llvm::LLVMContext& context_;
  std::auto_ptr<llvm::Module> module_;

  void CreateMenus();
  void CreatePanels();
  void LoadModule(const wxString& path);
  void ShowItemDetails(TreeItemBase* treeItem);
  void BuildTree();
  bool Destroy();
};

#endif // LLBROWSE_BROWSERFRAME_H
