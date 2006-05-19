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

#include <hlvm/AST/LinkageItem.h>

namespace hlvm {
namespace AST {

  class IntrinsicType;

  /// This class represents a Type in the HLVM Abstract Syntax Tree.  
  /// A Type defines the format of storage. 
  /// @brief HLVM AST Type Node
  class Type : public LinkageItem
  {
    /// @name Constructors
    /// @{
    protected:
      Type(
        NodeIDs id ///< The Type identifier
      ) : LinkageItem(id)  {}
    public:
      virtual ~Type();

    /// @}
    /// @name Accessors
    /// @{
    public:
      inline bool isPrimitiveType() const { return id <= LastPrimitiveTypeID; }
      inline bool isIntegralType()  const { 
        return id == IntegerTypeID || id == RangeTypeID; 
      }
      inline bool isContainerType() const { 
        return id >= FirstContainerTypeID; 
      }
      inline bool isAnyType() const { return id == AnyTypeID; }
      inline bool isBooleanType() const { return id == BooleanTypeID; }
      inline bool isCharacterType() const { return id == CharacterTypeID; }
      inline bool isOctetType() const { return id == OctetTypeID; }
      inline bool isIntegerType() const { return id == IntegerTypeID; }
      inline bool isRangeType() const { return id == RangeTypeID; }
      inline bool isRealType() const { return id == RealTypeID; }
      inline bool isRationalType() const { return id == RationalTypeID; }
      inline bool isPointerType() const { return id == PointerTypeID; }
      inline bool isArrayType() const { return id == ArrayTypeID; }
      inline bool isVectorType() const { return id == VectorTypeID; }
      inline bool isStructureType() const { return id == StructureTypeID; }
      inline bool isSignatureType() const { return id == SignatureTypeID; }
      inline bool isVoidType() const { return id == VoidTypeID; }

      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const Type*) { return true; }
      static inline bool classof(const Node* n) { return n->isType(); }

    /// @}
    /// @name Mutators
    /// @{
    public:
      // We override receiveChild generically here to produce an error. Most
      // Type subclasses can't receive children. Those that do, can override
      // again.
      virtual void insertChild(Node* n);

    /// @}
    /// @name Data
    /// @{
    protected:
    /// @}
    friend class AST;
  };

  /// This class is type that combines a name with an arbitrary type. This
  /// construct is used any where a named and typed object is needed such as
  /// the parameter to a function or the field of a structure. 
  class NamedType : public Type
  {
    /// @name Constructors
    /// @{
    public:
      NamedType() : Type(NamedTypeID) {}
      virtual ~NamedType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Get the name for the type
      const std::string&  getName() const { return name; }
      
      // Get the type for the name
      Type *  getType() const { return type; }

      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const NamedType*) { return true; }
      static inline bool classof(const Node* T) { return T->is(NamedTypeID); }

    /// @}
    /// @name Mutators
    /// @{
    public:
      // Set the name for the type
      void setName(const std::string& n) { name = n; }
      
      // Set the type for the name
      void setType(Type * t) { type = t; }

