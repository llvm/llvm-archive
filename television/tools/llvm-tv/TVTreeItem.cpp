#include "HTMLMarkup.h"
#include "HTMLPrinter.h"
#include "TVTreeItem.h"
#include "llvm/Module.h"
using namespace llvm;

void TVTreeModuleItem::print(std::ostream &os) {
  myModule->print(os);
}

void TVTreeModuleItem::printHTML(std::ostream &os) {
  if (myModule) {
    cw.setStream(os);
    cw.setModule(myModule);
    cw << CachedWriter::SymTypeOn;
    HTMLMarkup *Simple = createSimpleHTMLMarkup(os);
    HTMLPrinter HP(cw, *Simple);
    HP.visit(*myModule);
  }
}

void TVTreeFunctionItem::print(std::ostream &os) { 
  myFunc->print(os);
}

void TVTreeFunctionItem::printHTML(std::ostream &os) {
  if (myFunc) {
    cw.setStream(os);
    cw.setModule(myFunc->getParent());
    cw << CachedWriter::SymTypeOn;
    HTMLMarkup *Simple = createSimpleHTMLMarkup(os);
    HTMLPrinter HP(cw, *Simple);
    HP.visit(*myFunc);
  }
}

Module* TVTreeFunctionItem::getModule() {
  return myFunc ? myFunc->getParent() : 0; 
}

std::string TVTreeFunctionItem::getTitle () {
  return myFunc->getName () + "()";
}
