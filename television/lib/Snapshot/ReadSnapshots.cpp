//===- ReadSnapshots.cpp - View snapshots that were saved previously ------===//
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

#include "llvm/Module.h"
#include "llvm/Bytecode/Reader.h"
#include "llvm-tv/Support/FileUtils.h"
#include "llvm-tv/Support/Snapshots.h"
#include <string>
#include <vector>
using namespace llvm;

namespace {
  const std::string bytecodePath = "/tmp/llvm-tv/snapshots";
}

/// ReadSnapshots - load all bytecode files in a directory that haven't yet been
/// slurped in earlier.
///
void llvm::ReadSnapshots(std::vector<std::string> &oldModules,
                         std::vector<Module*> NewModules) {
  std::string Filename (bytecodePath);
  std::vector<std::string> FileListing;
  GetFilesInDir(bytecodePath, FileListing);

  for (std::vector<std::string>::iterator i = FileListing.begin(),
         e = FileListing.end(); i != e; ++i)
    if (std::find(oldModules.begin(), oldModules.end(), *i) != oldModules.end())
      NewModules.push_back(ParseBytecodeFile(*i));
}
