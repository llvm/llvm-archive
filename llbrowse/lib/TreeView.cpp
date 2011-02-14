/*
 * TreeItems.cpp
 */

#include "llvm/LLVMContext.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Constants.h"
#include "llvm/Module.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/TypeSymbolTable.h"

#include "llbrowse/Formatting.h"
#include "llbrowse/TreeView.h"
#include "llbrowse/DetailsView.h"
#include "llbrowse/Resources.h"

#include "wx/sstream.h"

using namespace llvm;

namespace {

wxString GetLinkageName(GlobalValue::LinkageTypes linkage) {
  switch (linkage) {
    case GlobalValue::ExternalLinkage:
      return _("ExternalLinkage");
    case GlobalValue::AvailableExternallyLinkage:
      return _("AvailableExternallyLinkage");
    case GlobalValue::LinkOnceAnyLinkage:
      return _("LinkOnceAnyLinkage");
    case GlobalValue::LinkOnceODRLinkage:
      return _("LinkOnceODRLinkage");
    case GlobalValue::WeakAnyLinkage:
      return _("WeakAnyLinkage");
    case GlobalValue::WeakODRLinkage:
      return _("WeakODRLinkage");
    case GlobalValue::AppendingLinkage:
      return _("AppendingLinkage");
    case GlobalValue::InternalLinkage:
      return _("InternalLinkage");
    case GlobalValue::PrivateLinkage:
      return _("PrivateLinkage");
    case GlobalValue::LinkerPrivateLinkage:
      return _("LinkerPrivateLinkage");
    case GlobalValue::LinkerPrivateWeakLinkage:
      return _("LinkerPrivateWeakLinkage");
    case GlobalValue::LinkerPrivateWeakDefAutoLinkage:
      return _("LinkerPrivateWeakDefAutoLinkage");
    case GlobalValue::DLLImportLinkage:
      return _("DLLImportLinkage");
    case GlobalValue::DLLExportLinkage:
      return _("DLLExportLinkage");
    case GlobalValue::ExternalWeakLinkage:
      return _("ExternalWeakLinkage");
    case GlobalValue::CommonLinkage:
      return _("CommonLinkage");
    default:
      assert(false && "Invalid linkage type!");
      return wxString();
  }
}

wxString GetDITagName(unsigned tag) {
  return wxString::From8BitData(llvm::dwarf::TagString(tag));
}

}

//===----------------------------------------------------------------------===//
// TreeItemBase Implementation
//===----------------------------------------------------------------------===//

IMPLEMENT_DYNAMIC_CLASS(TreeView, wxTreeCtrl)

void TreeView::SetModule(const Module* module, const wxString& rootName) {
  DeleteAllItems();
  rootItem_ = new ModuleItem(module);
  wxTreeItemId rootId = AddRoot(rootName, 0, -1, rootItem_);
  rootItem_->CreateChildren(this, rootId);

  // Root item is initially expanded
  Expand(rootId);
}

//===----------------------------------------------------------------------===//
// TreeItemBase Implementation
//===----------------------------------------------------------------------===//

wxTreeItemId TreeItemBase::CreateChild(
    wxTreeCtrl* tree,
    const wxTreeItemId & parent,
    TreeItemBase* itemData)
{
  wxTreeItemId id = tree->AppendItem(
      parent, itemData->GetCaption(), itemData->GetIcon(), -1, itemData);
  if (itemData->CanCreateChildren()) {
    tree->SetItemHasChildren(id, true);
  }

  return id;
}

//===----------------------------------------------------------------------===//
// ModuleItem Implementation
//===----------------------------------------------------------------------===//

int ModuleItem::GetIcon() const {
  return Resources::ICON_MODULE;
}

void ModuleItem::CreateChildren(wxTreeCtrl* tree, const wxTreeItemId & id) {
  CreateChild(tree, id, new TypeListItem(module_));
  CreateChild(tree, id, new FunctionListItem(module_));
  CreateChild(tree, id, new VariableListItem(module_));
  CreateChild(tree, id, new AliasListItem(module_));
  CreateChild(tree, id, new NamedMetaListItem(module_));
  CreateChild(tree, id, new DebugSymbolsRootItem(module_));
}

