//===- FileUtils.h - File system utility functions for snapshotting -------===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// File-system functionality useful for snapshotting (Unix-specific).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TV_FILEUTILS_H
#define LLVM_TV_FILEUTILS_H

#include <string>

unsigned llvm::GetNumFilesInDir(const std::string &path);

#endif
