//===-- TVFrame.cpp - Main window class for LLVM-TV -----------------------===//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#include "wx/wx.h"
#include "TVFrame.h"
#include <sstream>
#include "llvm/Assembly/Writer.h"

/// refreshView - Make sure the display is up-to-date with respect to
/// the list.
///
void TVListCtrl::refreshView () {
  // Clear out the list and then re-add all the items.
  int index = 0;
  ClearAll ();
  for (Items::iterator i = itemList.begin (), e = itemList.end ();
	   i != e; ++i) {
	InsertItem (index, i->label ());
	++index;
  }
}

/// TreeCtrl constructor that creates the root and adds it to the tree
///
TVTreeCtrl::TVTreeCtrl(wxWindow *parent, const wxWindowID id,
                       const wxPoint& pos, const wxSize& size,
                       long style)
  : wxTreeCtrl(parent, id, pos, size, style) {
  wxTreeItemId rootId = AddRoot(wxT("Snapshots"),
				-1, -1, new TVTreeItemData(wxT("Snapshot Root")));
  
  SetItemImage(rootId, TreeCtrlIcon_FolderOpened, wxTreeItemIcon_Expanded);
}


/// TreeCtrl desconstructor
///      
TVTreeCtrl::~TVTreeCtrl() {
}

/// AddSnapshotsToTree - Given a list of snapshots the tree is populated
///
void TVTreeCtrl::AddSnapshotsToTree(std::vector<TVSnapshot> &list) {
  
  wxTreeItemId rootId = GetRootItem();
  for (std::vector<TVSnapshot>::iterator I = list.begin(), E = list.end();
       I != E; ++I) {
    
    // Get the Module associated with this snapshot
    Module *M = I->getModule();
    
    wxTreeItemId id = AppendItem(rootId, I->label(), -1, -1,
                                 new TVTreeModuleItem(I->label(), M));

    // Loop over functions in the module and add them to the tree
    for (Module::iterator I = M->begin(), E = M->end(); I != E; ++I) {
      wxTreeItemId childID =
        AppendItem(id, ((Function*)I)->getName().c_str(), -1, -1,
                   new TVTreeFunctionItem(((Function*)I)->getName().c_str(),I));
    }
    
  }   
}

/// updateSnapshotList - Update the tree with the current snapshot list
///
void TVTreeCtrl::updateSnapshotList(std::vector<TVSnapshot>& list) {
  DeleteChildren(GetRootItem());
  AddSnapshotsToTree(list);
}

/// updateTextDisplayed  - Updates text with the data from the item selected
///
void TVTreeCtrl::updateTextDisplayed() {
  // Get parent and then the text window, then get the selected LLVM object.
  wxTextCtrl *textDisplay = (wxTextCtrl*)
    ((wxSplitterWindow*) GetParent())->GetWindow2();
  TVTreeItemData *item = (TVTreeItemData*)GetItemData(GetSelection());

  // Display the assembly language for the selected LLVM object in the
  // right-hand pane.
  std::ostringstream Out;
  item->print(Out);
  textDisplay->Clear();
  textDisplay->AppendText(Out.str().c_str());
}

/// OnSelChanged - Trigger the text display to be updated with the new
/// item selected
void TVTreeCtrl::OnSelChanged(wxTreeEvent &event) {
  updateTextDisplayed();
}

///==---------------------------------------------------------------------==///

/// Default ctor - used to set up typical appearance of demo frame
///
TVFrame::TVFrame (const char *title) : wxFrame (NULL, -1, title) {
  // Set up appearance
  CreateStatusBar ();
  SetSize (wxRect (100, 100, 400, 200));
  Show (TRUE);
  splitterWindow = new wxSplitterWindow(this, LLVM_TV_SPLITTER_WINDOW,
                                        wxDefaultPosition, wxDefaultSize,
                                        wxSP_3D);

  // Create tree view of snapshots
  CreateTree(wxTR_HIDE_ROOT | wxTR_DEFAULT_STYLE | wxSUNKEN_BORDER,
             mySnapshotList);
  
  // Create static text to display module
  displayText = new wxTextCtrl(splitterWindow, LLVM_TV_TEXT_CTRL,
                               "LLVM Code goes here", wxDefaultPosition,
                               wxDefaultSize,
                               wxTE_READONLY | wxTE_MULTILINE | wxHSCROLL);

  // Split window vertically
  splitterWindow->SplitVertically(myTreeCtrl, displayText, 100);
}

/// OnExit - respond to a request to exit the program.
///
void TVFrame::OnExit (wxCommandEvent &event) {
  // They say that to exit the program, you should Destroy the top-level
  // frame.
  Destroy ();
}

/// OnExit - respond to a request to display the About box.
///
void TVFrame::OnAbout (wxCommandEvent &event) {
  wxMessageBox ("This is my demo. Is it not nifty??");
}

/// OnRefresh - respond to a request to refresh the list
///
void TVFrame::OnRefresh (wxCommandEvent &event) {
  // FIXME: Having the list model and the window squashed together into
  // TVFrame sucks. The way it should probably really work is:
  // TVSnapshotList tlm;  // the list model
  // TVListCtrl tlv;      // the list view
  // signal handler catches signal
  // --> calls tlm->changed() 
  //   --> <re-reads directory, refreshes list of snapshots,
  //   -->  kind of like refreshSnapshotList()>
  //   --> calls tlv->redraw()
  //       --> <clears out list of items, re-adds items, OR
  //       -->  adds only changed items, or whatever makes sense,
  //       -->  kind of like TVFrame::refreshView()>
  refreshSnapshotList ();
}

void TVFrame::Resize() {
  wxSize size = GetClientSize();
  myTreeCtrl->SetSize(0, 0, size.x, 2*size.y/3);
}

void TVFrame::CallGraphView(wxCommandEvent &event) {
  // Get the selected LLVM object.
  TVTreeItemData *item =
    (TVTreeItemData *) myTreeCtrl->GetItemData (myTreeCtrl->GetSelection ());

  // Display the assembly language for the selected LLVM object in the
  // right-hand pane.
  std::ostringstream Out;
  Module *M = item->getModule ();
  if (!M) {
    wxMessageBox ("The selected item doesn't have a call graph.");
    return;
  }
  wxMessageBox ("ready to display call graph");
}

void TVFrame::CFGView(wxCommandEvent &event) {
  wxMessageBox ("cfg view goes here");
}

void TVFrame::CreateTree(long style, std::vector<TVSnapshot> &list) {
  myTreeCtrl = new TVTreeCtrl(splitterWindow, LLVM_TV_TREE_CTRL,
                              wxDefaultPosition, wxDefaultSize, style);
  Resize();
}

BEGIN_EVENT_TABLE (TVFrame, wxFrame)
  EVT_MENU (wxID_EXIT, TVFrame::OnExit)
  EVT_MENU (wxID_ABOUT, TVFrame::OnAbout)
  EVT_MENU (LLVM_TV_REFRESH, TVFrame::OnRefresh)
  EVT_MENU (LLVM_TV_CALLGRAPHVIEW, TVFrame::CallGraphView)
  EVT_MENU (LLVM_TV_CFGVIEW, TVFrame::CFGView)
END_EVENT_TABLE ()

BEGIN_EVENT_TABLE(TVTreeCtrl, wxTreeCtrl)
  EVT_TREE_SEL_CHANGED(LLVM_TV_TREE_CTRL, TVTreeCtrl::OnSelChanged)
END_EVENT_TABLE ()
