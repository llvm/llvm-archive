//===-- DSGraphDrawer.h - DSGraph viewing ------------------------*- C++ -*-==//
//
// Classes for viewing DataStructure analysis graphs
//
//===----------------------------------------------------------------------===//

#ifndef DSAGRAPHDRAWER_H
#define DSAGRAPHDRAWER_H

#include "GraphDrawer.h"
#include <wx/wx.h>
#include <string>

namespace llvm {
  class Function;
  class Module;
  class Pass;
}

//===----------------------------------------------------------------------===//

// DSGraphDrawer abstract class
//
class DSGraphDrawer : public GraphDrawer {
protected:
  llvm::Function *F;
  llvm::Module *M;
  wxImage *drawGraphImage();
  virtual llvm::Pass *getFunctionPass(llvm::Function *F) = 0;
  virtual llvm::Pass *getModulePass() = 0;
  virtual std::string getFilename(llvm::Function *F) = 0;
  virtual std::string getFilename(llvm::Module *M) = 0;
public:
  DSGraphDrawer(llvm::Function *_F) : GraphDrawer(), F(_F), M(0) {}
  DSGraphDrawer(llvm::Module *_M) : GraphDrawer(), F(0), M(_M) {}
};

//===----------------------------------------------------------------------===//

// BUGraphDrawer 
//
class BUGraphDrawer : public DSGraphDrawer {
  llvm::Pass *getFunctionPass(llvm::Function *F);
  llvm::Pass *getModulePass();
  std::string getFilename(llvm::Function *F);
  std::string getFilename(llvm::Module *M);
public:
  BUGraphDrawer(llvm::Module *M) : DSGraphDrawer(M) {}
  BUGraphDrawer(llvm::Function *F) : DSGraphDrawer(F) {}
};

//===----------------------------------------------------------------------===//

// TDGraphDrawer 
//
class TDGraphDrawer : public DSGraphDrawer {
  llvm::Pass *getFunctionPass(llvm::Function *F);
  llvm::Pass *getModulePass();
  std::string getFilename(llvm::Function *F);
  std::string getFilename(llvm::Module *M);
public:
  TDGraphDrawer(llvm::Module *M) : DSGraphDrawer(M) {}
  TDGraphDrawer(llvm::Function *F) : DSGraphDrawer(F) {}
};

//===----------------------------------------------------------------------===//

// LocalGraphDrawer 
//
class LocalGraphDrawer : public DSGraphDrawer {
  llvm::Pass *getFunctionPass(llvm::Function *F);
  llvm::Pass *getModulePass();
  std::string getFilename(llvm::Function *F);
  std::string getFilename(llvm::Module *M);
public:
  LocalGraphDrawer(llvm::Module *M) : DSGraphDrawer(M) {}
  LocalGraphDrawer(llvm::Function *F) : DSGraphDrawer(F) {}
};

#endif // DSAGRAPHDRAWER_H
