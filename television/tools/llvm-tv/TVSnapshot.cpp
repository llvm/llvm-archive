#include "TVSnapshot.h"
#include "llvm/Bytecode/Reader.h"
#include "llvm-tv/Config.h"
using namespace llvm;

void TVSnapshot::readBytecodeFile () {
  std::string errorStr;
  M = ParseBytecodeFile (filename, &errorStr);
  if (!M)
    throw std::string ("Error reading bytecode from '" + filename + "': "
                       + errorStr);
}

