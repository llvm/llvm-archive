//===-- TVTreeItem.h - Nodes for the tree view -------------------*- C++ -*-==//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVTREEITEM_H
#define TVTREEITEM_H

#include "llvm/Assembly/CachedWriter.h"
#include "GraphDrawer.h"
#include <wx/wx.h>
#include <wx/treectrl.h>
#include <ostream>
#include <string>

namespace llvm {
  class Function;
  class GlobalValue;
  class Module;
}

/// TVTreeItemData - Base class for LLVM TV Tree Data
///  
class TVTreeItemData : public wxTreeItemData {
public:
  TVTreeItemData(const wxString& desc) : m_desc(desc) { }
  
  void ShowInfo(wxTreeCtrl *tree);
  const wxChar *GetDesc() const { return m_desc.c_str(); }
  virtual void print(std::ostream&) { }
  virtual llvm::Module *getModule() { return 0; }
  virtual llvm::Function *getFunction() { return 0; }
  virtual wxImage *graphOn(GraphDrawer *grapher) { return 0; }
  virtual void printHTML(std::ostream &os) { }
  virtual std::string getTitle () { return "an untitled object"; }
  virtual std::string dsGraphName () { return "graph of " + getTitle (); }
protected:
  void printFunctionHeader(llvm::Function *F);
  void printFunction(llvm::Function *F);
  void printModule(llvm::Module *M);
  llvm::CachedWriter cw;
  wxString m_desc;
};


/// TVTreeModuleItem - Tree Item containing a Module
///  
class TVTreeModuleItem : public TVTreeItemData {
private:
  llvm::Module *myModule;
public:
  TVTreeModuleItem(const wxString& desc, llvm::Module *mod) 
    : TVTreeItemData(desc), myModule(mod) {}
  
  void print(std::ostream &out);
  void printHTML(std::ostream &os);
  wxImage *graphOn(GraphDrawer *grapher) {
    return grapher->drawModuleGraph (myModule);
  }

  virtual std::string getTitle () { return m_desc.c_str(); }
  virtual std::string dsGraphName () { return "globals graph"; }

  llvm::Module *getModule() { return myModule; }
};


/// TVTreeFunctionItem - Tree Item containing a Function
///  
class TVTreeFunctionItem : public TVTreeItemData {
private:
  llvm::Function *myFunc;
public:
  TVTreeFunctionItem(const wxString& desc, llvm::Function *func)
    : TVTreeItemData(desc),  myFunc(func) {}
  
  void print(std::ostream &out);
  void printHTML(std::ostream &os);
  wxImage *graphOn(GraphDrawer *grapher) {
    return grapher->drawFunctionGraph (myFunc); 
  }

  virtual std::string getTitle ();

  llvm::Module *getModule();
  llvm::Function *getFunction() { return myFunc; }
};


#endif
