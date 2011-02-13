/*
 * LLBrowseFrame.cpp
 */

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/raw_ostream.h"

#include "llbrowse/BrowserFrame.h"
#include "llbrowse/BrowserApp.h"
#include "llbrowse/TreeView.h"
#include "llbrowse/DetailsView.h"
#include "llbrowse/Resources.h"

#include "wx/wfstream.h"
#include "wx/imaglist.h"

using namespace llvm;

BrowserFrame::BrowserFrame(
    const wxString& title, const wxPoint& pos, const wxSize& size)
  : wxFrame(NULL, -1, title, pos, size)
  , context_(getGlobalContext())
{
  CreateMenus();
  CreateStatusBar();
  CreatePanels();

  SetStatusText(_("No module loaded"));

  fileDialog_ = new wxFileDialog(this, _("Open LLVM module"),
      wxString(), wxString(),
      wxString::FromAscii("LLVM IR files (*.bc;*.ll)|*.bc;*.ll"),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  Show(true);
}

BrowserFrame::~BrowserFrame() {
  delete fileDialog_;
}

void BrowserFrame::OnQuit(wxCommandEvent& WXUNUSED(event)) {
  Close(true);
}

void BrowserFrame::OnOpen(wxCommandEvent& WXUNUSED(event)) {
  if (fileDialog_->ShowModal() == wxID_OK) {
    // Load the module.
    LoadModule(fileDialog_->GetPath());
  }
}

void BrowserFrame::CreateMenus() {
  // File menu
  wxMenu* menuFile = new wxMenu();
  menuFile->Append(wxID_OPEN, _("&Open..."));
  menuFile->AppendSeparator();
  menuFile->Append(wxID_ABOUT, _("&About..."));
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT, _("E&xit"));
  Connect(wxID_OPEN, wxEVT_COMMAND_MENU_SELECTED,
      (wxObjectEventFunction) (&BrowserFrame::OnOpen));
  Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED,
      (wxObjectEventFunction) (&BrowserFrame::OnAbout));
  Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
      (wxObjectEventFunction) (&BrowserFrame::OnQuit));

  // View menu
//  wxMenu* menuView = new wxMenu();
//  menuFile->Append(ID_SORT_FUNCTION_NAMES, _("Sort &Functions by name"));
//  menuFile->Append(ID_SORT_VARIABLE_NAMES, _("Sort &Variables by name"));

  // Menu bar
  wxMenuBar* menuBar = new wxMenuBar();
  menuBar->Append(menuFile, _("&File"));
//  menuBar->Append(menuView, _("&View"));
  SetMenuBar(menuBar);
}

void BrowserFrame::CreatePanels() {
  splitter_ = new wxSplitterWindow(this);

  tree_ = new TreeView(splitter_);
  tree_->SetWindowStyleFlag(wxTR_HAS_BUTTONS);
  tree_->SetButtonsImageList(Resources::GetTreeListButtonList());
  tree_->SetImageList(Resources::GetTreeListIconList());

  details_ = new DetailsView(splitter_);
  splitter_->Initialize(tree_);
  splitter_->SplitVertically(tree_, details_);

  Connect(tree_->GetId(), wxEVT_COMMAND_TREE_ITEM_EXPANDING,
      (wxObjectEventFunction) &BrowserFrame::OnTreeItemExpanding);
  Connect(tree_->GetId(), wxEVT_COMMAND_TREE_SEL_CHANGED,
      (wxObjectEventFunction) &BrowserFrame::OnTreeSelChanged);
}

void BrowserFrame::LoadModule(const wxString & path) {
  wxFileInputStream inStream(path);
  if (!inStream.IsOk()) {
    // TODO: Report error
    return;
  }

  SMDiagnostic err;
  std::auto_ptr<Module> mod;
  OwningPtr<MemoryBuffer> buffer;
  buffer.reset(MemoryBuffer::getNewUninitMemBuffer(inStream.GetSize(), ""));

  inStream.Read((void *) buffer->getBufferStart(), buffer->getBufferSize());
  if (!inStream.IsOk()) {
    // TODO: Report error
    return;
  }

  mod.reset(ParseIR(buffer.take(), err, context_));
  if (mod.get() == 0) {
    std::string errs;
    raw_string_ostream errStrm(errs);
    err.Print("llbrowse", errStrm);
    wxString errString = wxString::From8BitData(&*errs.begin(), errs.size());
    // Show error dialog.
    //return 1;
  }

  module_.reset(mod.release());
  lastLoadDir_ = fileDialog_->GetDirectory();

  BuildTree();
}

void BrowserFrame::BuildTree() {
  wxString rootName(_("Module - "));
  const std::string & moduleName = module_->getModuleIdentifier();
  if (!moduleName.empty()) {
    rootName.Append(
        wxString::From8BitData(&*moduleName.begin(), moduleName.size()));
  }

  tree_->SetModule(module_.get(), rootName);
  TreeItemBase* treeItem =
      static_cast<TreeItemBase*>(tree_->GetItemData(tree_->GetRootItem()));
  ShowItemDetails(treeItem);
}

void BrowserFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
  wxMessageBox(_("Visual browser for LLVM .bc files"),
      _("About LLBrowse"),
      wxOK | wxICON_INFORMATION, this);
}

void BrowserFrame::OnTreeItemExpanding(wxTreeEvent& event) {
  const wxTreeItemId item = event.GetItem();

  // Don't add items if folder already expanded.
  if (tree_->GetChildrenCount(item) > 0) {
    return;
  }

  TreeItemBase* treeItem =
      static_cast<TreeItemBase*>(tree_->GetItemData(item));
  treeItem->CreateChildren(tree_, item);
}

void BrowserFrame::OnTreeSelChanged(wxTreeEvent& event) {
  const wxTreeItemId item = event.GetItem();
  TreeItemBase* treeItem =
      static_cast<TreeItemBase*>(tree_->GetItemData(item));
  ShowItemDetails(treeItem);
}

void BrowserFrame::ShowItemDetails(TreeItemBase* treeItem) {
  details_->DeleteAllItems();
  treeItem->ShowDetails(details_);
  details_->SetColumnWidth(0, wxLIST_AUTOSIZE);
  details_->SetColumnWidth(1, wxLIST_AUTOSIZE);
}

bool BrowserFrame::Destroy() {
  wxGetApp().SavePrefs();
  return wxFrame::Destroy();
}

void BrowserFrame::SetCurrentDir(wxString & dir) {
  lastLoadDir_ = dir;
  fileDialog_->SetDirectory(dir);
}

wxString BrowserFrame::GetCurrentDir() {
  return lastLoadDir_;
}
