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
#include <vector>

namespace llvm {

/// GetNumFilesInDir - returns a count of files in directory
///
unsigned GetNumFilesInDir(const std::string &path);

/// GetFilesInDir - populate vector with a listing of files in directory
///
void GetFilesInDir(const std::string &path, std::vector<std::string> &list);

bool DirectoryExists (const std::string &dirPath);

void EnsureDirectoryExists (const std::string &dirPath);

};

#endif
