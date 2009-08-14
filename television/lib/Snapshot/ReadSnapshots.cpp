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

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm-tv/Config.h"
#include "llvm-tv/Support/FileUtils.h"
#include "llvm-tv/Support/Snapshots.h"
#include <string>
#include <vector>
using namespace llvm;

/// ReadSnapshots - load all bytecode files in a directory that haven't yet been
/// slurped in earlier.
///
void llvm::ReadSnapshots(const std::vector<std::string> &oldModules,
                         std::vector<Module*> &NewModules) {
  std::vector<std::string> FileListing;
  GetFilesInDir(snapshotsPath, FileListing);

  LLVMContext &Context = getGlobalContext();

  for (std::vector<std::string>::const_iterator i = FileListing.begin(),
         e = FileListing.end(); i != e; ++i) {
    if (std::find(oldModules.begin(), oldModules.end(), *i) != oldModules.end())
      continue;

    std::string ErrorMessage;
    std::auto_ptr<MemoryBuffer> Buffer(MemoryBuffer::getFile((*i).c_str(),
                                                             &ErrorMessage));
    Module *M = 0;
    if (Buffer.get()) {
      M = ParseBitcodeFile(Buffer.get(), Context, &ErrorMessage);
    } else {
      errs() << *i << ": could not load into memory.\n";
      errs() << "Reason: " << ErrorMessage << "\n";
      continue;
    }

    if (M == 0) {
      errs() << *i << ": bitcode didn't read correctly.\n";
      errs() << "Reason: " << ErrorMessage << "\n";
      continue;
    }
    NewModules.push_back(M);
  }
}
