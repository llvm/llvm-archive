//===-- TVSnapshotList.cpp - List of snapshots class for llvm-tv -*- C++ -*--=//
//
// Encapsulates a list of snapshots and the directory they are in.
//
//===----------------------------------------------------------------------===//

#include "TVSnapshotList.h"
#include <dirent.h>


//Each time this function is called the directory is rescanned
//for any new snapshots
bool TVSnapshotList::refreshList() {
    // re-load the list of snapshots
    const char *directoryName = mySnapshotDirName.c_str();
    clearList();
    DIR *d = opendir(directoryName);
    if (!d)
      return false;
    
    while (struct dirent *de = readdir (d))
      if (memcmp(de->d_name, ".", 2) && memcmp(de->d_name, "..", 3))
	addSnapshot(mySnapshotDirName + "/" + de->d_name);
    
    sortList();
    
    closedir (d);
    return true;
}

//Sort the snapshot list
void TVSnapshotList::sortList() {
  sort (mySnapshotList.begin (), mySnapshotList.end ());
}


//Add a snapshot to our vector of snapshots
void TVSnapshotList::addSnapshot(std::string snapshotName) { 
  mySnapshotList.push_back(snapshotName); 
}

