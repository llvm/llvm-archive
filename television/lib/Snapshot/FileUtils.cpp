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

#include "FileUtils.h"
#include <cdirent>

namespace llvm {

  // Fine how many files are in directory using scandir()
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
