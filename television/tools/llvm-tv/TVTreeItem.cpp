#include "TVTreeItem.h"
#include "llvm/Module.h"
using namespace llvm;

static TVTreeRootItem* theInstance = 0;

TVTreeRootItem* TVTreeRootItem::instance() {
  if (theInstance == 0) {
    theInstance = new TVTreeRootItem("Snapshot Root");
  }
  return theInstance;
}

void TVTreeModuleItem::print(std::ostream &os) {
  myModule->print(os);
}

void TVTreeFunctionItem::print(std::ostream &os) { 
  myFunc->print(os);
}

std::string TVTreeFunctionItem::getTitle () {
  return myFunc->getName () + "()";
}
