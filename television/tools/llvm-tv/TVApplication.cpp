//===-- TVApplication.cpp - Main application class for llvm-tv ------------===//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#include "TVApplication.h"
#include "TVFrame.h"
#include "llvm-tv/Config.h"
#include <cerrno>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <unistd.h>

///==---------------------------------------------------------------------==///

static TVFrame *frame;

void sigHandler(int sigNum) {
  printf("llvm-tv: caught update signal!\n");
  frame->refreshSnapshotList();
}

/// FatalErrorBox - pop up an error message (given in printf () form) and quit.
///
static void FatalErrorBox (const char *errfmt, ...) {
  // Just write it to stderr for now; we can make a nice dialog box later.
  va_list ap;
  va_start (ap, errfmt);
  fprintf (stderr,
  "------------------------------------------------------------------------\n");
  fprintf (stderr, "FATAL ERROR: ");
  vfprintf (stderr, errfmt, ap);
  fprintf (stderr, "\n");
  fprintf (stderr,
  "------------------------------------------------------------------------\n");
  va_end (ap);
  exit (1);
}

void TVFrame::refreshSnapshotList () {
  // re-load the list of snapshots
  const char *directoryName = mySnapshotDirectoryName.c_str ();
  mySnapshotList.clear ();
  DIR *d = opendir (directoryName);
  if (!d)
    FatalErrorBox ("trying to open directory %s: %s", directoryName,
                   strerror (errno)); 
  while (struct dirent *de = readdir (d))
    if (memcmp(de->d_name, ".", 2) && memcmp(de->d_name, "..", 3))
      mySnapshotList.push_back (de->d_name);
  closedir (d);

  if (myTreeCtrl != 0)
    myTreeCtrl->updateSnapshotList(mySnapshotList);
}

void TVFrame::initializeSnapshotListAndView (std::string dirName) {
  // Initialize the snapshot list
  mySnapshotDirectoryName = dirName;
  refreshSnapshotList ();

  // Initialize the snapshot list view
  if (myTreeCtrl == 0)
    CreateTree(wxTR_DEFAULT_STYLE | wxSUNKEN_BORDER, mySnapshotList);
  SetStatusText ("Snapshot list has been loaded.");
}

static void setUpMenus (wxFrame *frame) {
  wxMenuBar *menuBar = new wxMenuBar ();

  wxMenu *fileMenu = new wxMenu ("", 0);
  fileMenu->Append (LLVM_TV_REFRESH, "Refresh list");
  fileMenu->Append (wxID_EXIT, "Quit");
  menuBar->Append (fileMenu, "File");

  wxMenu *helpMenu = new wxMenu ("", 0);
  helpMenu->Append (wxID_HELP_CONTENTS, "Help with LLVM-TV");
  helpMenu->Append (wxID_ABOUT, "About LLVM-TV");
  menuBar->Append (helpMenu, "Help");

  frame->SetMenuBar (menuBar);
}

IMPLEMENT_APP (TVApplication)

/// saveMyPID - Save my process ID into a file of the given name.
static void saveMyPID (const std::string &pidfilename) {
  std::ofstream pidFile (pidfilename.c_str ());
  if (pidFile.good () && pidFile.is_open ()) {
    pidFile << getpid ();
    pidFile.close ();
  } else {
    std::cerr << "Warning: could not save PID into " << pidfilename << "\n";
  }
}

bool TVApplication::OnInit () {
  // Save my PID into the file where the snapshot-making pass knows to
  // look for it.
  saveMyPID (llvmtvPID);

  // Build top-level window.
  frame = new TVFrame ("Snapshot List");
  SetTopWindow (frame);

  // Build top-level window's menu bar.
  setUpMenus (frame);

  // Read the snapshot list out of the given directory,
  // and load the snapshot list view into the frame.
  frame->initializeSnapshotListAndView (snapshotsPath);

  signal(SIGUSR1, sigHandler);

  return true;
}


