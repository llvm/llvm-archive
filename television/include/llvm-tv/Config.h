#ifndef LLVM_TV_CONFIG_H
#define LLVM_TV_CONFIG_H

#include <string>

namespace {

  // To make sure we don't collide if working on the same machine,
  // the llvm-tv data directory is user-specific
  const std::string llvmtvPath    = "/tmp/llvm-tv-" + 
                                    std::string(getenv("USER"));
  const std::string snapshotsPath = llvmtvPath + "/snapshots";
  const std::string llvmtvPID     = llvmtvPath + "/llvm-tv.pid";

}

#endif
