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
/// @file hlvm/AST/Type.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Type
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_TYPE_H
#define HLVM_AST_TYPE_H

#include <hlvm/AST/Node.h>

namespace hlvm 
{

/// This class provides the Abstract Syntax Tree base class for all Types. A
/// Type describes the memory layout of a Value. There are many subclasses of
/// of Type, each with a particular way of describing memory layout. In HLVM,
/// Type resolution is done by name. That is, two types with identical layout
/// but different names are not equivalent.
/// @brief HLVM AST Type Node
class Type : public Documentable
{
  /// @name Constructors
  /// @{
  protected:
    Type( NodeIDs id ) : Documentable(id)  {}
    virtual ~Type();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string& getName() const { return name; }
    virtual const char* getPrimitiveName() const;
    bool isPrimitive() const { return getPrimitiveName() != 0; }

  /// @}
  /// @name Type Identification
  /// @{
  public:

    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const Type*) { return true; }
    static inline bool classof(const Documentable* n) { return n->isType(); }
    static inline bool classof(const Node* n) { return n->isType(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    // We override receiveChild generically here to produce an error. Most
    // Type subclasses can't receive children. Those that do, can override
    // again.
    virtual void insertChild(Node* n);

    virtual void setName(const std::string n) { name = n; }

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string name;
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node for describing a type that
/// can accept any type of Value. The AnyType can be used to represent
/// dynamically typed variables and it, essentially, bypasses the HLVM type 
/// system but in a type-safe way.  The AnyType instances will, internally,
/// carry a type identifier with the value. If the value changes to a new type,
/// then the type identifier changes with it. In this way, the correct type for
/// whatever is assigned to an AnyType is maintained by the runtime so that it
/// cannot be mis-used.
/// @brief AST Dynamically Typed Type
class AnyType : public Type
{
  /// @name Constructors
  /// @{
  protected:
    AnyType() : Type(AnyTypeID) {}
    virtual ~AnyType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const;

    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const AnyType*) { return true; }
    static inline bool classof(const Node* T) { return T->is(AnyTypeID); }
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node to represent the boolean
/// type. Booleans have a simple true-or-false value and could be represented by
/// a single bit.
/// @brief AST Boolean Type
class BooleanType : public Type
{
  /// @name Constructors
  /// @{
  protected:
    BooleanType() : Type(BooleanTypeID) {}
    virtual ~BooleanType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const;
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const BooleanType*) { return true; }
    static inline bool classof(const Node* T) { return T->is(BooleanTypeID); }
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node to represent the character
/// type. A Character is a UTF-16 encoded value. It is sixteen bits long and
/// interpreted with the UTF-16 codeset. 
/// @brief AST Character Type
class CharacterType : public Type
{
  /// @name Constructors
  /// @{
  protected:
    CharacterType() : Type(CharacterTypeID) {}
    virtual ~CharacterType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const;
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const CharacterType*) { return true; }
    static inline bool classof(const Node* T) { return T->is(CharacterTypeID); }
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an octet
/// type. Octets are simply 8 bits of data without interpretation. Octets are
/// used as the element type of a buffer. Octets cannot be used in arithmetic
/// computation. They simply hold 8 bits of data.  Octets are commonly used in
/// stream I/O to represent the raw data flowing on the stream. 
/// @brief AST Octet Type
class OctetType : public Type
{
  /// @name Constructors
  /// @{
  protected:
    OctetType() : Type(OctetTypeID) {}
    virtual ~OctetType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const;
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const OctetType*) { return true; }
    static inline bool classof(const Node* T) { return T->is(OctetTypeID); }
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents the void
/// type.  The void type represents a Value that has no value. It is zero bits
/// long. Consequently, its utility is limited. 
/// @brief AST Void Type
class VoidType : public Type
{
  /// @name Constructors
  /// @{
  protected:
    VoidType() : Type(VoidTypeID) {}
    virtual ~VoidType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const;
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const VoidType*) { return true; }
    static inline bool classof(const Node* T) { return T->is(VoidTypeID); }
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an integer
/// type in HLVM. An integer is simply a sequence of bits, of specific length,
/// that is interpreted to be an integer value.  An integer type declares the
/// the minimum number of bits that are required to store the integer type. The
/// runtime may, for pragmatic reason, choose to store the type in more bits 
/// than specified by the type. There are a number of "primitive" integer types
/// of specific size, that line up with the natural word sizes of an underlying
/// machines. Such types are preferred because they imply less translation and
/// a more efficient computation.  If the number of bits is specified as zero,
/// it implies infinite precision integer arithmetic. Integer types may be
/// declared as either signed or unsigned. The maximum number of bits for an 
/// integer type that may be specified is 32,767. Larger integer types should
/// use infinite precision (number of bits = 0).
class IntegerType : public Type
{
  /// @name Constructors
  /// @{
  protected:
    IntegerType(
      NodeIDs id,  ///< The node type identifier, passed on to Node base class
      int16_t bits = 32, ///< The number of bits in this integer type
      bool sign = true ///< Indicates if this is a signed integer type, or not.
    ) : Type(id) {
      setBits(bits);
      setSigned(sign); 
    }

  public:
    virtual ~IntegerType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    /// Get the primitive name for this type. That is, if this type conforms to
    /// one of the primitive integer types, then return that primitive's name.
    virtual const char* getPrimitiveName() const;

    /// @brief Return the number of bits in this integer type
    int16_t getBits()  const { return int16_t(flags & 0x7FFF); }

    /// @brief Return the signedness of this type
    bool     isSigned() const { return flags & 0x8000; }

    /// @brief Methods to support type inquiry via isa, cast, dyn_cast
    static inline bool classof(const IntegerType*) { return true; }
    static inline bool classof(const Node* T) { return T->isIntegralType(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    /// An int
    /// @brief Set the number of bits for this integer type
    void setBits(int16_t bits) { 
      flags &= 0x8000; flags |= uint16_t(bits)&0x7FFF; }

    /// @brief Set the signedness of the type
    void setSigned(bool isSigned) { 
      if (isSigned) flags |= 0x8000; else flags &= 0x7FFF; }

  /// @}
  friend class AST;
};

/// This class specifies an Abstract Syntax Tree node that represents a range
/// type. A RangeType is an integral type that constricts the set of allowed 
/// values for the integer to be within an inclusive range. The number of bits
/// required to represent the range is selected by HLVM. The selected integer
/// size will contain sufficient bits to represent the requested range. Range
/// types are limited to values within the limits of a signed 64-bit integer.
/// The use of RangeType implies range checking whenever the value of a 
/// RangeType variable is assigned.
class RangeType: public Type
{
  /// @name Constructors
  /// @{
  protected:
    RangeType() : Type(RangeTypeID), min(0), max(256) {}
  public:
    virtual ~RangeType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const;
    /// Get min value of range
    int64_t getMin() { return min; }

    /// Get max value of range
    int64_t getMax() { return max; }

    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const RangeType*) { return true; }
    static inline bool classof(const Node* T) { return T->is(RangeTypeID); }

  /// @}
  /// @name Mutators
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

/// This class provides an Abstract Syntax Tree node that represents an 
/// enumeration of names. Enumerations consist of a set of enumerators. An
/// enumerator is simply a name that uniquely identifies one of the possible
/// values of the EnumerationType.  Enumerators are not integers as in some 
/// other languages. HLVM will select a representation that is efficient based 
/// on the characteristics of the EnumerationType. The enumerators define, by 
/// their order of insertion into the enumeration, a collation order for the
/// enumerators. This collation order can be used to test enumeration values
/// for equivalence or inequivalence using the equality and inequality 
/// operators. 
/// @brief AST Enumeration Type
class EnumerationType : public Type
{
  /// @name Types
  /// @{
  public:
    typedef std::vector<std::string> EnumeratorList;
    typedef EnumeratorList::iterator iterator;
    typedef EnumeratorList::const_iterator const_iterator;

  /// @}
  /// @name Constructors
  /// @{
  protected:
    EnumerationType() : Type(EnumerationTypeID), enumerators() {}
  public:
    virtual ~EnumerationType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const;
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const EnumerationType*) { return true; }
    static inline bool classof(const Node* T) 
      { return T->is(EnumerationTypeID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void addEnumerator(const std::string& en) { enumerators.push_back(en); }

  /// @}
  /// @name Iterators
  /// @{
  public:
    iterator          begin()       { return enumerators.begin(); }
    const_iterator    begin() const { return enumerators.begin(); }
    iterator          end  ()       { return enumerators.end(); }
    const_iterator    end  () const { return enumerators.end(); }
    size_t            size () const { return enumerators.size(); }
    bool              empty() const { return enumerators.empty(); }
    std::string       front()       { return enumerators.front(); }
    const std::string front() const { return enumerators.front(); }
    std::string       back()        { return enumerators.back(); }
    const std::string back()  const { return enumerators.back(); }

  /// @}
  /// @name Data
  /// @{
  protected:
    EnumeratorList enumerators; ///< The list of the enumerators
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a real
/// number. All floating point and real number arithmetic is done using values
/// of this type. The precision and mantissa are specified as a number of 
/// binary digits to be provided as a minimum.  HLVM will use the machine's 
/// natural floating point representation for those RealTypes that can fit 
/// within the precision and mantissa lengths supported by the machine. 
/// Otherwise, if a machine floating point type cannot meet the requirements of
/// the RealType, infinite precision floating point arithmetic will be utilized.
/// @brief AST Real Number Type
class RealType : public Type
{
  /// @name Constructors
  /// @{
  protected:
    RealType(NodeIDs id, uint32_t m=52, uint32_t x=11) 
      : Type(id), mantissa(m), exponent(x) {}
  public:
    virtual ~RealType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const;
    /// Get the mantissa bits
    uint32_t getMantissa() { return mantissa; }

    /// Get the exponent bits
    uint32_t getExponent() { return exponent; }

    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const RealType*) { return true; }
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
    uint32_t mantissa; ///< Number of bits in mantissa
    uint32_t exponent; ///< Number of bits of precision
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an opaque
/// type. Opaque types are those whose definition is not known. Opaque types are
/// used to handle forward type references and recursion within container types.
/// HLVM will automatically resolve OpaqueTypes whose definition is later 
/// discovered. It can also be used as the element type of a PointerType to 
/// effectively hide the implementation details of a given type. This helps
/// libraries to retain control over the implementation details of a type that
/// is to be treated as generic by the library's users.
/// @see PointerType
/// @brief AST Opaque Type
class OpaqueType : public Type
{
  /// @name Constructors
  /// @{
  protected:
    OpaqueType(const std::string& nm) : 
      Type(OpaqueTypeID) { this->setName(nm); }
  public:
    virtual ~OpaqueType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const OpaqueType*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(OpaqueTypeID); }

  /// @}
  friend class AST;
};

} // hlvm
#endif
