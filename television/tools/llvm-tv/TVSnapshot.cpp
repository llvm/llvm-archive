#include "TVSnapshot.h"
#include "llvm/Bytecode/Reader.h"
using namespace llvm;

void TVSnapshot::readBytecodeFile () {
  std::string errorStr;
  M = ParseBytecodeFile (itemName, &errorStr);
  if (!M)
    std::cerr << "Error reading bytecode from '" << itemName << "': "
              << errorStr << "\n";
}

