//===-- TVSnapshotList.h - List of snapshots class for llvm-tv ---*- C++ -*--=//
//
// Encapsulates a list of snapshots and the directory they are in.
//
//===----------------------------------------------------------------------===//

#ifndef TVSNAPSHOTLIST_H
#define TVSNAPSHOTLIST_H

#include <string>
#include <vector>
#include "TVSnapshot.h"

/// Containts a list of snapshots and the directory they are in.
/// Put into a class to easily pass between the application and frame
///
class TVSnapshotList {
  std::vector<TVSnapshot> mySnapshotList;
  std::string mySnapshotDirName;
public:
  TVSnapshotList(std::string dir) :  mySnapshotDirName(dir) {}
  std::string getSnapshotDirName() { return mySnapshotDirName; }
  void setSnapshotDirName(std::string dir) { mySnapshotDirName = dir; }
  void addSnapshot(std::string snapshotName);
  void clearList() { mySnapshotList.clear(); }
  void sortList();
  bool refreshList();

  //Iterators
  typedef std::vector<TVSnapshot>::iterator iterator;
  iterator begin() { return mySnapshotList.begin(); }
  iterator end() { return mySnapshotList.end(); }

};


#endif 
