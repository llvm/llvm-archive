//===-- TVFrame.cpp - Main window class for LLVM-TV -----------------------===//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#include "TVApplication.h"
#include "TVFrame.h"
#include "TVHtmlWindow.h"
#include "TVTextCtrl.h"
#include "TVTreeItem.h"
#include "llvm-tv/Config.h"
#include "CFGGraphDrawer.h"
#include "CallGraphDrawer.h"
#include "DSAGraphDrawer.h"
#include "PictureFrame.h"
#include <dirent.h>
#include <cassert>
#include <sstream>

/// TreeCtrl constructor - creates the root and adds it to the tree
///
TVTreeCtrl::TVTreeCtrl(wxWindow *parent, TVFrame *frame, const wxWindowID id,
                       const wxPoint& pos, const wxSize& size,
                       long style)
  : wxTreeCtrl(parent, id, pos, size, style), myFrame (frame) {
  wxTreeItemId rootId = AddRoot("Snapshots", -1, -1,TVTreeRootItem::instance());
}

/// AddSnapshotsToTree - Given a list of snapshots the tree is populated
///
void TVTreeCtrl::AddSnapshotsToTree(std::vector<TVSnapshot> &list) {
  wxTreeItemId rootId = GetRootItem();
  for (std::vector<TVSnapshot>::iterator I = list.begin(), E = list.end();
       I != E; ++I) {
    // Get the Module associated with this snapshot and add it to the tree
    Module *M = I->getModule();
    wxTreeItemId id = AppendItem(rootId, I->label(), -1, -1,
                                 new TVTreeModuleItem(I->label(), M));

    // Loop over functions in the module and add them to the tree as children
    for (Module::iterator I = M->begin(), E = M->end(); I != E; ++I) {
      Function *F = I;
      if (!F->isExternal()) {
        const char *FuncName = F->getName().c_str();
        AppendItem(id, FuncName, -1, -1, new TVTreeFunctionItem(FuncName, I));
      }
    }
  }   
}

/// updateSnapshotList - Update the tree with the current snapshot list
///
void TVTreeCtrl::updateSnapshotList(std::vector<TVSnapshot>& list) {
  DeleteChildren(GetRootItem());
  AddSnapshotsToTree(list);
}

/// GetSelectedItemData - Return the currently-selected visualizable
/// object (TVTreeItemData object).
///
TVTreeItemData *TVTreeCtrl::GetSelectedItemData () {
  return dynamic_cast<TVTreeItemData *> (GetItemData (GetSelection ()));
}

/// OnSelChanged - Inform the parent frame that the selection has changed,
/// and pass the newly selected item to it.
///
void TVTreeCtrl::OnSelChanged(wxTreeEvent &event) {
  myFrame->updateDisplayedItem (GetSelectedItemData ());
}

///==---------------------------------------------------------------------==///

void TVTextCtrl::displayItem (TVTreeItemData *item) {
  std::ostringstream Out;
  item->print (Out);
  myTextCtrl->SetValue ("");
  myTextCtrl->AppendText (Out.str ().c_str ());
  myTextCtrl->ShowPosition (0);
  myTextCtrl->SetInsertionPoint (0);
}

void TVHtmlWindow::displayItem (TVTreeItemData *item) {
  std::ostringstream Out;
  item->printHTML (Out);
  myHtmlWindow->Hide ();
  myHtmlWindow->SetPage (wxString (""));
  myHtmlWindow->AppendToPage (wxString (Out.str ().c_str ()));
  myHtmlWindow->Show ();
}

///==---------------------------------------------------------------------==///

static const wxString Explanation
  ("Click on a Module or Function in the left-hand pane\n"
   "to display its code in the right-hand pane. Then, you\n"
   "can choose from the View menu to see graphical code views.\n"); 

/// updateDisplayedItem - Updates right-hand pane with a view of the item that
/// is now selected.
///
void TVFrame::updateDisplayedItem (TVTreeItemData *newlySelectedItem) {
  // Tell the current visualizer widget to display the selected
  // LLVM object in its window, which is displayed inside the notebook.
  assert (newlySelectedItem
          && "newlySelectedItem was null in updateDisplayedItem()");
  notebook->SetSelectedItem (newlySelectedItem);
}

