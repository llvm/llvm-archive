//===-- AST Bundle Class ----------------------------------------*- C++ -*-===//
//
//                      High Level Virtual Machine (HLVM)
//
// Copyright (C) 2006 Reid Spencer. All Rights Reserved.
//
// This software is free software; you can redistribute it and/or modify it 
// under the terms of the GNU Lesser General Public License as published by 
// the Free Software Foundation; either version 2.1 of the License, or (at 
// your option) any later version.
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for 
// more details.
//
// You should have received a copy of the GNU Lesser General Public License 
// along with this library in the file named LICENSE.txt; if not, write to the 
// Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301 USA
//
//===----------------------------------------------------------------------===//
/// @file hlvm/AST/Bundle.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Bundle
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_BUNDLE_H
#define HLVM_AST_BUNDLE_H

#include <hlvm/AST/Node.h>
#include <hlvm/AST/SymbolTable.h>

namespace hlvm 
{ 

class Type;
class Variable;
class Function;

/// This class is simply a collection of definitions. Things that can be 
/// defined in a bundle include types, global variables, functions, classes,
/// etc. A bundle is the unit of linking and loading. A given compilation unit 
/// may define as many bundles as it desires. When a bundle is loaded, all of 
/// its definitions become active.  Only those things defined in a bundle 
/// participate in linking A Bundle's parent is always the AST node. Each 
/// Bundle has a name and that name forms a namespace for the definitions 
/// within the bundle. Bundles cannot be nested. 
/// @brief AST Bundle Node
class Bundle : public Documentable
{
  /// @name Types
  /// @{
  public:
    typedef SymbolTable TypeList;
    typedef TypeList::iterator type_iterator;
    typedef TypeList::const_iterator type_const_iterator;

    typedef SymbolTable FuncList;
    typedef FuncList::iterator func_iterator;
    typedef FuncList::const_iterator func_const_iterator;

    typedef SymbolTable VarList;
    typedef VarList::iterator var_iterator;
    typedef VarList::const_iterator var_const_iterator;

  /// @}
  /// @name Constructors
  /// @{
  public:
    static Bundle* create(const Locator* location, const std::string& pubid);

  protected:
    Bundle() : Documentable(BundleID), name(), types(), vars(), funcs() {}
    virtual ~Bundle();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string& getName() const { return name; }
    static inline bool classof(const Bundle*) { return true; }
    static inline bool classof(const Node* N) { return N->is(BundleID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setName(const std::string& n) { name = n; }
    virtual void insertChild(Node* kid);
    virtual void removeChild(Node* kid);

  /// @}
  /// @name Finders
  /// @{
  public:
    Type*     type_find(const std::string& n) const;
    Function* func_find(const std::string& n) const;
    Variable*  var_find(const std::string& n) const;

  /// @}
  /// @name Iterators
  /// @{
  public:
    type_iterator           type_begin()       { return types.begin(); }
    type_const_iterator     type_begin() const { return types.begin(); }
    type_iterator           type_end  ()       { return types.end(); }
    type_const_iterator     type_end  () const { return types.end(); }
    size_t                  type_size () const { return types.size(); }
    bool                    type_empty() const { return types.empty(); }

    func_iterator           func_begin()       { return funcs.begin(); }
    func_const_iterator     func_begin() const { return funcs.begin(); }
    func_iterator           func_end  ()       { return funcs.end(); }
    func_const_iterator     func_end  () const { return funcs.end(); }
    size_t                  func_size () const { return funcs.size(); }
    bool                    func_empty() const { return funcs.empty(); }

    var_iterator            var_begin()       { return vars.begin(); }
    var_const_iterator      var_begin() const { return vars.begin(); }
    var_iterator            var_end  ()       { return vars.end(); }
    var_const_iterator      var_end  () const { return vars.end(); }
    size_t                  var_size () const { return vars.size(); }
    bool                    var_empty() const { return vars.empty(); }
  /// @}
  /// @name Data
  /// @{
  protected:
    std::string name;   ///< The name for this bundle
    TypeList    types;  ///< The list of types
    VarList     vars;   ///< The list of variables
    FuncList    funcs;  ///< The list of functions
  /// @}
  friend class AST;
};

} // end hlvm namespace
#endif
