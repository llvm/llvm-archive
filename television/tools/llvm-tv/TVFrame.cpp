//===-- TVFrame.cpp - Main window class for LLVM-TV -----------------------===//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#include "wx/wx.h"
#include "TVFrame.h"

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

///==---------------------------------------------------------------------==///

/// Default ctor - used to set up typical appearance of demo frame
///
TVFrame::TVFrame (const char *title) : wxFrame (NULL, -1, title) {
  // Set up appearance
  CreateStatusBar ();
  SetSize (wxRect (100, 100, 400, 200));
  Show (TRUE);

  // We'll initialize this later...  hack hack hack
  myListCtrl = 0;
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
  wxMessageBox ("the list is supposed to refresh now...");
  refreshSnapshotList ();
  if (myListCtrl)
    myListCtrl->refreshView ();
}

BEGIN_EVENT_TABLE (TVFrame, wxFrame)
  EVT_MENU (wxID_EXIT, TVFrame::OnExit)
  EVT_MENU (wxID_ABOUT, TVFrame::OnAbout)
  EVT_MENU (LLVM_TV_REFRESH, TVFrame::OnRefresh)
END_EVENT_TABLE ()
