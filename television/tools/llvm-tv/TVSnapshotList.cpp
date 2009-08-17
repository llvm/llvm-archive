//===-- TVSnapshotList.cpp - List of snapshots class for llvm-tv -*- C++ -*--=//
//
// Encapsulates a list of snapshots and the directory they are in.
//
//===----------------------------------------------------------------------===//

#include "TVSnapshotList.h"
#include "llvm/System/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <dirent.h>
#include <string>
#include <vector>

TVSnapshotList::~TVSnapshotList() {
  for (std::vector<TVSnapshot*>::iterator i = mySnapshotList.begin(),
         e = mySnapshotList.end(); i != e; ++i) {
    delete *i;
  }
  mySnapshotList.clear();
}

// Each time this function is called the directory is rescanned
// for any new snapshots.
bool TVSnapshotList::refreshList() {
  clearList();
  llvm::sys::Path path(mySnapshotDirName);
  std::set<llvm::sys::Path> contents;
  std::string ErrMsg;
  path.getDirectoryContents(contents, &ErrMsg);

  if (!ErrMsg.empty()) {
    errs() << "Error getting contents of dir: " << mySnapshotDirName
           << " : " << ErrMsg << "\n";
    return false;
  }

  for (std::set<llvm::sys::Path>::const_iterator i = contents.begin(),
         e = contents.end(); i != e; ++i) {
    if (!(*i).isDirectory() && (*i).canRead()) {
      errs() << "Processing: " << (*i).toString() << "\n";
      addSnapshot((*i).toString());
    }
  }

  sortList();
  return true;
}

// Sort the snapshot list.
void TVSnapshotList::sortList() {
  sort (mySnapshotList.begin (), mySnapshotList.end ());
}

// Add a snapshot to our vector of snapshots.
void TVSnapshotList::addSnapshot(const std::string &snapshotName) {
  mySnapshotList.push_back(new TVSnapshot(snapshotName));
}
