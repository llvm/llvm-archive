#include "DSAGraphDrawer.h"
#include "TVTreeItem.h"
#include "GraphPrinters.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Target/TargetData.h"
using namespace llvm;

//===----------------------------------------------------------------------===//

// DSGraphDrawer implementation
wxImage *DSGraphDrawer::drawModuleGraph(Module *M) {
  PassManager PM;
  PM.add(new TargetData("llvm-tv", M));
  PM.add(getModulePass());
  PM.run(*M);
  return buildwxImageFromDotFile(getFilename(M));
}

wxImage *DSGraphDrawer::drawFunctionGraph(Function *F) {
  Module *M = F->getParent();
  PassManager PM;
  PM.add(new TargetData("llvm-tv", M));
  PM.add(getFunctionPass(F));
  PM.run(*M);
  return buildwxImageFromDotFile(getFilename(F));
}

std::string DSGraphDrawer::getDisplayTitle (TVTreeItemData *item) {
  return "DS " + item->dsGraphName ();
}

//===----------------------------------------------------------------------===//

// BUGraphDrawer implementation
Pass *BUGraphDrawer::getFunctionPass(Function *F) {
  return createBUDSFunctionPrinterPass(F);
}

Pass *BUGraphDrawer::getModulePass() {
  return createBUDSModulePrinterPass();
}

std::string BUGraphDrawer::getFilename(Function *F) {
  return "buds." + F->getName() + ".dot";
}

std::string BUGraphDrawer::getFilename(Module *M) {
  return "buds.dot";
}

std::string BUGraphDrawer::getDisplayTitle (TVTreeItemData *item) {
  return "Bottom-up data structure " + item->dsGraphName ();
}

//===----------------------------------------------------------------------===//

// TDGraphDrawer implementation
Pass *TDGraphDrawer::getFunctionPass(Function *F) {
  return createTDDSFunctionPrinterPass(F);
}

Pass *TDGraphDrawer::getModulePass() {
  return createTDDSModulePrinterPass();
}

std::string TDGraphDrawer::getFilename(Function *F) {
  return "tdds." + F->getName() + ".dot";
}

std::string TDGraphDrawer::getFilename(Module *M) {
  return "tdds.dot";
}

std::string TDGraphDrawer::getDisplayTitle (TVTreeItemData *item) {
  return "Top-down data structure " + item->dsGraphName ();
}

//===----------------------------------------------------------------------===//

// LocalGraphDrawer implementation
Pass *LocalGraphDrawer::getFunctionPass(Function *F) {
  return createLocalDSFunctionPrinterPass(F);
}

Pass *LocalGraphDrawer::getModulePass() {
  return createLocalDSModulePrinterPass();
}

std::string LocalGraphDrawer::getFilename(Function *F) {
  return "localds." + F->getName() + ".dot";
}

std::string LocalGraphDrawer::getFilename(Module *M) {
  return "localds.dot";
}

std::string LocalGraphDrawer::getDisplayTitle (TVTreeItemData *item) {
  return "Local data structure " + item->dsGraphName ();
}
