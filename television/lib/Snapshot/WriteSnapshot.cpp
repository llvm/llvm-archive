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
#include "llvm-tv/Config.h"
#include "Support/FileUtils.h"
#include "Support/StringExtras.h"
#include "Support/SystemUtils.h"
#include <csignal>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
using namespace llvm;

namespace {
  
  struct Snapshot : public Pass {

    virtual void getAnalysisUsage(AnalysisUsage &AU) {
      AU.setPreservesAll();
    }

    bool run(Module &M);

  private:
    bool sendSignalToLLVMTV();

  };

  RegisterOpt<Snapshot> X("snapshot", "Snapshot a module, signal llvm-tv");
}

bool Snapshot::run(Module &M) {
  // Assumption: directory only has numbered .bc files, from 0 -> n-1, next one
  // we add will be n.bc . Subtract 2 for "." and ".."
  unsigned numFiles = GetNumFilesInDir(snapshotsPath) - 2;

  std::string Filename(snapshotsPath);
  Filename = Filename + "/" + utostr(numFiles) + ".bc";

  std::ofstream os(Filename.c_str());
  WriteBytecodeToFile(&M, os);
  os.close();

  // Communicate to llvm-tv that we have added a new snapshot
  if (!sendSignalToLLVMTV()) return false;

  // Since we were not successful in sending a signal to an already-running
  // instance of llvm-tv, start a new instance and send a signal to it.
  std::string llvmtvExe = FindExecutable("llvm-tv.exe", ""); 
  if (llvmtvExe != "" && isExecutableFile(llvmtvExe)) {
    int pid = fork();
    // Child process morphs into llvm-tv
    if (!pid) {
      char *argv[1]; argv[0] = 0; 
      char *envp[1]; envp[0] = 0;
      if (execve(llvmtvExe.c_str(), argv, envp) == -1) {
        perror("execve");
        return false;
      }
    }
    
    // parent waits for llvm-tv to write out its pid to a file
    // and then sends it a signal
    sleep(3);
    sendSignalToLLVMTV();
  }
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

  if (pid > 0)
    if (kill(pid, SIGUSR1) == -1)
      return true;
    else
      return false;

  return true;
}