void ModuleItem::ShowDetails(DetailsView* detailsView) {
  if (!module_->getModuleIdentifier().empty()) {
    detailsView->Add(_("Module ID"), module_->getModuleIdentifier());
  }

  if (!module_->getDataLayout().empty()) {
    detailsView->Add(_("Data Layout"), module_->getDataLayout());
  }

  if (!module_->getTargetTriple().empty()) {
    detailsView->Add(_("Target Triple"), module_->getTargetTriple());
  }

  // For some reason this causes the app to crash in the wxWidgets translation
  // string lookup code, but I can't find anything wrong with it.
  switch (module_->getEndianness()) {
    case Module::AnyEndianness:
      detailsView->Add(_("Byte Order"), _("Any"));
      break;
    case Module::LittleEndian:
      detailsView->Add(_("Byte Order"), _("Little Endian"));
      break;
    case Module::BigEndian:
      detailsView->Add(_("Byte Order"), _("Big Endian"));
      break;
  }

  /// An enumeration for describing the size of a pointer on the target machine.
//  enum PointerSize { AnyPointerSize, Pointer32, Pointer64 };
//  PointerSize getPointerSize() const;
}

//===----------------------------------------------------------------------===//
// ListItem Implementation
//===----------------------------------------------------------------------===//

int ListItem::GetIcon() const {
  return Resources::ICON_FOLDER;
}

//===----------------------------------------------------------------------===//
// TypeListItem Implementation
//===----------------------------------------------------------------------===//

void TypeListItem::CreateChildren(
    wxTreeCtrl* tree, const wxTreeItemId & id) {
  const TypeSymbolTable & types = module_->getTypeSymbolTable();
  for (TypeSymbolTable::const_iterator it = types.begin(), itEnd = types.end();
      it != itEnd; ++it) {
    CreateChild(tree, id,
        new TypeItem(module_, it->second, toWxStr(it->first)));
  }
  tree->SortChildren(id);
}

bool TypeListItem::CanCreateChildren() const {
  return !module_->getTypeSymbolTable().empty();
}

//===----------------------------------------------------------------------===//
// FunctionListItem Implementation
//===----------------------------------------------------------------------===//

void FunctionListItem::CreateChildren(
    wxTreeCtrl* tree, const wxTreeItemId & id) {
  const Module::FunctionListType & funcs = module_->getFunctionList();
  for (Module::FunctionListType::const_iterator it = funcs.begin(),
      itEnd = funcs.end();
      it != itEnd; ++it) {
    CreateChild(tree, id, new FunctionItem(module_, it));
  }
  tree->SortChildren(id);
}

bool FunctionListItem::CanCreateChildren() const {
  return !module_->getFunctionList().empty();
}

//===----------------------------------------------------------------------===//
// VariableListItem Implementation
//===----------------------------------------------------------------------===//

void VariableListItem::CreateChildren(
    wxTreeCtrl* tree, const wxTreeItemId & id) {
  const Module::GlobalListType & vars = module_->getGlobalList();
  for (Module::GlobalListType::const_iterator it = vars.begin(),
      itEnd = vars.end();
      it != itEnd; ++it) {
    CreateChild(tree, id, new VariableItem(module_, it));
  }
  tree->SortChildren(id);
}

bool VariableListItem::CanCreateChildren() const {
  return !module_->getGlobalList().empty();
}

//===----------------------------------------------------------------------===//
// AliasListItem Implementation
//===----------------------------------------------------------------------===//

void AliasListItem::CreateChildren(
    wxTreeCtrl* tree, const wxTreeItemId & id) {
}

bool AliasListItem::CanCreateChildren() const {
  return !module_->getAliasList().empty();
}

//===----------------------------------------------------------------------===//
// NamedMetaListItem Implementation
//===----------------------------------------------------------------------===//

void NamedMetaListItem::CreateChildren(
    wxTreeCtrl* tree, const wxTreeItemId & id) {
  typedef Module::const_named_metadata_iterator const_iterator;
  for (const_iterator it = module_->named_metadata_begin(),
      itEnd = module_->named_metadata_end();
      it != itEnd; ++it) {
    CreateChild(tree, id, new NamedMDNodeItem(module_, it));
  }
  tree->SortChildren(id);
}

bool NamedMetaListItem::CanCreateChildren() const {
  return module_->named_metadata_begin() != module_->named_metadata_end();
}

