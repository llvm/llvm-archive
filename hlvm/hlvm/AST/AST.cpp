//===-- hlvm/AST/AST.cpp - AST Container Class ------------------*- C++ -*-===//
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
/// @file hlvm/AST/AST.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::AST::AST.
//===----------------------------------------------------------------------===//

#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Function.h>
#include <hlvm/AST/Import.h>
#include <hlvm/AST/Variable.h>
#include <hlvm/AST/SymbolTable.h>

using namespace hlvm::AST;

namespace {
class ASTImpl : public AST
{
  public:
    ASTImpl() {}
    ~ASTImpl();

  private:
    // Pool pool;
    SymbolTable types;
    SymbolTable vars;
    SymbolTable funcs;
    SymbolTable unresolvedTypes;

  public:
    Type* resolveType(const std::string& name);
    void addType(Type*);
};

ASTImpl::~ASTImpl()
{
}

Type*
ASTImpl::resolveType(const std::string& name)
{
  Node* n = types.lookup(name);
  if (n)
    return llvm::cast<Type>(n);
  n = unresolvedTypes.lookup(name);
  if (n)
    return llvm::cast<OpaqueType>(n);
  OpaqueType* ot = this->new_OpaqueType(name);
  unresolvedTypes.insert(ot->getName(), ot);
  return ot;
}

void
ASTImpl::addType(Type* ty)
{
  Node* n = unresolvedTypes.lookup(ty->getName());
  if (n) {
    OpaqueType* ot = llvm::cast<OpaqueType>(n);
    // FIXME: Replace all uses of "ot" with "ty"
    unresolvedTypes.erase(ot);
  }
  types.insert(ty->getName(),ty);
}

}

namespace hlvm {
namespace AST {

AST* 
AST::create()
{
  return new ASTImpl();
}

void
AST::destroy(AST* ast)
{
  delete static_cast<ASTImpl*>(ast);
}

AST::~AST()
{
}

Type* 
AST::resolveType(const std::string& name)
{
  return static_cast<const ASTImpl*>(this)->resolveType(name);
}

Bundle*
AST::new_Bundle(const Locator& loc, const std::string& id)
{
  Bundle* result = new Bundle();
  result->setLocator(loc);
  result->setName(id);
  return result;
}

Function*
AST::new_Function(const Locator& loc, const std::string& id)
{
  Function* result = new Function();
  result->setLocator(loc);
  result->setName(id);
  return result;
}

Import*
AST::new_Import(const Locator& loc, const std::string& pfx)
{
  Import* result = new Import();
  result->setLocator(loc);
  result->setPrefix(pfx);
  return result;
}

IntegerType* 
AST::new_IntegerType(
  const Locator&loc, 
  const std::string& id, 
  uint64_t bits, 
  bool isSigned )
{
  IntegerType* result = new IntegerType();
  result->setBits(bits);
  result->setSigned(isSigned);
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

RangeType* 
AST::new_RangeType(const Locator&loc, const std::string& id, int64_t min, int64_t max)
{
  RangeType* result = new RangeType();
  result->setLocator(loc);
  result->setName(id);
  result->setMin(min);
  result->setMax(max);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

EnumerationType* 
AST::new_EnumerationType(
  const Locator&loc, 
  const std::string& id )
{
  EnumerationType* result = new EnumerationType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

RealType* 
AST::new_RealType(
  const Locator&loc,
  const std::string& id,  
  uint32_t mantissa, 
  uint32_t exponent)
{
  RealType* result = new RealType();
  result->setMantissa(mantissa);
  result->setExponent(exponent);
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

AnyType* 
AST::new_AnyType(const Locator&loc, const std::string& id)
{
  AnyType* result = new AnyType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

BooleanType* 
AST::new_BooleanType(const Locator&loc, const std::string& id)
{
  BooleanType* result = new BooleanType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

CharacterType* 
AST::new_CharacterType(const Locator&loc, const std::string& id)
{
  CharacterType* result = new CharacterType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

OctetType* 
AST::new_OctetType(const Locator&loc, const std::string& id)
{
  OctetType* result = new OctetType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

VoidType* 
AST::new_VoidType(const Locator&loc, const std::string& id)
{
  VoidType* result = new VoidType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

PointerType* 
AST::new_PointerType(
  const Locator& loc, 
  const std::string& id,
  Type* target
)
{
  PointerType* result = new PointerType();
  result->setLocator(loc);
  result->setName(id);
  result->setTargetType(target);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

ArrayType* 
AST::new_ArrayType(
  const Locator& loc, 
  const std::string& id,
  Type* elemType,
  uint64_t maxSize
)
{
  ArrayType* result = new ArrayType();
  result->setLocator(loc);
  result->setName(id);
  result->setElementType(elemType);
  result->setMaxSize(maxSize);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

VectorType* 
AST::new_VectorType(
  const Locator& loc, 
  const std::string& id,
  Type* elemType,
  uint64_t size
)
{
  VectorType* result = new VectorType();
  result->setLocator(loc);
  result->setName(id);
  result->setElementType(elemType);
  result->setSize(size);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

AliasType* 
AST::new_AliasType(const Locator& loc, const std::string& id, Type* referrant)
{
  AliasType* result = new AliasType();
  result->setLocator(loc);
  result->setName(id);
  result->setType(referrant);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

StructureType*
AST::new_StructureType(const Locator& loc, const std::string& id)
{
  StructureType* result = new StructureType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

SignatureType*
AST::new_SignatureType(const Locator& loc, const std::string& id, Type* ty)
{
  SignatureType* result = new SignatureType();
  result->setLocator(loc);
  result->setName(id);
  result->setResultType(ty);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

Variable*
AST::new_Variable(const Locator& loc, const std::string& id)
{
  Variable* result = new Variable();
  result->setLocator(loc);
  result->setName(id);
  return result;
}

OpaqueType*
AST::new_OpaqueType(const std::string& id)
{
  return new OpaqueType(id);
}

}}
