#include "TVSnapshot.h"
#include "llvm/Bytecode/Reader.h"
#include "llvm-tv/Config.h"
using namespace llvm;

void TVSnapshot::readBytecodeFile () {
  std::string errorStr;
  std::string FullFilePath = snapshotsPath + "/" + itemName;
  M = ParseBytecodeFile (FullFilePath, &errorStr);
  if (!M)
    std::cerr << "Error reading bytecode from '" << FullFilePath << "': "
              << errorStr << "\n";
}

