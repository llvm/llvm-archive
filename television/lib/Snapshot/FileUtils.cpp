//===- FileUtils.cpp - File system utility functions for snapshotting -----===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file is Unix-specific; to become part of LLVM it needs to be made
// portable for the architectures/systems LLVM supports.
//
//===----------------------------------------------------------------------===//

#include "llvm-tv/Support/FileUtils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

/// Returns the number of entries in the directory named PATH.
///
/// FIXME: If we want to be portable, we can use opendir/readdir/closedir()
/// and stat(), instead of scandir() and alphasort(). Also, this function
/// will probably not skip directories (better verify this!)
///
unsigned llvm::GetNumFilesInDir(const std::string &path) {
  struct dirent **namelist;
  int n = scandir(path.c_str(), &namelist, 0, alphasort);
  int num = n;
  if (n < 0)
    perror("scandir");
  else {
    while(n--)
      free(namelist[n]);
    free(namelist);
  }
  return num;
}


/// GetFilesInDir - returns a listing of files in directory
///
void llvm::GetFilesInDir(const std::string &path,
                         std::vector<std::string> &list)
{
  struct dirent **namelist;
  int n = scandir(path.c_str(), &namelist, 0, alphasort);
  if (n < 0)
    perror("scandir");
  else {
    while(n--) {
      list.push_back(std::string(namelist[n]->d_name));
      free(namelist[n]);
    }
    free(namelist);
  }
}

bool llvm::DirectoryExists (const std::string &dirPath) {
  struct stat stbuf;
  if (stat (dirPath.c_str (), &stbuf) < 0)
    return false;
  return S_ISDIR (stbuf.st_mode);
}

void llvm::EnsureDirectoryExists (const std::string &dirPath) {
  if (!DirectoryExists (dirPath))
    mkdir (dirPath.c_str (), 0777);
}