//===----------------------------------------------------------------------===//
// TypeItem Implementation
//===----------------------------------------------------------------------===//

int TypeItem::GetIcon() const {
  return Resources::ICON_TYPE;
}

wxString TypeItem::GetCaption() const {
  wxStringOutputStream strm;
  wxTextOutputStream tstrm(strm);
  tstrm << typeName_ << _(" = ");
  printTypeExpansion(tstrm, module_, type_, 1);
  return strm.GetString();
}

void TypeItem::CreateChildren(wxTreeCtrl* tree, const wxTreeItemId & id) {
  unsigned ct = type_->getNumContainedTypes();
  for (unsigned i = 0; i < ct; ++i) {
    CreateChild(tree, id, new TypeRefItem(module_, type_->getContainedType(i)));
  }
}

bool TypeItem::CanCreateChildren() const {
  return type_->getNumContainedTypes() > 0;
}

void TypeItem::ShowDetails(DetailsView* detailsView) {
}

//===----------------------------------------------------------------------===//
// TypeRefItem Implementation
//===----------------------------------------------------------------------===//

int TypeRefItem::GetIcon() const {
  return Resources::ICON_TYPEREF;
}

wxString TypeRefItem::GetCaption() const {
  wxStringOutputStream strm;
  wxTextOutputStream tstrm(strm);
  printType(tstrm, module_, type_, 1);
  return strm.GetString();
}

void TypeRefItem::CreateChildren(wxTreeCtrl* tree, const wxTreeItemId & id) {
  // Auto-deref pointer types
  const Type* ty = type_;
  if (isa<PointerType>(ty)) {
    ty = ty->getContainedType(0);
  }
  unsigned ct = ty->getNumContainedTypes();
  for (unsigned i = 0; i < ct; ++i) {
    CreateChild(tree, id, new TypeRefItem(module_, ty->getContainedType(i)));
  }
}

bool TypeRefItem::CanCreateChildren() const {
  return type_->getNumContainedTypes() > 0;
}

void TypeRefItem::ShowDetails(DetailsView* detailsView) {
}

//===----------------------------------------------------------------------===//
// FunctionItem Implementation
//===----------------------------------------------------------------------===//

int FunctionItem::GetIcon() const {
  return Resources::ICON_FUNCTION;
}

wxString FunctionItem::GetCaption() const {
  wxStringOutputStream strm;
  wxTextOutputStream tstrm(strm);
  printType(tstrm, module_, function_->getFunctionType()->getReturnType(), 1);
  tstrm << _(" ");
  tstrm << toWxStr(function_->getName()) << _("(");
  const Function::ArgumentListType & args = function_->getArgumentList();
  for (Function::ArgumentListType::const_iterator it = args.begin(),
      itEnd = args.end(); it != itEnd; ++it) {
    if (it != args.begin()) {
      tstrm << _(", ");
    }
    printType(tstrm, module_, it->getType(), 1);
  }
  tstrm << _(")");
  // TODO: Function attributes?
  return strm.GetString();
}

void FunctionItem::CreateChildren(wxTreeCtrl* tree, const wxTreeItemId & id) {
  // Expand to params, possibly blocks.
}

bool FunctionItem::CanCreateChildren() const {
  return false; // for now
}

void FunctionItem::ShowDetails(DetailsView* detailsView) {
  detailsView->Add(_("Linkage"), GetLinkageName(function_->getLinkage()));

  if (function_->hasGC()) {
    detailsView->Add(_("GC"), function_->getGC());
  }

  wxArrayString attrs;
  if (function_->doesNotReturn()) {
    attrs.Add(_("NoReturn"));
  }
  if (function_->doesNotThrow()) {
    attrs.Add(_("NoUnwind"));
  }
  if (function_->doesNotAccessMemory()) {
    attrs.Add(_("ReadNone"));
  }
  if (function_->onlyReadsMemory()) {
    attrs.Add(_("ReadOnly"));
  }
  if (function_->hasStructRetAttr()) {
    attrs.Add(_("StructRet"));
  }

  if (attrs.GetCount() > 0) {
    for (unsigned i = 0; i < attrs.GetCount(); ++i) {
      if (i == 0) {
        detailsView->Add(_("Attributes"), attrs[i]);
      } else {
        detailsView->Add(wxString(), attrs[i]);
      }
    }
  }
}

