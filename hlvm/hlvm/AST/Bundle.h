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
#include <hlvm/AST/Constants.h>
#include <hlvm/AST/SymbolTable.h>

namespace hlvm 
{ 

class Type;
class Linkable;

/// This type enumerates the intrinsic types. Intrinsic types are those which
/// are intrinsic to HLVM. They are well known, have immutable names, and are
/// generally fundamental in character.
/// @brief AST Intrinsic Types Enum
enum IntrinsicTypes {
  NoIntrinsicType, ///< This is used for errors, etc.
  boolTy,     ///< The boolean type
  FirstIntrinsicType = boolTy,
  bufferTy,   ///< The memory buffer type
  charTy,     ///< The UTF-8 character type
  doubleTy,   ///< 64-bit IEEE 754 double precision
  f32Ty,      ///< 32-bit IEEE 754 single precision 
  f44Ty,      ///< 43-bit IEEE 754 extended single precision 
  f64Ty,      ///< 64-bit IEEE 754 double precision
  f80Ty,      ///< 80-bit IEEE 754 extended double precision
  f96Ty,      ///< 96-bit IEEE 754 long extended double precision
  f128Ty,     ///< 128-bit IEEE 754 quad precision
  floatTy,    ///< 32-bit IEEE 754 single precision
  intTy,      ///< Signed 32-bit integer quantity
  longTy,     ///< Signed 64-bit integer quantity
  octetTy,    ///< Unsigned 8-bit integer quantity, not computable
  qs16Ty,     ///< Signed 16-bit rational quantity
  qs32Ty,     ///< Signed 32-bit rational quantity
  qs64Ty,     ///< Signed 64-bit rational quantity
  qs128Ty,    ///< Signed 8-bit rational quantity
  qu16Ty,     ///< Unsigned 16-bit rational quantity
  qu32Ty,     ///< Unsigned 32-bit rational quantity
  qu64Ty,     ///< Unsigned 64-bit rational quantity
  qu128Ty,    ///< Unsigned 8-bit rational quantity
  r8Ty,       ///< Range checked signed 8-bit integer quantity
  r16Ty,      ///< Range checked signed 16-bit integer quantity
  r32Ty,      ///< Range checked signed 32-bit integer quantity
  r64Ty,      ///< Range checked signed 64-bit integer quantity
  s8Ty,       ///< Signed 8-bit integer quantity
  s16Ty,      ///< Signed 16-bit integer quantity
  s32Ty,      ///< Signed 32-bit integer quantity
  s64Ty,      ///< Signed 64-bit integer quantity
  s128Ty,     ///< Signed 128-bit integer quantity
  shortTy,    ///< Signed 16-bit integer quantity
  streamTy,   ///< The I/O stream type
  stringTy,   ///< The UTF-8 string type
  textTy,     ///< The UTF-8 text type
  u8Ty,       ///< Unsigned 8-bit integer quantity
  u16Ty,      ///< Unsigned 16-bit integer quantity
  u32Ty,      ///< Unsigned 32-bit integer quantity
  u64Ty,      ///< Unsigned 64-bit integer quantity
  u128Ty,     ///< Unsigned 128-bit integer quantity
  voidTy,     ///< OpaqueType, 0-bits, non-readable, non-writable
  LastIntrinsicType = u128Ty
};

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
    typedef std::vector<Type*> TypeList;
    typedef TypeList::iterator tlist_iterator;
    typedef TypeList::const_iterator tlist_const_iterator;

    typedef SymbolTable<Type> TypeTable;
    typedef TypeTable::iterator ttable_iterator;
    typedef TypeTable::const_iterator ttable_const_iterator;

    typedef std::vector<Constant*> ConstantList;
    typedef ConstantList::iterator clist_iterator;
    typedef ConstantList::const_iterator clist_const_iterator;

    typedef SymbolTable<Constant> ConstantTable;
    typedef ConstantTable::iterator ctable_iterator;
    typedef ConstantTable::const_iterator ctable_const_iterator;

