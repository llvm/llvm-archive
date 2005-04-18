//===-- TVTreeItem.h - Nodes for the tree view -------------------*- C++ -*-==//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVTREEITEM_H
#define TVTREEITEM_H

#include "GraphDrawer.h"
#include "CodeViewer.h"
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
  const wxChar *GetDesc() const { return m_desc.c_str(); }
  virtual void print(std::ostream&) { }
  virtual wxImage *graphOn(GraphDrawer *grapher) { return 0; }
  virtual void viewCodeOn(TVCodeViewer *viewer) { }
  virtual std::string getTitle () { return "an untitled object"; }
  virtual std::string dsGraphName () { return "graph of " + getTitle (); }

protected:
  TVTreeItemData(const wxString& desc) : m_desc(desc) { }
  void printFunctionHeader(llvm::Function *F);
  void printFunction(llvm::Function *F);
  void printModule(llvm::Module *M);
  wxString m_desc;
};

/// TVTreeRootItem - Tree Item representing root of the hierarchy (Singleton)
///
class TVTreeRootItem : public TVTreeItemData {
public:
  static TVTreeRootItem* instance();
protected:
  TVTreeRootItem(const wxString& desc) : TVTreeItemData(desc) {}
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
  wxImage *graphOn(GraphDrawer *grapher) {
    return grapher->drawModuleGraph (myModule);
  }
  void viewCodeOn(TVCodeViewer *viewer) { viewer->viewModuleCode (myModule); }

  virtual std::string getTitle () { return m_desc.c_str(); }
  virtual std::string dsGraphName () { return "globals graph"; }
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
  wxImage *graphOn(GraphDrawer *grapher) {
    return grapher->drawFunctionGraph (myFunc); 
  }
  void viewCodeOn(TVCodeViewer *viewer) { viewer->viewFunctionCode (myFunc); }

  virtual std::string getTitle ();
};

#endif