//===----------------------------------------------------------------------===//
// VariableItem Implementation
//===----------------------------------------------------------------------===//

int VariableItem::GetIcon() const {
  return Resources::ICON_VARIABLE;
}

wxString VariableItem::GetCaption() const {
  return toWxStr(var_->getName());
}

void VariableItem::CreateChildren(wxTreeCtrl* tree, const wxTreeItemId & id) {
  CreateChild(tree, id, new TypeRefItem(module_, var_->getType()));
  if (var_->hasInitializer()) {
    CreateChild(tree, id, new ConstantItem(module_, var_->getInitializer()));
  }
}

bool VariableItem::CanCreateChildren() const {
  return true;
}

void VariableItem::ShowDetails(DetailsView* detailsView) {
  detailsView->Add(_("Linkage"), GetLinkageName(var_->getLinkage()));
}

//===----------------------------------------------------------------------===//
// ConstantItem Implementation
//===----------------------------------------------------------------------===//

int ConstantItem::GetIcon() const {
  return Resources::ICON_CONSTANT;
}

wxString ConstantItem::GetCaption() const {
  wxStringOutputStream strm;
  wxTextOutputStream tstrm(strm);
  printConstant(tstrm, module_, val_, 2);
  return strm.GetString();
}

void ConstantItem::CreateChildren(wxTreeCtrl* tree, const wxTreeItemId & id) {
  unsigned cnt = val_->getNumOperands();
  for (unsigned i = 0; i < cnt; ++i) {
    const Constant* v = cast<Constant>(val_->getOperand(i));
    if (const GlobalVariable* gv = dyn_cast<GlobalVariable>(v)) {
      // This should really be a 'VariableRefItem' that doesn't expand.
      CreateChild(tree, id, new VariableItem(module_, gv));
    } else if (const Function* fn = dyn_cast<Function>(v)) {
      // This should really be a 'FunctionRefItem' that doesn't expand.
      CreateChild(tree, id, new FunctionItem(module_, fn));
    } else {
      CreateChild(tree, id, new ConstantItem(
          module_, cast<Constant>(val_->getOperand(i))));
    }
  }
}

bool ConstantItem::CanCreateChildren() const {
  return val_->getNumOperands() > 0;
}

void ConstantItem::ShowDetails(DetailsView* detailsView) {
}

//===----------------------------------------------------------------------===//
// NamedMDNodeItem Implementation
//===----------------------------------------------------------------------===//

int NamedMDNodeItem::GetIcon() const {
  return Resources::ICON_META;
}

wxString NamedMDNodeItem::GetCaption() const {
  return toWxStr(node_->getName());
}

void NamedMDNodeItem::CreateChildren(
    wxTreeCtrl* tree, const wxTreeItemId & id) {
  unsigned ct = node_->getNumOperands();
  for (unsigned i = 0; i < ct; ++i) {
    CreateChild(tree, id, new MetadataNodeItem(module_, node_->getOperand(i)));
  }
}

bool NamedMDNodeItem::CanCreateChildren() const {
  return node_->getNumOperands() > 0;
}

void NamedMDNodeItem::ShowDetails(DetailsView* detailsView) {
}

//===----------------------------------------------------------------------===//
// MetadataNodeItem Implementation
//===----------------------------------------------------------------------===//

int MetadataNodeItem::GetIcon() const {
  return Resources::ICON_META;
}

wxString MetadataNodeItem::GetCaption() const {
  wxStringOutputStream strm;
  wxTextOutputStream tstrm(strm);
  printMetadata(tstrm, nodeVal_, 1);
  return strm.GetString();
}

void MetadataNodeItem::CreateChildren(
    wxTreeCtrl* tree, const wxTreeItemId & id) {
  if (const MDNode* mn = dyn_cast<MDNode>(nodeVal_)) {
    unsigned ct = mn->getNumOperands();
    for (unsigned i = 0; i < ct; ++i) {
      Value* v = mn->getOperand(i);
      if (v == NULL) {
        // ??? a null child node?
      } else if (GlobalVariable* gv = dyn_cast<GlobalVariable>(v)) {
        CreateChild(tree, id, new VariableItem(module_, gv));
      } else if (Function* fn = dyn_cast<Function>(v)) {
        CreateChild(tree, id, new FunctionItem(module_, fn));
      } else if (Constant* c = dyn_cast<Constant>(v)) {
        CreateChild(tree, id, new ConstantItem(module_, c));
      } else {
        CreateChild(tree, id, new MetadataNodeItem(module_, v));
      }
    }
  }
}

