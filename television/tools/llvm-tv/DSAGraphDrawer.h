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
  virtual llvm::Pass *getFunctionPass(llvm::Function *F) = 0;
  virtual llvm::Pass *getModulePass() = 0;
  virtual std::string getFilename(llvm::Function *F) = 0;
  virtual std::string getFilename(llvm::Module *M) = 0;
public:
  wxImage *drawFunctionGraph(llvm::Function *F);
  wxImage *drawModuleGraph(llvm::Module *M);
  DSGraphDrawer (wxWindow *parent) : GraphDrawer (parent) { }
  static std::string getDisplayTitle (TVTreeItemData *item);
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
  BUGraphDrawer (wxWindow *parent) : DSGraphDrawer (parent) { }
  static std::string getDisplayTitle (TVTreeItemData *item);
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
  TDGraphDrawer (wxWindow *parent) : DSGraphDrawer (parent) { }
  static std::string getDisplayTitle (TVTreeItemData *item);
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
  LocalGraphDrawer (wxWindow *parent) : DSGraphDrawer (parent) { }
  static std::string getDisplayTitle (TVTreeItemData *item);
};

#endif // DSAGRAPHDRAWER_H
