//===- Snapshot.cpp - Snapshot Module views and communicate with llvm-tv --===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// * If llvm-tv is not running, start it.
// * Send update to llvm-tv each time this pass is called on the command line,
//   e.g.  opt -snapshot -licm -snapshot -gcse -snapshot ... 
//
//===----------------------------------------------------------------------===//

#include "Support/StringExtras.h"
#include "Support/FileUtils.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Bytecode/WriteBytecodePass.h"
#include <dirent.h>
#include <fstream>
#include <string>
#include <vector>
using namespace llvm;

namespace {
  
  const std::string bytecodePath = "/tmp/llvm-tv/snapshots";

  struct Snapshot : public Pass {

    virtual void getAnalysisUsage(AnalysisUsage &AU) {
      AU.setPreservesAll();
    }

    bool run(Module &M);
  };

  RegisterOpt<Snapshot> X("snapshot", "Snapshot a module, update llvm-tv view");
}

bool Snapshot::run(Module &M) {
  // Assumption: directory only has numbered .bc files, from 0 -> n-1, next one
  // we add will be n.bc
  unsigned numFiles = GetNumFilesInDir(bytecodePath);

  std::string Filename (bytecodePath);
  Filename = Filename + utostr (numFiles) + ".bc";

  std::ofstream os (Filename.c_str ());
  WriteBytecodeToFile(&M, os);
  os.close();

  // Communicate to llvm-tv that we have added a new snapshot
  // ???

  return false;
}
