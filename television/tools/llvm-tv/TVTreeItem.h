//===-- TVTreeItem.h - Nodes for the tree view -------------------*- C++ -*-==//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVTREEITEM_H
#define TVTREEITEM_H

#include "wx/wx.h"
#include "wx/treectrl.h"
#include <ostream>
#include <sstream>

namespace llvm {
  class CachedWriter;
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
  virtual void print(std::ostream&) {};
  virtual llvm::Module *getModule() { return 0; }
  virtual llvm::Function *getFunction() { return 0; }
  virtual void printHTML(std::ostream &os) {}
protected:
  void printFunction(llvm::Function *F, llvm::CachedWriter &cw);
  void printModule(llvm::Module *M, llvm::CachedWriter &cw);
private:
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

  llvm::Module *getModule();
  llvm::Function *getFunction() { return myFunc; }
};


#endif
