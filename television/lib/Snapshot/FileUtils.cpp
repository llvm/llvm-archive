//===- FileUtils.cpp - File system utility functions for snapshotting -----===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file is Unix-specific; to become part of LLVM it needs to be made
// portable for the architectures/systems LLVM supports.
//
//===----------------------------------------------------------------------===//

#include "Support/FileUtils.h"
#include <dirent.h>

namespace llvm {
  /// Returns the number of entries in the directory named PATH.
  ///
  /// FIXME: If we want to be portable, we can use opendir/readdir/closedir()
  /// and stat(), instead of scandir() and alphasort(). Also, this function
  /// will probably not skip directories (better verify this!)
  ///
  unsigned GetNumFilesInDir(const std::string &path) {
    struct dirent **namelist;
    int n = scandir(path.c_str(), &namelist, 0, alphasort);
    if (n < 0)
      perror("scandir");
    else {
      while(n--)
        free(namelist[n]);
      free(namelist);
    }
    return n;
  }
}
