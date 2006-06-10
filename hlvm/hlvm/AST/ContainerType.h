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

/// This class represents a Type that uses a single other element type in its
/// construction.
class UniformContainerType : public Type
{
  /// @name Constructors
  /// @{
  protected:
    UniformContainerType(NodeIDs id) : Type(id), type(0) {}
    virtual ~UniformContainerType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    Type* getElementType() const { return type; }
    virtual const char* getPrimitiveName() const; // asserting override
    // Methods to support type inquiry via isa, cast, dyn_cast
    static inline bool classof(const UniformContainerType*) { return true; }
    static inline bool classof(const Type* T) { 
      return T->isUniformContainerType(); 
    }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setElementType(Type* t) { type = t; }

  protected:
    virtual void insertChild(Node* n);
    virtual void removeChild(Node* n);

  /// @}
  /// @name Data
  /// @{
  protected:
    Type* type; ///< The contained types
  /// @}
};

/// This class represents a storage location that is a pointer to another
/// type. 
class PointerType : public UniformContainerType
{
  /// @name Constructors
  /// @{
  protected:
    PointerType() : UniformContainerType(PointerTypeID) {}
    virtual ~PointerType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const PointerType*) { return true; }
    static inline bool classof(const Node* T) { return T->is(PointerTypeID); }

  /// @}
  friend class AST;
};

/// This class represents a resizeable, aligned array of some other type. The
/// Array references a Type that specifies the type of elements in the array.
class ArrayType : public UniformContainerType
{
  /// @name Constructors
  /// @{
  protected:
    ArrayType() : UniformContainerType(ArrayTypeID), maxSize(0) {}
    virtual ~ArrayType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    /// Get the maximum size the array can grow to.
    uint64_t getMaxSize()  const { return maxSize; }

    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const ArrayType*) { return true; }
    static inline bool classof(const Node* T) { return T->is(ArrayTypeID); }
    
  /// @}
  /// @name Mutators
  /// @{
  public:
    /// Set the maximum size the array can grow to.
    void setMaxSize(uint64_t max) { maxSize = max; }

  /// @}
  /// @name Data
  /// @{
  protected:
    uint64_t maxSize; ///< The maximum number of elements in the array
  /// @}
  friend class AST;
};

/// This class represents a fixed size, packed vector of some other type.
/// Where possible, HLVM will attempt to generate code that makes use of a
/// machines vector instructions to process such types. If not possible, HLVM
/// will treat the vector the same as an Array.
class VectorType : public UniformContainerType
{
  /// @name Constructors
  /// @{
  protected:
    VectorType() : UniformContainerType(VectorTypeID), size(0) {}
    virtual ~VectorType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    /// Get the maximum size the array can grow to.
    uint64_t getSize()  const { return size; }

    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const VectorType*) { return true; }
    static inline bool classof(const Node* T) { return T->is(VectorTypeID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    /// Set the size of the vector.
    void setSize(uint64_t max) { size = max; }

  /// @}
  /// @name Data
  /// @{
  protected:
    uint64_t size; ///< The (fixed) size of the vector
  /// @}
  friend class AST;
};

/// This class is type that combines a name with an arbitrary type. This
/// construct is used any where a named and typed object is needed such as
/// the parameter to a function or the field of a structure. 
class AliasType : public UniformContainerType
{
  /// @name Constructors
  /// @{
  protected:
    AliasType() : UniformContainerType(AliasTypeID) {}
    virtual ~AliasType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const;
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const AliasType*) { return true; }
    static inline bool classof(const Node* T) { return T->is(AliasTypeID); }

  /// @}
  friend class AST;
};

/// This class represents a Type in the HLVM Abstract Syntax Tree.  
/// A Type defines the format of storage. 
/// @brief HLVM AST Type Node
class DisparateContainerType : public Type
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
    DisparateContainerType(NodeIDs id) : Type(id), contents() {}
    virtual ~DisparateContainerType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const;
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const DisparateContainerType*) { return true; }
    static inline bool classof(const Node* N) { 
      return N->isDisparateContainerType(); }

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
class StructureType : public DisparateContainerType
{
  /// @name Constructors
  /// @{
  protected:
    StructureType(NodeIDs id = StructureTypeID ) : 
      DisparateContainerType(id) {}
    virtual ~StructureType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const StructureType*) { return true; }
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
  friend class AST;
};

/// This class holds data for a continuation. TBD later.
class ContinuationType : public StructureType
{
  /// @name Constructors
  /// @{
  protected:
    ContinuationType() : StructureType(ContinuationTypeID) {}
    virtual ~ContinuationType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const ContinuationType*) { return true; }
    static inline bool classof(const Node* T) 
      { return T->is(ContinuationTypeID); }
  /// @}

  friend class AST;
};

typedef AliasType Argument;

/// This class represents an HLVM type that is a sequence of data fields 
/// of varying type. 
class SignatureType : public DisparateContainerType
{
  /// @name Constructors
  /// @{
  protected:
    SignatureType() 
      : DisparateContainerType(SignatureTypeID), result(0), varargs(false) {}
    virtual ~SignatureType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const Type* getResultType() const { return result; }
    bool  isVarArgs() const { return varargs; }

    // Methods to support type inquiry via is, cast, dyn_cast
    static inline bool classof(const SignatureType*) { return true; }
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
