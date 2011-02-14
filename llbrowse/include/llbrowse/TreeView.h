/*
 * Tree.h
 */

#ifndef LLBROWSE_TREEVIEW_H
#define LLBROWSE_TREEVIEW_H

#include "llvm/Analysis/DebugInfo.h"

#include "wx/treectrl.h"

// Forward declarations

namespace llvm {
class LLVMContext;
class Module;
class Type;
}

class ModuleItem;
class DetailsView;

/// Tree control class

class TreeView : public wxTreeCtrl {
public:
  TreeView() : wxTreeCtrl() {}
  TreeView(wxWindow* parent) : wxTreeCtrl(parent), rootItem_(NULL) {}

  DECLARE_DYNAMIC_CLASS(TreeView)

  /** Re-create all tree nodes, using 'module' as the root. */
  void SetModule(const llvm::Module* module, const wxString& name);

private:
  ModuleItem* rootItem_;
};

/// Base class for tree items

class TreeItemBase : public wxTreeItemData {
public:
  /** Return the icon index for this tree item. */
  virtual int GetIcon() const = 0;

  /** Return the caption for this tree item. */
  virtual wxString GetCaption() const = 0;

  /** Called when this tree item is expanded for the first time. Subclasses should
      implement this to lazily create the children on the tree item.
   */
  virtual void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId &id) {}

  /** Return true if this item has any child nodes. */
  virtual bool CanCreateChildren() const { return false; }

  /** Called when the tree item is selected. Subclasses should implement this to
      show the details of the item.
   */
  virtual void ShowDetails(DetailsView* detailsView) {}

protected:
  wxTreeItemId CreateChild(
      wxTreeCtrl* tree,
      const wxTreeItemId& parent,
      TreeItemBase* itemData);
};

/// Tree item representing a module

class ModuleItem : public TreeItemBase {
public:
  ModuleItem(const llvm::Module* module) : module_(module) {}

  // Overrides

  int GetIcon() const;
  wxString GetCaption() const { return _("Module"); }
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const { return true; }
  void ShowDetails(DetailsView* detailsView);

private:
  const llvm::Module* const module_;
};

/// Tree item representing a list of elements within a module.

class ListItem : public TreeItemBase {
public:
  ListItem(const llvm::Module* module) : module_(module) {}

  // Overrides

  int GetIcon() const;

protected:
  const llvm::Module* const module_;
};

/// Tree item representing the list of named types within a module

class TypeListItem : public ListItem {
public:
  TypeListItem(const llvm::Module* module) : ListItem(module) {}

  // Overrides

  wxString GetCaption() const { return _("Types"); }
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
};

/// Tree item representing the list of functions within a module

class FunctionListItem : public ListItem {
public:
  FunctionListItem(const llvm::Module* module) : ListItem(module) {}

  // Overrides

  wxString GetCaption() const { return _("Functions"); }
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
};

/// Tree item representing the list of global variables within a module

class VariableListItem : public ListItem {
public:
  VariableListItem(const llvm::Module* module) : ListItem(module) {}

  // Overrides

  wxString GetCaption() const { return _("Global Variables"); }
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
};

/// Tree item representing the list of global aliases within a module

class AliasListItem : public ListItem {
public:
  AliasListItem(const llvm::Module* module) : ListItem(module) {}

  // Overrides

  wxString GetCaption() const { return _("Aliases"); }
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
};

/// Tree item representing the list of named metadata nodes

class NamedMetaListItem : public ListItem {
public:
  NamedMetaListItem(const llvm::Module* module) : ListItem(module) {}

  // Overrides

  wxString GetCaption() const { return _("Metadata"); }
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
};

/// Tree item representing a single type

class TypeItem : public TreeItemBase {
public:
  TypeItem(const llvm::Module* module, const llvm::Type* type,
        const wxString& typeName)
    : module_(module), type_(type), typeName_(typeName) {}

  // Overrides

  int GetIcon() const;
  wxString GetCaption() const;
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
  void ShowDetails(DetailsView* detailsView);

private:
  const llvm::Module* const module_;
  const llvm::Type* const type_;
  const wxString typeName_;
};

/// Tree item representing a reference to a type

class TypeRefItem : public TreeItemBase {
public:
  TypeRefItem(const llvm::Module* module, const llvm::Type* type)
    : module_(module), type_(type) {}

  // Overrides

  int GetIcon() const;
  wxString GetCaption() const;
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
  void ShowDetails(DetailsView* detailsView);

private:
  const llvm::Module* const module_;
  const llvm::Type* const type_;
};

