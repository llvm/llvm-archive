//===-- TVSnapshot.h - Wrapper class for llvm-tv snapshots -------*- C++ -*-==//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVSNAPSHOT_H
#define TVSNAPSHOT_H

#include <string>
#include <vector>
#include "llvm/Module.h"
using namespace llvm;

/// TVSnapshot - Wrapper class for snapshots.
///
class TVSnapshot {
  Module *M;
  void readBytecodeFile ();
 public:
  std::string itemName;
  TVSnapshot () : itemName ("") { }
  TVSnapshot (const std::string &_name) : itemName (_name) { }
  TVSnapshot (const char *_name) : itemName (_name) { }
  const char *label () { return itemName.c_str (); }
  Module *getModule () {
    if (!M)
      readBytecodeFile ();
    return M;
  }
};

#endif // TVSNAPSHOT_H
