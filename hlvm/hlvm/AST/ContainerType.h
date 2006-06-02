//===-- AST ContainerType Class ---------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/ContainerType.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::ContainerType
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_CONTAINERTYPE_H
#define HLVM_AST_CONTAINERTYPE_H

#include <hlvm/AST/Type.h>

namespace hlvm 
{

/// This class represents a Type in the HLVM Abstract Syntax Tree.  
/// A Type defines the format of storage. 
/// @brief HLVM AST Type Node
class ContainerType : public Type
{
  /// @name Types
  /// @{
  public:
    typedef std::vector<AliasType*> ContentsList;
    typedef ContentsList::iterator iterator;
    typedef ContentsList::const_iterator const_iterator;

  /// @}
  /// @name Constructors
  /// @{
  protected:
    ContainerType(NodeIDs id) : Type(id), contents() {}
    virtual ~ContainerType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const;
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const ContainerType*) { return true; }
    static inline bool classof(const Type* T) { return T->isContainerType(); }

  /// @}
  /// @name Mutators
  /// @{
  protected:
    virtual void insertChild(Node* n);
    virtual void removeChild(Node* n);

  /// @}
  /// @name Iterators
  /// @{
  public:
    iterator         begin()       { return contents.begin(); }
    const_iterator   begin() const { return contents.begin(); }
    iterator         end  ()       { return contents.end(); }
    const_iterator   end  () const { return contents.end(); }
    size_t           size () const { return contents.size(); }
    bool             empty() const { return contents.empty(); }
    AliasType*       front()       { return contents.front(); }
    const AliasType* front() const { return contents.front(); }
    AliasType*       back()        { return contents.back(); }
    const AliasType* back()  const { return contents.back(); }

  /// @}
  /// @name Data
  /// @{
  protected:
    ContentsList contents; ///< The contained types
  /// @}
};

typedef AliasType Field;

/// This class represents an HLVM type that is a sequence of data fields 
/// of varying type. 
class StructureType : public ContainerType
{
  /// @name Constructors
  /// @{
  public:
    StructureType() : ContainerType(StructureTypeID) {}
    virtual ~StructureType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const StructureType*) { return true; }
    static inline bool classof(const Type* T) { return T->isStructureType(); }
    static inline bool classof(const Node* T) 
      { return T->is(StructureTypeID); }

  /// @}
  /// @name Mutators
  /// @{
  protected:
    void addField(Field* field) { contents.push_back(field); }

  /// @}
  /// @name Data
  /// @{
  protected:
  /// @}
};

typedef AliasType Argument;

/// This class represents an HLVM type that is a sequence of data fields 
/// of varying type. 
class SignatureType : public ContainerType
{
  /// @name Constructors
  /// @{
  public:
    SignatureType() 
      : ContainerType(SignatureTypeID), result(0), varargs(false) {}
    virtual ~SignatureType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const Type* getResultType() const { return result; }
    bool  isVarArgs() const { return varargs; }

    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const SignatureType*) { return true; }
    static inline bool classof(const Type* T) { return T->isSignatureType(); }
    static inline bool classof(const Node* T) 
      { return T->is(SignatureTypeID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setResultType(const Type* ty) { result = ty; }
    void setIsVarArgs(bool is) { varargs = is; }
    void addArgument(Argument* arg) { contents.push_back(arg); }

  /// @}
  /// @name Data
  /// @{
  protected:
    const Type* result;  ///< The result type of the function signature
    bool varargs;        ///< Indicates variable arguments function
  /// @}
  friend class AST;
};

} // hlvm
#endif