bool MetadataNodeItem::CanCreateChildren() const {
  if (const MDNode* mn = dyn_cast<MDNode>(nodeVal_)) {
    return mn->getNumOperands() > 0;
  }
  return false;
}

void MetadataNodeItem::ShowDetails(DetailsView* detailsView) {
}

//===----------------------------------------------------------------------===//
// DebugSymbolsRootItem Implementation
//===----------------------------------------------------------------------===//

DebugSymbolsRootItem::DebugSymbolsRootItem(
    const llvm::Module* module)
  : ListItem(module)
{
  diFinder_.processModule(*const_cast<Module *>(module));
}

void DebugSymbolsRootItem::CreateChildren(
    wxTreeCtrl* tree, const wxTreeItemId & id) {
  if (diFinder_.compile_unit_count() > 0) {
    CreateChild(tree, id,
         new DIEListItem(module_, _("Compile Units"),
             diFinder_.compile_unit_begin(),
             diFinder_.compile_unit_end()));
  }

  if (diFinder_.type_count() > 0) {
    CreateChild(tree, id,
         new DIETypeListItem(module_, _("Types"),
             diFinder_.type_begin(),
             diFinder_.type_end()));
  }

  if (diFinder_.global_variable_count() > 0) {
    CreateChild(tree, id,
         new DIEListItem(module_, _("Global Variables"),
             diFinder_.global_variable_begin(),
             diFinder_.global_variable_end()));
  }

  if (diFinder_.subprogram_count() > 0) {
    CreateChild(tree, id,
         new DIEListItem(module_, _("Subprograms"),
             diFinder_.subprogram_begin(),
             diFinder_.subprogram_end()));
  }
}

bool DebugSymbolsRootItem::CanCreateChildren() const {
  return diFinder_.compile_unit_count() > 0 ||
      diFinder_.type_count() > 0 ||
      diFinder_.global_variable_count() > 0 ||
      diFinder_.subprogram_count() > 0;
}

//===----------------------------------------------------------------------===//
// DIEListItem Implementation
//===----------------------------------------------------------------------===//

void DIEListItem::CreateChildren(
    wxTreeCtrl *tree, const wxTreeItemId& id) {
  for (DebugInfoFinder::iterator it = nodeList_.begin(); it != nodeList_.end();
      ++it) {
    CreateChild(tree, id, new DIEItem(module_, *it));
  }
  tree->SortChildren(id);
}

bool DIEListItem::CanCreateChildren() const {
  return true;
}

//===----------------------------------------------------------------------===//
// DIETypeListItem Implementation
//===----------------------------------------------------------------------===//

void DIETypeListItem::CreateChildren(
    wxTreeCtrl *tree, const wxTreeItemId& id) {
  NodeList classTypes;
  NodeList enumTypes;
  NodeList structTypes;
  NodeList otherTypes;

  for (NodeList::const_iterator it = nodeList_.begin(); it != nodeList_.end();
      ++it) {
    DIType diType(*it);
    switch (diType.getTag()) {
      case llvm::dwarf::DW_TAG_class_type:
        classTypes.push_back(*it);
        break;

      case llvm::dwarf::DW_TAG_enumeration_type:
        enumTypes.push_back(*it);
        break;

      case llvm::dwarf::DW_TAG_structure_type:
        structTypes.push_back(*it);
        break;

      case llvm::dwarf::DW_TAG_base_type:
      case llvm::dwarf::DW_TAG_inheritance:
      case llvm::dwarf::DW_TAG_member:
      case llvm::dwarf::DW_TAG_subroutine_type:
      case llvm::dwarf::DW_TAG_union_type:
      case llvm::dwarf::DW_TAG_array_type:
      case llvm::dwarf::DW_TAG_pointer_type:
        break;

      default:
        otherTypes.push_back(*it);
        break;
    }
  }

  if (!classTypes.empty()) {
    CreateChild(tree, id,
         new DIEListItem(module_, _("Class Types"),
             classTypes.begin(),
             classTypes.end()));
  }

  if (!enumTypes.empty()) {
    CreateChild(tree, id,
         new DIEListItem(module_, _("Enumeration Types"),
             enumTypes.begin(),
             enumTypes.end()));
  }

  if (!structTypes.empty()) {
    CreateChild(tree, id,
         new DIEListItem(module_, _("Structure Types"),
             structTypes.begin(),
             structTypes.end()));
  }

  if (!otherTypes.empty()) {
    CreateChild(tree, id,
         new DIEListItem(module_, _("Other Types"),
             otherTypes.begin(),
             otherTypes.end()));
  }

  tree->SortChildren(id);
}