/// Tree item representing a function

class FunctionItem : public TreeItemBase {
public:
  FunctionItem(const llvm::Module* module, const llvm::Function* function)
    : module_(module), function_(function) {}

  // Overrides

  int GetIcon() const;
  wxString GetCaption() const;
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
  void ShowDetails(DetailsView* detailsView);

private:
  const llvm::Module* const module_;
  const llvm::Function* const function_;
};

/// Tree item representing a global variable

class VariableItem : public TreeItemBase {
public:
  VariableItem(const llvm::Module* module, const llvm::GlobalVariable* var)
    : module_(module), var_(var) {}

  // Overrides

  int GetIcon() const;
  wxString GetCaption() const;
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
  void ShowDetails(DetailsView* detailsView);

private:
  const llvm::Module* const module_;
  const llvm::GlobalVariable* const var_;
};

/// Tree item representing a constant

class ConstantItem : public TreeItemBase {
public:
  ConstantItem(const llvm::Module* module, const llvm::Constant* val)
    : module_(module), val_(val) {}

  // Overrides

  int GetIcon() const;
  wxString GetCaption() const;
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
  void ShowDetails(DetailsView* detailsView);

private:
  const llvm::Module* const module_;
  const llvm::Constant* const val_;
};

/// Tree item representing a named metadata node

class NamedMDNodeItem : public TreeItemBase {
public:
  NamedMDNodeItem(const llvm::Module* module, const llvm::NamedMDNode* node)
    : module_(module), node_(node) {}

  // Overrides

  int GetIcon() const;
  wxString GetCaption() const;
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
  void ShowDetails(DetailsView* detailsView);

private:
  const llvm::Module* const module_;
  const llvm::NamedMDNode* const node_;
};

/// Tree item representing a metadata node

class MetadataNodeItem : public TreeItemBase {
public:
  MetadataNodeItem(const llvm::Module* module, const llvm::Value* nodeVal)
    : module_(module), nodeVal_(nodeVal) {}

  // Overrides

  int GetIcon() const;
  wxString GetCaption() const;
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
  void ShowDetails(DetailsView* detailsView);

private:
  const llvm::Module* const module_;
  const llvm::Value* const nodeVal_;
};

/// Tree item representing all DWARF debug info for a module.

class DebugSymbolsRootItem : public ListItem {
public:
  DebugSymbolsRootItem(const llvm::Module* module);

  // Overrides

  wxString GetCaption() const { return _("Debug Symbols"); }
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;

protected:
  typedef llvm::SmallVector<llvm::MDNode*, 32> NodeList;

  llvm::DebugInfoFinder diFinder_;
};

/// Tree item representing a list of DWARF DIEs

class DIEListItem : public ListItem {
public:
  DIEListItem(
      const llvm::Module* module,
      const wxString& caption,
      llvm::DebugInfoFinder::iterator begin,
      llvm::DebugInfoFinder::iterator end)
    : ListItem(module)
    , caption_(caption)
    , nodeList_(begin, end)
  {}

  // Overrides

  wxString GetCaption() const { return caption_; }
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;

protected:
  typedef llvm::SmallVector<llvm::MDNode*, 32> NodeList;

  const wxString caption_;
  NodeList nodeList_;
};

/// Tree item representing a list of DWARF type DIE. This divides
/// the type list into multiple groups representing different
/// kinds of types.

class DIETypeListItem : public DIEListItem {
public:
  DIETypeListItem(
      const llvm::Module* module,
      const wxString& caption,
      llvm::DebugInfoFinder::iterator begin,
      llvm::DebugInfoFinder::iterator end)
    : DIEListItem(module, caption, begin, end)
  {}

  // Overrides

  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
};

/// Tree item representing a single DIE

class DIEItem : public TreeItemBase {
public:
  DIEItem(
      const llvm::Module* module,
      const llvm::MDNode* node)
    : module_(module)
    , node_(node)
  {}

  // Overrides

  int GetIcon() const;
  wxString GetCaption() const;
  void CreateChildren(wxTreeCtrl* tree, const wxTreeItemId& id);
  bool CanCreateChildren() const;
  void ShowDetails(DetailsView* detailsView);

private:
  const llvm::Module* const module_;
  const llvm::MDNode*  node_;

  void ShowCompileUnit(DetailsView* detailsView, llvm::DICompileUnit cu);
  void ShowContext(DetailsView* detailsView, llvm::DIScope scope);
  void FormatDIType(wxTextOutputStream& out, llvm::DIType type) const;
};

#endif // LLBROWSE_TREEVIEW_H
