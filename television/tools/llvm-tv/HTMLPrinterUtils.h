#ifndef HTMLPRINTERUTILS_H
#define HTMLPRINTERUTILS_H

#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace llvm {
  class Constant;
  class Module;
  class SlotCalculator;
  class Type;
  class Value;
}

const llvm::Module *getModuleFromVal(const llvm::Value *V);

llvm::SlotCalculator *createSlotCalculator(const llvm::Value *V);

std::string getLLVMName(const std::string &Name);

void fillTypeNameTable(const llvm::Module *M,
                       std::map<const llvm::Type *, std::string> &TypeNames);

std::string calcTypeName(const llvm::Type *Ty, 
                         std::vector<const llvm::Type *> &TypeStack,
                         std::map<const llvm::Type *, std::string> &TypeNames);

std::ostream &printTypeInt(std::ostream &Out, const llvm::Type *Ty,
                           std::map<const llvm::Type*, std::string> &TypeNames);

void WriteConstantInt(std::ostream &Out, const llvm::Constant *CV, 
                      bool PrintName,
                      std::map<const llvm::Type *, std::string> &TypeTable,
                      llvm::SlotCalculator *Table);

void WriteAsOperandInternal(std::ostream &Out, const llvm::Value *V,
                            bool PrintName,
                            std::map<const llvm::Type*, std::string> &TypeTable,
                            llvm::SlotCalculator *Table);

#endif
