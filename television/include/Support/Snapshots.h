//===- Snapshots.h - Snapshot utility functions ---------------*- C++ -*---===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TV_SUPPORT_SNAPSHOTS_H
#define LLVM_TV_SUPPORT_SNAPSHOTS_H

#include <string>
#include <vector>

namespace llvm {
  
void ReadSnapshots(std::vector<std::string> &oldModules,
                   std::vector<Module*> NewModules); 

}

#endif
