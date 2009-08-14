//===- Config.h - LLVM-TV configuration parameters ------------*- C++ -*---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TV_CONFIG_H
#define LLVM_TV_CONFIG_H

#include <cstdlib>
#include <string>

namespace {

// To make sure we don't collide if working on the same machine,
// the llvm-tv data directory is user-specific.
const std::string llvmtvPath    = "/tmp/llvm-tv-" + std::string(getenv("USER"));
const std::string snapshotsPath = llvmtvPath + "/snapshots";
const std::string llvmtvPID     = llvmtvPath + "/llvm-tv.pid";

}

#endif
