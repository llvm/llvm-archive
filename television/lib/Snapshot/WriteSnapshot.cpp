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

#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Bytecode/WriteBytecodePass.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/SystemUtils.h"
#include "llvm-tv/Support/FileUtils.h"
#include "llvm-tv/Config.h"
#include <csignal>
#include <cstdlib>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
using namespace llvm;

extern char **environ;

namespace {
  struct Snapshot : public ModulePass {
    /// getAnalysisUsage - this pass does not require or invalidate any analysis
    ///
    virtual void getAnalysisUsage(AnalysisUsage &AU) {
      AU.setPreservesAll();
    }
    
    /// runOnModule - save the Module in a pre-defined location with our naming
    /// strategy
    bool runOnModule(Module &M);

  private:
    bool sendSignalToLLVMTV();
    std::string findOption(unsigned Idx);
  };

  // Keep our place in the list of passes that opt was run with to give the
  // snapshots appropriate names.
  static unsigned int PassPosition = 1;
  /// The name of the command-line option to invoke the snapshot pass
  const std::string SnapshotCmd = "snapshot";
  RegisterOpt<Snapshot> X("snapshot", "Snapshot a module, signal llvm-tv");
}


/// runOnModule - save snapshot to a pre-defined directory with a consecutive
/// number in the name (for alphabetization) and the name of the pass that ran
/// just before this one. Signal llvm-tv that fresh bytecode file has arrived
/// for consumption.
bool Snapshot::runOnModule(Module &M) {
  // Make sure the snapshots dir exists, which it will unless this
  // is the first time we've ever run the -snapshot pass.
  EnsureDirectoryExists (llvmtvPath);
  EnsureDirectoryExists (snapshotsPath);

  // Assumption: directory only has numbered .bc files, from 0 -> n-1, next one
  // we add will be n.bc . Subtract 2 for "." and ".."
  unsigned numFiles = GetNumFilesInDir(snapshotsPath) - 2;

  std::string Filename(snapshotsPath);
  Filename = Filename + "/" + utostr(numFiles) + "-" +
    findOption(PassPosition++) + ".bc";

  std::ofstream os(Filename.c_str());
  WriteBytecodeToFile(&M, os);
  os.close();

  // Communicate to llvm-tv that we have added a new snapshot
  if (!sendSignalToLLVMTV()) return false;

  // Since we were not successful in sending a signal to an already-running
  // instance of llvm-tv, start a new instance and send a signal to it.
  sys::Path llvmtvExe = FindExecutable("llvm-tv", ""); 
  if (llvmtvExe.isValid() && !llvmtvExe.isEmpty() && llvmtvExe.isFile() &&
      llvmtvExe.canExecute()) {
    int pid = fork();
    // Child process morphs into llvm-tv
    if (!pid) {
      char *argv[1]; argv[0] = 0; 
      if (execve(llvmtvExe.toString().c_str(), argv, environ) == -1) {
        perror("execve");
        return false;
      }
    }
    
    // parent waits for llvm-tv to write out its pid to a file
    // and then sends it a signal
    sleep(3);
    sendSignalToLLVMTV();
  } else
    std::cerr << "Cannot find llvm-tv in the path!\n";

  return false;
}

/// sendSignalToLLVMTV - read pid from file, send signal to llvm-tv process
///
bool Snapshot::sendSignalToLLVMTV() {
  // See if we can open a file with llvm-tv's pid in it
  std::ifstream is(llvmtvPID.c_str());
  int pid = 0;
  if (is.good() && is.is_open())
    is >> pid;
  else
    return true;

  is.close();
  if (pid > 0)
    return (kill(pid, SIGUSR1) == -1);

  return true;
}

/// findOption - read the environment variable OPTPASSES which should contain
/// the arguments passed to opt and select all those that start with a
/// dash. Return the last one before the Idx-th invocation of `-snapshot'
std::string Snapshot::findOption(unsigned Idx) {
  unsigned currIdx = 0;
  char *OptCmdline = getenv("OPTPASSES");
  if (!OptCmdline) return "noenv";
  std::vector<std::string> InputArgv;
  InputArgv.push_back("filler");
  while (*OptCmdline) {
    if (*OptCmdline == '-') {
      size_t len = strcspn(OptCmdline, " ");
      // Do not add in the '-' or the space after the switch
      char *arg = strdup(OptCmdline+1);
      arg[len - 1] = '\0';
      InputArgv.push_back(std::string(arg));
      free(arg);
    }
    OptCmdline += strcspn(OptCmdline, " ") + 1;
  }

  std::vector<std::string>::iterator stringPos = InputArgv.begin();
  while (currIdx < Idx && stringPos != InputArgv.end()) {
    stringPos = std::find(stringPos+1, InputArgv.end(), SnapshotCmd);
    ++currIdx;
  }

  return (stringPos == InputArgv.end()) ? "notfound"
    : (stringPos == InputArgv.begin()) ? "clean" : *--stringPos;
}