//===----------------------------------------------------------------------===//
// DIEItem Implementation
//===----------------------------------------------------------------------===//

int DIEItem::GetIcon() const {
  // TODO: Different icons for different DIE types.
  return Resources::ICON_DEBUG;
}

wxString DIEItem::GetCaption() const {
  using namespace llvm::dwarf;
  wxStringOutputStream strm;
  wxTextOutputStream tstrm(strm);
  DIDescriptor diDesc(node_);

  unsigned tag = diDesc.getTag();
  tstrm << _("[") << GetDITagName(tag) << _("] ");
  if (diDesc.isCompileUnit()) {
    DICompileUnit diCU(node_);
    tstrm << _(" ") << toWxStr(diCU.getFilename());
  } else if (diDesc.isGlobalVariable()) {
    DIGlobalVariable diVar(node_);
    tstrm << _(" ") << toWxStr(diVar.getName());
  } else if (diDesc.isSubprogram()) {
    DISubprogram diSubprogram(node_);
    tstrm << _(" ") << toWxStr(diSubprogram.getLinkageName());
  } else if (diDesc.isCompositeType()) {
    DICompositeType diCompType(node_);
    tstrm << _(" ") << toWxStr(diCompType.getName());
  } else if (diDesc.isDerivedType()) {
    DIDerivedType diDerivedType(node_);
    tstrm << _(" ");
    FormatDIType(tstrm, diDerivedType);
  } else if (diDesc.isBasicType()) {
    DIBasicType diBasicType(node_);
    tstrm << _(" ");
    FormatDIType(tstrm, diBasicType);
  } else if (diDesc.isEnumerator()) {
    DIEnumerator diEnum(node_);
    tstrm << _(" ") << toWxStr(diEnum.getName());
  }

  return strm.GetString();
}

void DIEItem::CreateChildren(wxTreeCtrl *tree, const wxTreeItemId& id) {
  DIDescriptor diDesc(node_);
  if (diDesc.isCompileUnit()) {
    // It's expensive to rescan the entire module, but it's only done lazily.
    DebugInfoFinder diFinder;
    diFinder.processModule(*const_cast<Module *>(module_));

    // CU-specific Types
    if (diFinder.type_count() > 0) {
      llvm::SmallVector<llvm::MDNode*, 32> cuTypes;
      for (DebugInfoFinder::iterator it = diFinder.type_begin(),
          itEnd = diFinder.type_end(); it != itEnd; ++it) {
        DIType diType(*it);
        if (diType.getCompileUnit() == node_) {
          cuTypes.push_back(*it);
        }
      }
      if (!cuTypes.empty()) {
        CreateChild(tree, id, new DIETypeListItem(
            module_, _("Types"), cuTypes.begin(), cuTypes.end()));
      }
    }

    // CU-specific Global variables
    if (diFinder.global_variable_count() > 0) {
      llvm::SmallVector<llvm::MDNode*, 32> cuVariables;
      for (DebugInfoFinder::iterator it = diFinder.global_variable_begin(),
          itEnd = diFinder.global_variable_end(); it != itEnd; ++it) {
        DIGlobalVariable diVar(*it);
        if (diVar.getCompileUnit() == node_) {
          cuVariables.push_back(*it);
        }
      }
      if (!cuVariables.empty()) {
        CreateChild(tree, id, new DIEListItem(
            module_, _("Global Variables"),
            cuVariables.begin(), cuVariables.end()));
      }
    }

    // CU-specific Subprograms
    if (diFinder.subprogram_count() > 0) {
      llvm::SmallVector<llvm::MDNode*, 32> cuSubprograms;
      for (DebugInfoFinder::iterator it = diFinder.subprogram_begin(),
          itEnd = diFinder.subprogram_end(); it != itEnd; ++it) {
        DISubprogram diSubprogram(*it);
        if (diSubprogram.getCompileUnit() == node_) {
          cuSubprograms.push_back(*it);
        }
      }
      if (!cuSubprograms.empty()) {
        CreateChild(tree, id, new DIEListItem(
            module_, _("Subprograms"),
            cuSubprograms.begin(), cuSubprograms.end()));
      }
    }
  } else if (diDesc.isSubprogram()) {
    DISubprogram diSubprogram(node_);
    CreateChild(tree, id, new DIEItem(module_, diSubprogram.getType()));
  } else if (diDesc.isGlobalVariable()) {
    DIGlobalVariable diVar(node_);
    CreateChild(tree, id, new DIEItem(module_, diVar.getType()));
  } else if (diDesc.isCompositeType()) {
    DICompositeType diCompType(node_);
    DIArray typeArray = diCompType.getTypeArray();
    unsigned ct = typeArray.getNumElements();
    for (unsigned i = 0; i < ct; ++i) {
      CreateChild(tree, id, new DIEItem(module_, typeArray.getElement(i)));
    }
  } else if (diDesc.isDerivedType()) {
    DIDerivedType diDerivedType(node_);
    CreateChild(tree, id,
        new DIEItem(module_, diDerivedType.getTypeDerivedFrom()));
  }
}

