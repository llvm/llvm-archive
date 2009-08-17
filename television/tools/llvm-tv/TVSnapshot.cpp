//===- TVSnapshots.cpp - Read bytecode file -------------------------------===//
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

#include "TVSnapshot.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm-tv/Config.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
using namespace llvm;

void TVSnapshot::readBytecodeFile () {
  LLVMContext &Context = getGlobalContext();
  std::string ErrorMsg;
  std::auto_ptr<MemoryBuffer> Buffer(MemoryBuffer::getFile(filename.c_str(),
                                                           &ErrorMsg));
  if (Buffer.get()) {
    M = ParseBitcodeFile(Buffer.get(), Context, &ErrorMsg);
  } else {
    errs() << filename << ": could not load into memory.\n";
    errs() << "Reason: " << ErrorMsg << "\n";
    return;
  }

  if (0 == M) {
    errs() << filename << ": bitcode didn't read correctly.\n";
    errs() << "Reason: " << ErrorMsg << "\n";
  }
}