void TVFrame::refreshSnapshotList () {
  // re-load the list of snapshots
  const char *directoryName = mySnapshotDirName.c_str ();
  mySnapshotList.clear ();
  DIR *d = opendir (directoryName);
  if (!d)
    FatalErrorBox ("trying to open directory " + mySnapshotDirName + ": "
                   + strerror(errno));
  while (struct dirent *de = readdir (d))
    if (memcmp(de->d_name, ".", 2) && memcmp(de->d_name, "..", 3))
      mySnapshotList.push_back (mySnapshotDirName + "/" + de->d_name);
  sort (mySnapshotList.begin (), mySnapshotList.end ());

  closedir (d);

  if (myTreeCtrl != 0)
    myTreeCtrl->updateSnapshotList(mySnapshotList);
}

void TVFrame::initializeSnapshotListAndView (std::string dirName) {
  // Initialize the snapshot list
  mySnapshotDirName = dirName;
  refreshSnapshotList ();
  SetStatusText ("Snapshot list has been loaded.");
}

//==------------------------------------------------------------------------==//

void TVNotebook::displaySelectedItemOnPage (int page) {
  if (selectedItem)
    displayers[page]->displayItem (selectedItem);
}

void TVNotebook::SetSelectedItem (TVTreeItemData *newSelectedItem) {
  selectedItem = newSelectedItem;
  displaySelectedItemOnPage (GetSelection ());
}

bool TVNotebook::AddItemDisplayer (ItemDisplayer *displayer) {
  int pageIndex = GetPageCount ();
  displayers.resize (1 + pageIndex);
  displayers[pageIndex] = displayer;
  return AddPage (displayer->getWindow (),
                  displayer->getDisplayTitle (0).c_str (), true);
}

void TVNotebook::OnSelChanged (wxNotebookEvent &event) {
  displaySelectedItemOnPage (event.GetSelection ());
}

BEGIN_EVENT_TABLE (TVNotebook, wxNotebook)
  EVT_NOTEBOOK_PAGE_CHANGED(LLVM_TV_NOTEBOOK, TVNotebook::OnSelChanged)
END_EVENT_TABLE ()

//==------------------------------------------------------------------------==//

/// TVFrame constructor - used to set up typical appearance of visualizer's
/// top-level window.
///
TVFrame::TVFrame (TVApplication *app, const char *title)
  : wxFrame (NULL, -1, title), myApp (app) {
  // Set up appearance
  CreateStatusBar ();
  SetSize (wxRect (100, 100, 500, 200));
  Show (FALSE);
  splitterWindow = new wxSplitterWindow(this, LLVM_TV_SPLITTER_WINDOW,
                                        wxDefaultPosition, wxDefaultSize,
                                        wxSP_3D);

  // Create tree view of snapshots
  myTreeCtrl = new TVTreeCtrl(splitterWindow, this, LLVM_TV_TREE_CTRL);
  Resize();

  // Create right-hand pane's display widget and stick it in a notebook control.
  notebook = new TVNotebook (splitterWindow);
  notebook->AddItemDisplayer (new TVTextCtrl (notebook, Explanation));
  notebook->AddItemDisplayer (new TVHtmlWindow (notebook, Explanation));
  notebook->AddItemDisplayer (new TDGraphDrawer (notebook));

  // Split window vertically
  splitterWindow->SplitVertically(myTreeCtrl, notebook, 200);
  Show (TRUE);
}

/// OnHelp - display the help dialog
///
void TVFrame::OnHelp (wxCommandEvent &event) {
  wxMessageBox (Explanation, "Help with LLVM-TV");
}

/// OnExit - respond to a request to exit the program.
///
void TVFrame::OnExit (wxCommandEvent &event) {
  myApp->Quit ();
}

/// OnExit - respond to a request to display the About box.
///
void TVFrame::OnAbout (wxCommandEvent &event) {
  wxMessageBox("LLVM Visualization Tool\n\n"
               "By Misha Brukman, Tanya Brethour, and Brian Gaeke\n"
               "Copyright (C) 2004 University of Illinois at Urbana-Champaign\n"
               "http://llvm.cs.uiuc.edu", "About LLVM-TV");
}

