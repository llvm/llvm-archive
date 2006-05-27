//===-- AST Type Class ------------------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Type.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::AST::Type.
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Type.h>
#include <hlvm/Base/Assert.h>

namespace hlvm {

Type::~Type()
{
}

void
Type::insertChild(Node* n)
{
  hlvmAssert(!"This type doesn't accept children!");
}

const char*
Type::getPrimitiveName() const
{
  return 0;
}

AnyType::~AnyType()
{
}

const char* 
AnyType::getPrimitiveName() const
{
  return "any";
}

BooleanType::~BooleanType()
{
}

const char* 
BooleanType::getPrimitiveName() const
{
  return "bool";
}

CharacterType::~CharacterType()
{
}

const char* 
CharacterType::getPrimitiveName() const
{
  return "char";
}

IntegerType::~IntegerType()
{
}

IntegerType* 
IntegerType::clone(const std::string& newname)
{
  IntegerType* result = new IntegerType(*this);
  result->setName(newname);
  return result;
}

const char* 
IntegerType::getPrimitiveName() const
{
  if (numBits > 128)
    return 0;

  if (signedness) {
    if (numBits > 64)
      return "s128";
    else if (numBits > 32)
      return "s64";
    else if (numBits > 16)
      return "s32";
    else if (numBits > 8)
      return "s16";
    else
      return "s8";
  } else {
    if (numBits > 64)
      return "u128";
    else if (numBits > 32)
      return "u64";
    else if (numBits > 16)
      return "u32";
    else if (numBits > 8)
      return "u16";
    else
      return "u8";
  }
  hlvmDeadCode("Primitive Name");
}

OctetType::~OctetType()
{
}

const char* 
OctetType::getPrimitiveName() const
{
  return "octet";
}

RangeType::~RangeType()
{
}

const char* 
RangeType::getPrimitiveName() const
{
  if (min < 0) {
    if (min >= 0 && max <= 255U)
      return "u8";
    else if (min >= 0 && max <= 65535U)
      return "u16";
    else if (min >= 0 && max <= 4294967295U)
      return "u32";
    else if (min >= 0 && max <= 9223372036854775807LL)
      return "u64";
    else
      // FIXME: handle u128 case
      return 0;
  } else {
    if (min >= -127 && max <= 127)
      return "s8";
    else if (min >= -32767 && max <= 32767)
      return "s16";
    else if (min >= -2147483647 && max <= 2147483647)
      return "s32";
    else if (min >= -9223372036854775807LL && max <= 9223372036854775807LL)
      return "s64";
    else
      // FIXME: handle s128 case
      return 0;
  }
  return 0;
}

EnumerationType::~EnumerationType()
{
}

const char* 
EnumerationType::getPrimitiveName() const
{
  if (size() < 4294967295U)
    return "u32";
  return 0;
}

RealType::~RealType()
{
}

const char* 
RealType::getPrimitiveName() const
{
  switch (mantissa) {
    case 23:
      if (exponent  == 8)
        return "f32";
      break;
    case 32:
      if (exponent == 11)
        return "f43";
      break;
    case 52:
      if (exponent == 11) 
        return "f64";
      break;
    case 64:
      if (exponent == 15) 
        return "f80";
      break;
    case 112:
      if (exponent == 15)
        return "f128";
      break;
    default:
      break;
  }
  return 0;
}

VoidType::~VoidType()
{
}

const char* 
VoidType::getPrimitiveName() const
{
  return "void";
}

PointerType::~PointerType()
{
}

ArrayType::~ArrayType()
{
}

VectorType::~VectorType()
{
}

AliasType::~AliasType()
{
}

const char*
AliasType::getPrimitiveName() const
{
  return type->getPrimitiveName();
}

StringType::~StringType()
{
}

OpaqueType::~OpaqueType()
{
}

}
