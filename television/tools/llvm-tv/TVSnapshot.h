//===-- TVSnapshot.h - Wrapper class for llvm-tv snapshots -------*- C++ -*-==//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVSNAPSHOT_H
#define TVSNAPSHOT_H

#include <string>
#include <vector>
#include <libgen.h>
#include "llvm/Module.h"
using namespace llvm;

/// TVSnapshot - Wrapper class for snapshots.
///
class TVSnapshot {
  Module *M;
  std::string filename;
  std::string myLabel;

  void readBytecodeFile ();
  void fixLabel () { myLabel = basename ((char*)myLabel.c_str ()) ; }
 public:
  //TVSnapshot () : M (0), filename (), myLabel (filename) { }
  TVSnapshot (const std::string &_name) : M (0), filename (_name), myLabel (filename) { fixLabel(); }
  TVSnapshot (const char *_name) : M (0), filename (_name), myLabel (filename) { fixLabel(); }
  const char *label () const { return myLabel.c_str (); } 
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