/// OnRefresh - respond to a request to refresh the list
///
void TVFrame::OnRefresh (wxCommandEvent &event) {
  refreshSnapshotList ();
}

void TVFrame::OnOpen (wxCommandEvent &event) {
  wxFileDialog d (this, "Choose a bytecode file to display");
  int result = d.ShowModal ();
  if (result == wxID_CANCEL) return;
  // FIXME: the rest of this method can be moved into the "snapshots
  // list" object
  std::string command = std::string("cp ") + std::string(d.GetPath ().c_str ()) + " " + snapshotsPath;
  system (command.c_str ());
  refreshSnapshotList ();
}

void TVFrame::Resize() {
  wxSize size = GetClientSize();
  myTreeCtrl->SetSize(0, 0, size.x, 2*size.y/3);
}

// This method of TVApplication is placed in this file so that it can
// be instantiated by all its callers.
template<class Grapher>
void TVApplication::OpenGraphView (TVTreeItemData *item) {
  PictureFrame *wind = new PictureFrame (this);
  allMyWindows.push_back (wind);
  ItemDisplayer *drawer = new Grapher (wind);
  wind->SetTitle (drawer->getDisplayTitle (item).c_str ());
  allMyDisplayers.push_back (drawer);
  drawer->displayItem (item);
}

void TVFrame::CallGraphView(wxCommandEvent &event) {
  // Get the selected LLVM object and open up a new call graph view window.
  myApp->OpenGraphView<CallGraphDrawer> (myTreeCtrl->GetSelectedItemData ());
}

void TVFrame::CFGView(wxCommandEvent &event) {
  // Get the selected LLVM object and open up a new CFG view window.
  myApp->OpenGraphView<CFGGraphDrawer> (myTreeCtrl->GetSelectedItemData ());
}

void TVFrame::BUDSView(wxCommandEvent &event) {
  // Get the selected LLVM object and open up a new BUDS view window.
  myApp->OpenGraphView<BUGraphDrawer> (myTreeCtrl->GetSelectedItemData ());
}

void TVFrame::TDDSView(wxCommandEvent &event) {
  // Get the selected LLVM object and open up a new TDDS view window.
  myApp->OpenGraphView<TDGraphDrawer> (myTreeCtrl->GetSelectedItemData ());
}

void TVFrame::LocalDSView(wxCommandEvent &event) {
  // Get the selected LLVM object and open up a new Local DS view window.
  myApp->OpenGraphView<LocalGraphDrawer> (myTreeCtrl->GetSelectedItemData ());
}

void TVFrame::CodeView(wxCommandEvent &event) {
  // Get the selected LLVM object and open up a new CodeViewer window.
  myApp->OpenGraphView<TVCodeViewer> (myTreeCtrl->GetSelectedItemData ());
}

BEGIN_EVENT_TABLE (TVFrame, wxFrame)
  EVT_MENU (wxID_OPEN, TVFrame::OnOpen)
  EVT_MENU (LLVM_TV_REFRESH, TVFrame::OnRefresh)
  EVT_MENU (wxID_EXIT, TVFrame::OnExit)

  EVT_MENU (wxID_HELP_CONTENTS, TVFrame::OnHelp)
  EVT_MENU (wxID_ABOUT, TVFrame::OnAbout)

  EVT_MENU (LLVM_TV_CALLGRAPHVIEW, TVFrame::CallGraphView)
  EVT_MENU (LLVM_TV_CFGVIEW, TVFrame::CFGView)
  EVT_MENU (LLVM_TV_BUDS_VIEW, TVFrame::BUDSView)
  EVT_MENU (LLVM_TV_TDDS_VIEW, TVFrame::TDDSView)
  EVT_MENU (LLVM_TV_LOCALDS_VIEW, TVFrame::LocalDSView)
  EVT_MENU (LLVM_TV_CODEVIEW, TVFrame::CodeView)
END_EVENT_TABLE ()

BEGIN_EVENT_TABLE(TVTreeCtrl, wxTreeCtrl)
  EVT_TREE_SEL_CHANGED(LLVM_TV_TREE_CTRL, TVTreeCtrl::OnSelChanged)
END_EVENT_TABLE ()
