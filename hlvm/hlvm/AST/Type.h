//
// Copyright (C) 2006 HLVM Group. All Rights Reserved.
//
// This program is open source software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License (GPL) as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. You should have received a copy of the GPL in a
// file named COPYING that was included with this program; if not, you can
// obtain a copy of the license through the Internet at http://www.fsf.org/
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
////////////////////////////////////////////////////////////////////////////////
/// @file hlvm/AST/Type.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Type
////////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_AST_TYPE_H
#define HLVM_AST_TYPE_H

#include <hlvm/AST/Node.h>

namespace hlvm {
namespace AST {

  /// This class represents a Type in the HLVM Abstract Syntax Tree.  
  /// A Type defines the format of storage. 
  /// @brief HLVM AST Type Node
  class Type : public Node
  {
    /// @name Constructors
    /// @{
    public:
      Type(
        NodeIDs id, ///< The Type identifier
        Node* parent = 0, ///< The bundle in which the function is defined
        const std::string& name = "" ///< The name of the function
      ) : Node(id, parent, name)  {}
      virtual ~Type();

    /// @}
    /// @name Accessors
    /// @{
      inline bool isPrimitiveType() const { return id_ <= LastPrimitiveTypeID; }
      inline bool isIntegralType()  const { 
        return id_ == IntegerTypeID || id_ == RangeTypeID; 
      }
      inline bool isContainerType() const { 
        return id_ >= FirstContainerTypeID; 
      }
      inline bool isIntegerType() const { return id_ == IntegerTypeID; }
      inline bool isRangeType() const { return id_ == RangeTypeID; }
      inline bool isRealType() const { return id_ == RealTypeID; }
      inline bool isRationalType() const { return id_ == RationalTypeID; }
      inline bool isPointerType() const { return id_ == PointerTypeID; }
      inline bool isArrayType() const { return id_ == ArrayTypeID; }
      inline bool isVectorType() const { return id_ == VectorTypeID; }
      inline bool isStructureType() const { return id_ == StructureTypeID; }
      inline bool isSignatureType() const { return id_ == SignatureTypeID; }

      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const Node*) { return true; }
      static inline bool classof(const Type*) { return true; }

    /// @}
    /// @name Data
    /// @{
    protected:
    /// @}
  };

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
      IntegerType(
        Node* parent = 0, ///< The bundle in which the function is defined
        const std::string& name = "" ///< The name of the function
      ) : Type(IntegerTypeID,parent,name) {}
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
  };

  /// A RangeType is an IntegerType that allows the range of values to be
  /// constricted. The use of RangeType implies range checking whenever the
  /// value of a RangeType variable is assigned.
  class RangeType: public Type
  {
    /// @name Constructors
    /// @{
    public:
      RangeType(
        Node* parent, ///< The bundle in which the function is defined
        const std::string& name ///< The name of the function
      ) : Type(RangeTypeID,parent,name) {}
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
      RealType(
        Node* parent, ///< The bundle in which the function is defined
        const std::string& name ///< The name of the function
      ) : Type(RealTypeID,parent,name) {}
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
  };
} // AST
} // hlvm
#endif