    /// @}
    /// @name Data
    /// @{
    protected:
      Type* type;
      std::string name;
    /// @}
  };

  class AnyType : public Type
  {
    /// @name Constructors
    /// @{
    public:
      AnyType() : Type(AnyTypeID) {}
      virtual ~AnyType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const AnyType*) { return true; }
      static inline bool classof(const Type* T) { return T->isAnyType(); }
      static inline bool classof(const Node* T) { return T->is(AnyTypeID); }
    /// @}
    friend class AST;
  };

  class BooleanType : public Type
  {
    /// @name Constructors
    /// @{
    public:
      BooleanType() : Type(BooleanTypeID) {}
      virtual ~BooleanType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const BooleanType*) { return true; }
      static inline bool classof(const Type* T) { return T->isBooleanType(); }
      static inline bool classof(const Node* T) { return T->is(BooleanTypeID); }
    /// @}
    friend class AST;
  };

  class CharacterType : public Type
  {
    /// @name Constructors
    /// @{
    public:
      CharacterType() : Type(CharacterTypeID) {}
      virtual ~CharacterType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const CharacterType*) { return true; }
      static inline bool classof(const Type* T) { return T->isCharacterType(); }
      static inline bool classof(const Node* T) 
        { return T->is(CharacterTypeID); }
    /// @}
    friend class AST;
  };

  class OctetType : public Type
  {
    /// @name Constructors
    /// @{
    public:
      OctetType() : Type(OctetTypeID) {}
      virtual ~OctetType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const OctetType*) { return true; }
      static inline bool classof(const Type* T) { return T->isOctetType(); }
      static inline bool classof(const Node* T) { return T->is(OctetTypeID); }
    /// @}
    friend class AST;
  };

  class VoidType : public Type
  {
    /// @name Constructors
    /// @{
    public:
      VoidType() : Type(VoidTypeID) {}
      virtual ~VoidType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const VoidType*) { return true; }
      static inline bool classof(const Type* T) { return T->isVoidType(); }
      static inline bool classof(const Node* T) { return T->is(VoidTypeID); }
    /// @}
    friend class AST;
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
      IntegerType() : Type(IntegerTypeID), numBits(32), signedness(true) {}
      virtual ~IntegerType();

    /// @}
    /// @name Accessors
    /// @{
    public:

      /// Return the number of bits
      uint64_t getBits()  const { return numBits; }

      /// Return the signedness of the type
      bool     isSigned() const { return signedness ; }

      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const IntegerType*) { return true; }
      static inline bool classof(const Type* T) { return T->isIntegerType(); }
      static inline bool classof(const Node* T) { return T->is(IntegerTypeID); }

    /// @}
    /// @name Accessors
    /// @{
    public:
      /// Set the number of bits for this integer type
      void setBits(uint64_t bits) { numBits = bits; }

      /// Set the signedness of the type
      void setSigned(bool isSigned) { signedness = isSigned; }

    /// @}
    /// @name Data
    /// @{
    protected:
      uint32_t numBits; ///< Minimum number of bits
      bool signedness;  ///< Whether the integer type is signed or not

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
      RangeType() : Type(RangeTypeID), min(0), max(256) {}
      virtual ~RangeType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      /// Get min value of range
      int64_t getMin() { return min; }

      /// Get max value of range
      int64_t getMax() { return max; }

      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const RangeType*) { return true; }
      static inline bool classof(const Type* T) { return T->isRangeType(); }
      static inline bool classof(const Node* T) { return T->is(RangeTypeID); }

    /// @}
    /// @name Accessors
    /// @{
    public:
      /// Set min value of range
      void setMin(int64_t val) { min = val; }

      /// Set max value of range
      void setMax(int64_t val) { max = val; }

    /// @}
    /// @name Data
    /// @{
    protected:
      int64_t min; ///< Lowest value accepted
      int64_t max; ///< Highest value accepted
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
      RealType() : Type(RealTypeID), mantissa(52), exponent(11) {}
      virtual ~RealType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      /// Get the mantissa bits
      uint32_t getMantissa() { return mantissa; }

      /// Get the exponent bits
      uint32_t getExponent() { return exponent; }

      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const RealType*) { return true; }
      static inline bool classof(const Type* T) { return T->isRealType(); }
      static inline bool classof(const Node* T) { return T->is(RealTypeID); }

    /// @}
    /// @name Mutators
    /// @{
    public:
      /// Set the mantissa bits
      void setMantissa(uint32_t bits) { mantissa = bits; }

      /// Set the exponent bits
      void setExponent(uint32_t bits) { exponent = bits; }

    /// @}
    /// @name Data
    /// @{
    protected:
      uint32_t mantissa;  ///< Number of decimal digits in mantissa
      uint32_t exponent; ///< Number of decimal digits of precision
    /// @}
    friend class AST;
  };
} // AST
} // hlvm
#endif