bool DIEItem::CanCreateChildren() const {
  DIDescriptor diDesc(node_);
  if (diDesc.isCompileUnit() ||
      diDesc.isVariable() ||
      diDesc.isGlobalVariable() ||
      diDesc.isSubprogram() ||
      diDesc.isDerivedType()) {
    return true;
  } else if (diDesc.isCompositeType()) {
    DICompositeType diCompType(node_);
    return diCompType.getTypeArray().getNumElements() > 0;
  } else {
    return false;
  }
}

void DIEItem::ShowDetails(DetailsView* detailsView) {
  DIDescriptor diDesc(node_);
  unsigned tag = diDesc.getTag();
  detailsView->Add(_("Tag"), GetDITagName(tag));
  if (diDesc.isCompileUnit()) {
    DICompileUnit diCU(node_);
    detailsView->Add(_("DescriptorType"), _("DICompileUnit"));
    detailsView->Add(_("Name"), diCU.getFilename());
    detailsView->Add(_("Directory"), diCU.getDirectory());
    detailsView->Add(_("Producer"), diCU.getProducer());
    detailsView->Add(_("IsMain"), diCU.isMain());
    detailsView->Add(_("IsOptimized"), diCU.isOptimized());
  } else if (diDesc.isGlobalVariable()) {
    DIGlobalVariable diVar(node_);
    detailsView->Add(_("DescriptorType"), _("DIGlobalVariable"));
    detailsView->Add(_("Name"), diVar.getName());
    detailsView->Add(_("DisplayName"), diVar.getDisplayName());
    detailsView->Add(_("LinkageName"), diVar.getLinkageName());
    ShowCompileUnit(detailsView, diVar.getCompileUnit());
    detailsView->Add(_("Line"), diVar.getLineNumber());
    ShowContext(detailsView, diVar.getContext());
    // Type
    detailsView->Add(_("IsLocalToUnit"), (bool) diVar.isLocalToUnit());
    detailsView->Add(_("IsDefinition"), (bool) diVar.isDefinition());
  } else if (diDesc.isSubprogram()) {
    DISubprogram diSubprogram(node_);
    detailsView->Add(_("DescriptorType"), _("DISubprogram"));
    detailsView->Add(_("Name"), diSubprogram.getName());
    detailsView->Add(_("DisplayName"), diSubprogram.getDisplayName());
    detailsView->Add(_("LinkageName"), diSubprogram.getLinkageName());
    ShowCompileUnit(detailsView, diSubprogram.getCompileUnit());
    detailsView->Add(_("Line"), diSubprogram.getLineNumber());
    ShowContext(detailsView, diSubprogram.getContext());
    // Type
    // Return typename
    detailsView->Add(_("IsLocalToUnit"), diSubprogram.isLocalToUnit());
    detailsView->Add(_("IsDefinition"), diSubprogram.isDefinition());
    // Virtuality
    // Virtual index
    detailsView->Add(_("Artificial"), (bool) diSubprogram.isArtificial());
    detailsView->Add(_("Private"), diSubprogram.isPrivate());
    detailsView->Add(_("Protected"), diSubprogram.isProtected());
    detailsView->Add(_("Explicit"), diSubprogram.isExplicit());
    detailsView->Add(_("Prototyped"), diSubprogram.isPrototyped());
  } else if (diDesc.isEnumerator()) {
    DIEnumerator diEnum(node_);
    detailsView->Add(_("DescriptorType"), _("DIEnumerator"));
    detailsView->Add(_("Name"), diEnum.getName());
  } else if (diDesc.isCompositeType()) {
    DICompositeType diCompType(node_);
    detailsView->Add(_("DescriptorType"), _("DICompositeType"));
    detailsView->Add(_("Name"), diCompType.getName());
    ShowCompileUnit(detailsView, diCompType.getCompileUnit());
    detailsView->Add(_("Line"), diCompType.getLineNumber());
    ShowContext(detailsView, diCompType.getContext());
    detailsView->Add(_("Private"), diCompType.isPrivate());
    detailsView->Add(_("Protected"), diCompType.isProtected());
    detailsView->Add(_("Artificial"), diCompType.isArtificial());
    detailsView->Add(_("Virtual"), diCompType.isVirtual());
    detailsView->Add(_("ForwardDecl"), diCompType.isForwardDecl());
/*
    DIFile getFile() const              { return getFieldAs<DIFile>(3); }
    uint64_t getSizeInBits() const      { return getUInt64Field(5); }
    uint64_t getAlignInBits() const     { return getUInt64Field(6); }
 */
  } else if (diDesc.isDerivedType()) {
    //DIDerivedType diDerivedType(node_);
    detailsView->Add(_("DescriptorType"), _("DIDerivedType"));
  } else if (diDesc.isBasicType()) {
    DIBasicType diBasicType(node_);
    detailsView->Add(_("DescriptorType"), _("DIBasicType"));
    const char * encodingName =
        dwarf::AttributeEncodingString(diBasicType.getEncoding());
    if (encodingName != NULL) {
      detailsView->Add(_("Encoding"), wxString::From8BitData(encodingName));
    }
  }
}