  /// @}
  /// @name Constructors
  /// @{
  protected:
    Bundle() 
      : Documentable(BundleID), name(), tlist(), ttable(), clist(), ctable() {}
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
  /// @name Type Management
  /// @{
  public:
    /// Return the type of a program, named ProgramType
    SignatureType* getProgramType();

    /// Get the Intrinsic Id from the intrinsic's name
    IntrinsicTypes getIntrinsicTypesValue(const std::string& name);

    /// Get the standard name of one of the primitive types
    void getIntrinsicName(IntrinsicTypes ty, std::string& name);

    /// Get one of the intrinsic types directly by its identifier
    Type* getIntrinsicType(IntrinsicTypes ty);

    /// Get one of the intrinsic types by its name
    Type* getIntrinsicType(const std::string& name);

    /// Get a standard pointer type to the element type Ty.
    PointerType* getPointerTo(const Type* Ty);

    /// Resolve a type name into a Type and allow for forward referencing.
    Type* getOrCreateType(const std::string& name);

    /// Get an existing type by name or 0 if there is no such type
    Type*      getType(const std::string& n);

    /// Get an existing constant by name or 0 if there is no such constant
    Constant*  getConst(const std::string& n) const;

  /// @}
  /// @name Iterators
  /// @{
  public:
    // Type Insertion Order Iteration
    tlist_iterator          tlist_begin()       { return tlist.begin(); }
    tlist_const_iterator    tlist_begin() const { return tlist.begin(); }
    tlist_iterator          tlist_end  ()       { return tlist.end(); }
    tlist_const_iterator    tlist_end  () const { return tlist.end(); }
    size_t                  tlist_size () const { return tlist.size(); }
    bool                    tlist_empty() const { return tlist.empty(); }

    /// Type Symbol Table Iteration
    ttable_iterator         ttable_begin()       { return ttable.begin(); }
    ttable_const_iterator   ttable_begin() const { return ttable.begin(); }
    ttable_iterator         ttable_end  ()       { return ttable.end(); }
    ttable_const_iterator   ttable_end  () const { return ttable.end(); }
    size_t                  ttable_size () const { return ttable.size(); }
    bool                    ttable_empty() const { return ttable.empty(); }

    /// Value Insertion Order Iteration
    clist_iterator          clist_begin()       { return clist.begin(); }
    clist_const_iterator    clist_begin() const { return clist.begin(); }
    clist_iterator          clist_end  ()       { return clist.end(); }
    clist_const_iterator    clist_end  () const { return clist.end(); }
    size_t                  clist_size () const { return clist.size(); }
    bool                    clist_empty() const { return clist.empty(); }

    /// Value Symbol Table Iteration
    ctable_iterator         ctable_begin()       { return ctable.begin(); }
    ctable_const_iterator   ctable_begin() const { return ctable.begin(); }
    ctable_iterator         ctable_end  ()       { return ctable.end(); }
    ctable_const_iterator   ctable_end  () const { return ctable.end(); }
    size_t                  ctable_size () const { return ctable.size(); }
    bool                    ctable_empty() const { return ctable.empty(); }

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string   name;      ///< The name for this bundle
    TypeList      tlist;     ///< The list of types
    TypeTable     ttable;    ///< The list of types
    TypeTable     unresolvedTypes; ///< The list of forward referenced types
    ConstantList  clist;    ///< The list of values in insertion order
    ConstantTable ctable;

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an Import 
/// of one Bundle into another. An Import encapsulates two data items: the URI
/// of the Bundle that is to be imported, and a prefix by which items in that
/// Bundle can be referenced. For example, if Bundle "Fooness" contains a 
/// definition named "foo" then another bundle specifying an import of "Fooness"
/// with prefix "F" can refer to "foo" in "Fooness" with "F:foo".
/// @see Bundle
/// @brief AST Import Node
class Import : public Documentable
{
  /// @name Constructors
  /// @{
  protected:
    Import() : Documentable(ImportID) {}
    virtual ~Import();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const Import*) { return true; }
    static inline bool classof(const Node* N) { return N->is(ImportID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setPrefix(const std::string& pfx) { prefix = pfx; }

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string prefix;
  /// @}
  friend class AST;
};

} // end hlvm namespace
#endif
