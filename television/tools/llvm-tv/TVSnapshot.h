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
  std::string filename;

  void readBytecodeFile ();
 public:
  TVSnapshot () : M (0), filename ("") { }
  TVSnapshot (const std::string &_name) : M (0), filename (_name) { }
  TVSnapshot (const char *_name) : M (0), filename (_name) { }
  const char *label () const { return basename (filename.c_str ()); }
  unsigned getTimestamp () const { return (unsigned) strtol (label(), 0, 0); }
  bool operator < (const TVSnapshot &s) const {
    return getTimestamp () < s.getTimestamp ();
  }
  Module *getModule () {
    if (!M)
      readBytecodeFile ();
    return M;
  }
};

#endif // TVSNAPSHOT_H
