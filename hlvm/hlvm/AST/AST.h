//===-- AST Container Class -------------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/AST.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::AST
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_AST_H
#define HLVM_AST_AST_H

#include <hlvm/AST/Node.h>
#include <hlvm/AST/Type.h>
#include <string>
#include <vector>

/// This namespace is for all HLVM software. It ensures that HLVM software does
/// not collide with any other software. Hopefully HLVM is not a namespace used
/// elsewhere. 
namespace hlvm
{

class Bundle;   
class Documentation;
class Block;
class Function; 
class Program; 
class Import;
class Locator; 
class SignatureType;
class StructureType;
class Variable; 
class ConstLiteralInteger;
class ReturnOp;
class AliasType;
typedef AliasType Argument;
typedef AliasType Field;

/// This class is used to hold or contain an Abstract Syntax Tree. It provides
/// those aspects of the tree that are not part of the tree itself.
/// @brief AST Container Class
class AST : public Node
{
  /// @name Types
  /// @{
  public:
    typedef std::vector<Bundle*> BundleList;
    typedef BundleList::iterator   iterator;
    typedef BundleList::const_iterator const_iterator;

  /// @}
  /// @name Constructors
  /// @{
  public:
    static AST* create();
    static void destroy(AST* ast);

  protected:
    AST() : Node(TreeTopID), sysid(), pubid(), bundles() {}
    ~AST();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string& getSystemID() const { return sysid; }
    const std::string& getPublicID() const { return pubid; }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setSystemID(const std::string& id) { sysid = id; }
    void setPublicID(const std::string& id) { pubid = id; }
    void addBundle(Bundle* b) { bundles.push_back(b); }

  /// @}
  /// @name Lookup
  /// @{
  public:
    Type* resolveType(const std::string& name);

  /// @}
  /// @name Iterators
  /// @{
  public:
    iterator           begin()       { return bundles.begin(); }
    const_iterator     begin() const { return bundles.begin(); }
    iterator           end  ()       { return bundles.end(); }
    const_iterator     end  () const { return bundles.end(); }
    size_t             size () const { return bundles.size(); }
    bool               empty() const { return bundles.empty(); }
    Bundle*            front()       { return bundles.front(); }
    const Bundle*      front() const { return bundles.front(); }
    Bundle*            back()        { return bundles.back(); }
    const Bundle*      back()  const { return bundles.back(); }

  /// @}
  /// @name Factories
  /// @{
  public:
    Bundle* new_Bundle(const Locator& loc, const std::string& id);
    Function* new_Function(const Locator& loc, const std::string& id);
    ReturnOp* new_ReturnOp(const Locator& loc);
    Block* new_Block(const Locator& loc);
    Program* new_Program(const Locator& loc, const std::string& id);
    Import* new_Import(const Locator& loc, const std::string& id);
    Variable* new_Variable(const Locator& loc, const std::string& id);
    IntegerType* new_IntegerType(
      const Locator&loc,      ///< The locator of the declaration
      const std::string& id,  ///< The name of the atom
      uint64_t bits = 32,     ///< The number of bits
      bool isSigned = true    ///< The signedness
    );
    RangeType* new_RangeType(
      const Locator&loc,      ///< The locator of the declaration
      const std::string& id,  ///< The name of the atom
      int64_t min,            ///< The minimum value accepted in range
      int64_t max             ///< The maximum value accepted in range
    );
    EnumerationType* new_EnumerationType(
      const Locator&loc,      ///< The locator of the declaration
      const std::string& id   ///< The name of the atom
    );
    RealType* new_RealType(
      const Locator&loc,      ///< The locator of the declaration
      const std::string& id,  ///< The name of the atom
      uint32_t mantissa = 52, ///< The bits in the mantissa (fraction)
      uint32_t exponent = 11  ///< The bits in the exponent
    );
    AnyType* new_AnyType(const Locator&loc, const std::string& id);
    BooleanType* 
      new_BooleanType(const Locator&loc, const std::string& id);
    CharacterType* 
      new_CharacterType(const Locator&loc, const std::string& id);
    OctetType* 
      new_OctetType(const Locator&loc, const std::string& id);
    VoidType* new_VoidType(const Locator&loc, const std::string& id);
    PointerType* new_PointerType(
      const Locator& loc, 
      const std::string& id,
      Type* target
    );
    ArrayType* new_ArrayType(
      const Locator& loc, 
      const std::string& id,
      Type* elemType,
      uint64_t maxSize
    );
    VectorType* new_VectorType(
      const Locator& loc, 
      const std::string& id,
      Type* elemType,
      uint64_t size
    );
    AliasType* new_AliasType(
      const Locator& loc,
      const std::string& id,
      Type* referrant
    );
    StructureType* 
      new_StructureType(const Locator& l, const std::string& id);
    SignatureType* new_SignatureType(
      const Locator& loc, 
      const std::string& id,
      Type *resultType
    );
    OpaqueType* new_OpaqueType(const std::string& id);
    RealType* new_f128(const Locator& l, const std::string& id)
      { return new_RealType(l,id,112,15); }
    RealType* new_f80(const Locator& l, const std::string& id)
      { return new_RealType(l,id,64,15); }
    RealType* new_f64(const Locator& l, const std::string& id)
      { return new_RealType(l,id,52,11); }
    RealType* new_f43(const Locator& l, const std::string& id)
      { return new_RealType(l,id,32,11); }
    RealType* new_f32(const Locator& l, const std::string& id)
      { return new_RealType(l,id,23,8); }
    IntegerType* new_s128(const Locator& l, const std::string& id)
      { return new_IntegerType(l,id,128,true); }
    IntegerType* new_s64(const Locator& l, const std::string& id)
      { return new_IntegerType(l,id,64,true); }
    IntegerType* new_s32(const Locator& l, const std::string& id)
      { return new_IntegerType(l,id,32,true); }
    IntegerType* new_s16(const Locator& l, const std::string& id)
      { return new_IntegerType(l,id,16,true); }
    IntegerType* new_s8(const Locator& l, const std::string& id)
      { return new_IntegerType(l,id,8,true); }
    IntegerType* new_u128(const Locator& l, const std::string& id)
      { return new_IntegerType(l,id,128,false); }
    IntegerType* new_u64(const Locator& l, const std::string& id)
      { return new_IntegerType(l,id,64,false); }
    IntegerType* new_u32(const Locator& l, const std::string& id)
      { return new_IntegerType(l,id,32,false); }
    IntegerType* new_u16(const Locator& l, const std::string& id)
      { return new_IntegerType(l,id,16,false); }
    IntegerType* new_u8(const Locator& l, const std::string& id)
      { return new_IntegerType(l,id,8,false); }

    Argument* 
      new_Argument(const Locator& loc, const std::string& id, Type* ty );

    StringType* new_StringType(const Locator& loc, const std::string& id);

    ConstLiteralInteger* new_ConstLiteralInteger(const Locator& loc);
    Documentation* new_Documentation(const Locator& loc);
    Type* getPrimitiveType(NodeIDs kind);

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string sysid;
    std::string pubid;
    BundleList bundles;
  /// @}
};

} // hlvm
#endif