void DIEItem::ShowCompileUnit(DetailsView* detailsView, DICompileUnit cu) {
  detailsView->Add(_("CompileUnit"),
      toWxStr(cu.getDirectory()) + _("/") + toWxStr(cu.getFilename()));
}

void DIEItem::ShowContext(DetailsView* detailsView, DIScope scope) {
  // TODO: Fill out these cases.
  if (scope.isCompileUnit()) {
    detailsView->Add(_("Context"), _("?CompileUnit"));
  } else if (scope.isFile()) {
    detailsView->Add(_("Context"), _("?File"));
  } else if (scope.isNameSpace()) {
    detailsView->Add(_("Context"), _("?namespace"));
  } else if (scope.isSubprogram()) {
    detailsView->Add(_("Context"), _("?SP"));
  } else if (scope.isLexicalBlock()) {
    detailsView->Add(_("Context"), _("?{}"));
  } else if (scope.isType()) {
    wxStringOutputStream strm;
    wxTextOutputStream tstrm(strm);
    FormatDIType(tstrm, DIType(scope));
    detailsView->Add(_("Context"), strm.GetString());
  } else {
    detailsView->Add(_("Context"), _("Unknown scope type"));
  }
}

void DIEItem::FormatDIType(wxTextOutputStream& out, DIType type) const {
  if (type.isCompositeType()) {
    DICompositeType diCompType(type);
    out << toWxStr(type.getName());
  } else if (type.isDerivedType()) {
    DIDerivedType diDerivedType(type);
    DIType diBase = diDerivedType.getTypeDerivedFrom();
    switch (diDerivedType.getTag()) {
      case llvm::dwarf::DW_TAG_inheritance:
        FormatDIType(out, diBase);
        break;

      case llvm::dwarf::DW_TAG_pointer_type:
        FormatDIType(out, diBase);
        out << _("*");
        break;
    }
    (void) diBase;
  } else if (type.isBasicType()) {
    DIBasicType diBasicType(type);
    const char * encodingName =
        dwarf::AttributeEncodingString(diBasicType.getEncoding());
    if (encodingName != NULL) {
      out << toWxStr(encodingName);
    }
  }
}
