//===-- TVApplication.cpp - Main application class for llvm-tv ------------===//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#include "CallGraphDrawer.h"
#include "CFGGraphDrawer.h"
#include "CodeViewer.h"
#include "DSAGraphDrawer.h"
#include "PictureFrame.h"
#include "TVApplication.h"
#include "TVTreeItem.h"
#include "TVFrame.h"
#include "llvm-tv/Config.h"
#include <wx/image.h>
#include <cerrno>
#include <fstream>
#include <functional>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

///==---------------------------------------------------------------------==///

static TVApplication *TheApp;

void sigHandler(int sigNum) {
  TheApp->ReceivedSignal();
}

void TVApplication::ReceivedSignal () {
  // Whenever we catch our prearranged signal, refresh the snapshot list.
  myFrame->refreshSnapshotList();
}

/// FatalErrorBox - pop up an error message and quit.
///
void FatalErrorBox (const std::string msg) {
  wxMessageBox(msg.c_str (), "Fatal Error", wxOK | wxICON_ERROR);
  exit (1);
}

static void setUpMenus (wxFrame *frame) {
  wxMenuBar *menuBar = new wxMenuBar ();

  wxMenu *fileMenu = new wxMenu ("", 0);
  fileMenu->Append (wxID_OPEN, "Add module...");
  fileMenu->Append (LLVM_TV_REFRESH, "Refresh list");
  fileMenu->Append (wxID_EXIT, "Quit");
  menuBar->Append (fileMenu, "File");

  wxMenu *viewMenu = new wxMenu ("", 0);
  viewMenu->Append (LLVM_TV_CALLGRAPHVIEW, "View call graph");
  viewMenu->Append (LLVM_TV_CFGVIEW, "View control-flow graph");
  viewMenu->Append (LLVM_TV_BUDS_VIEW, "View BU datastructure graph");
  viewMenu->Append (LLVM_TV_TDDS_VIEW, "View TD datastructure graph");
  viewMenu->Append (LLVM_TV_LOCALDS_VIEW, "View Local datastructure graph");
  viewMenu->Append (LLVM_TV_CODEVIEW, "View code (interactive)");
  menuBar->Append (viewMenu, "View");

  wxMenu *helpMenu = new wxMenu ("", 0);
  helpMenu->Append (wxID_HELP_CONTENTS, "Help with LLVM-TV");
  helpMenu->Append (wxID_ABOUT, "About LLVM-TV");
  menuBar->Append (helpMenu, "Help");

  frame->SetMenuBar (menuBar);
}

IMPLEMENT_APP (TVApplication)

static bool directoryExists (const std::string &dirPath) {
  struct stat stbuf;
  if (stat (dirPath.c_str (), &stbuf) < 0)
    return false;
  return S_ISDIR (stbuf.st_mode);
}

static void ensureDirectoryExists (const std::string &dirPath) {
  if (!directoryExists (dirPath))
    mkdir (dirPath.c_str (), 0777);
}

/// saveMyPID - Save my process ID into a temporary file.
static void saveMyPID () {
  ensureDirectoryExists (llvmtvPath);

  std::ofstream pidFile (llvmtvPID.c_str ());
  if (pidFile.good () && pidFile.is_open ()) {
    pidFile << getpid ();
    pidFile.close ();
  } else {
    std::cerr << "Warning: could not save PID into " << llvmtvPID << "\n";
  }
}

// eraseMyPID - Erase the PID file created by saveMyPID.
static void eraseMyPID () {
  unlink (llvmtvPID.c_str ());
}

void TVApplication::GoodbyeFrom (wxWindow *dyingWindow) {
  std::vector<wxWindow *>::iterator where =
    find (allMyWindows.begin(), allMyWindows.end(), dyingWindow);
  if (where != allMyWindows.end ())
    allMyWindows.erase (where);
}

void TVApplication::Quit () {
  // Destroy all the picture windows, then the toplevel window.
  for_each (allMyWindows.begin (), allMyWindows.end (),
            std::mem_fun (&wxWindow::Destroy));
  myFrame->Destroy ();
}

void TVApplication::OpenCallGraphView (TVTreeItemData *item) {
  std::string title = "Call graph of " + item->getTitle ();
  PictureFrame *wind = new PictureFrame (this, title.c_str ());
  allMyWindows.push_back (wind);
  ItemDisplayer *drawer = new CallGraphDrawer (wind);
  allMyDisplayers.push_back (drawer);
  drawer->displayItem (item);
}

void TVApplication::OpenCFGView (TVTreeItemData *item) {
  std::string title = "Control-flow graph of " + item->getTitle ();
  PictureFrame *wind = new PictureFrame (this, title.c_str ());
  allMyWindows.push_back (wind);
  ItemDisplayer *drawer = new CFGGraphDrawer (wind);
  allMyDisplayers.push_back (drawer);
  drawer->displayItem (item);
}

void TVApplication::OpenBUDSView (TVTreeItemData *item) {
  std::string title = "BU DataStructure " + item->dsGraphName ();
  PictureFrame *wind = new PictureFrame (this, title.c_str ()); 
  allMyWindows.push_back (wind);
  ItemDisplayer *drawer = new BUGraphDrawer (wind);
  allMyDisplayers.push_back (drawer);
  drawer->displayItem (item);
}

void TVApplication::OpenTDDSView (TVTreeItemData *item) {
  std::string title = "TD DataStructure " + item->dsGraphName ();
  PictureFrame *wind = new PictureFrame (this, title.c_str()); 
  allMyWindows.push_back (wind);
  ItemDisplayer *drawer = new TDGraphDrawer (wind);
  allMyDisplayers.push_back (drawer);
  drawer->displayItem (item);
}

void TVApplication::OpenLocalDSView (TVTreeItemData *item) {
  std::string title = "Local DataStructure " + item->dsGraphName ();
  PictureFrame *wind = new PictureFrame (this, title.c_str());
  allMyWindows.push_back (wind);
  ItemDisplayer *drawer = new LocalGraphDrawer (wind);
  allMyDisplayers.push_back (drawer);
  drawer->displayItem (item);
}


void TVApplication::OpenCodeView (Function *F) {
  allMyWindows.push_back(new CodeViewFrame(this, F));
}


bool TVApplication::OnInit () {
  // Save my PID into the file where the snapshot-making pass knows to
  // look for it.
  saveMyPID ();
  atexit (eraseMyPID);

  wxInitAllImageHandlers ();

  // Build top-level window.
  myFrame = new TVFrame (this, "LLVM Visualizer");
  SetTopWindow (myFrame);

  // Build top-level window's menu bar.
  setUpMenus (myFrame);

  // Read the snapshot list out of the given directory,
  // and load the snapshot list view into the frame.
  ensureDirectoryExists (snapshotsPath);
  myFrame->initializeSnapshotListAndView (snapshotsPath);

  // Set up signal handler so that we can get notified when
  // the -snapshot pass hands us new snapshot bytecode files.
  TheApp = this;
  signal(SIGUSR1, sigHandler);

  return true;
}
