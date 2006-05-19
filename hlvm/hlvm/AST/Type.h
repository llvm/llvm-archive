//===-- hlvm/AST/Type.h - AST Type Class ------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Type.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Type
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_TYPE_H
#define HLVM_AST_TYPE_H

#include <hlvm/AST/Node.h>

namespace hlvm {
namespace AST {

  /// This class represents a Type in the HLVM Abstract Syntax Tree.  
  /// A Type defines the format of storage. 
  /// @brief HLVM AST Type Node
  class Type : public ParentNode
  {
    /// @name Constructors
    /// @{
    public:
      Type(
        NodeIDs id ///< The Type identifier
      ) : ParentNode(id )  {}
      virtual ~Type();

    /// @}
    /// @name Accessors
    /// @{
      inline bool isPrimitiveType() const { return id <= LastPrimitiveTypeID; }
      inline bool isIntegralType()  const { 
        return id == IntegerTypeID || id == RangeTypeID; 
      }
      inline bool isContainerType() const { 
        return id >= FirstContainerTypeID; 
      }
      inline bool isIntegerType() const { return id == IntegerTypeID; }
      inline bool isRangeType() const { return id == RangeTypeID; }
      inline bool isRealType() const { return id == RealTypeID; }
      inline bool isRationalType() const { return id == RationalTypeID; }
      inline bool isPointerType() const { return id == PointerTypeID; }
      inline bool isArrayType() const { return id == ArrayTypeID; }
      inline bool isVectorType() const { return id == VectorTypeID; }
      inline bool isStructureType() const { return id == StructureTypeID; }
      inline bool isSignatureType() const { return id == SignatureTypeID; }

      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const Node*) { return true; }
      static inline bool classof(const Type*) { return true; }

    /// @}
    /// @name Data
    /// @{
    protected:
    /// @}
    friend class AST;
  };

  /// A NamedType is simply a pair involving a name and a pointer to a Type.
  /// This is so frequently needed, it is declared here for convenience.
  typedef std::pair<std::string,Type*> NamedType;

  /// This class represents all HLVM integer types. An integer type declares the
  /// the minimum number of bits that are required to store the integer type.
  /// HLVM will convert this specification to the most appropriate sized 
  /// machine type for computation. If the number of bits is specified as zero
  /// it implies infinite precision integer arithmetic.
  class IntegerType : public Type
  {
    /// @name Constructors
    /// @{
    public:
      IntegerType() : Type(IntegerTypeID), numBits(32) {}
      virtual ~IntegerType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const IntegerType*) { return true; }
      static inline bool classof(const Type* T) { return T->isIntegerType(); }

    /// @}
    /// @name Data
    /// @{
    protected:
      uint32_t numBits; ///< Minimum number of bits
    /// @}
    friend class AST;
  };

  /// A RangeType is an IntegerType that allows the range of values to be
  /// constricted. The use of RangeType implies range checking whenever the
  /// value of a RangeType variable is assigned.
  class RangeType: public Type
  {
    /// @name Constructors
    /// @{
    public:
      RangeType() : Type(RangeTypeID) {}
      virtual ~RangeType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const RangeType*) { return true; }
      static inline bool classof(const Type* T) { return T->isRangeType(); }
    /// @}
    /// @name Data
    /// @{
    protected:
      uint64_t min_value_; ///< Lowest value accepted
      uint64_t max_value_; ///< Highest value accepted
    /// @}
    friend class AST;
  };

  /// This class represents all HLVM real number types. The precision and 
  /// mantissa are specified as a number of decimal digits to be provided as a
  /// minimum.  HLVM will use the machine's natural floating point 
  /// representation for those real types that can fit within the requested
  /// precision and mantissa lengths. If not, infinite precision floating point
  /// arithmetic will be utilized.
  class RealType : public Type
  {
    /// @name Constructors
    /// @{
    public:
      RealType() : Type(RealTypeID) {}
      virtual ~RealType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const RealType*) { return true; }
      static inline bool classof(const Type* T) { return T->isRealType(); }
    /// @}
    /// @name Data
    /// @{
    protected:
      uint32_t precision_; ///< Number of decimal digits of precision
      uint32_t mantissa_;  ///< Number of decimal digits in mantissa
    /// @}
    friend class AST;
  };
} // AST
} // hlvm
#endif
