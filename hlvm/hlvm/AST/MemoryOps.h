//===-- HLVM AST Memory Operations Interface --------------------*- C++ -*-===//
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
/// @file hlvm/AST/MemoryOps.h
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/18
/// @since 0.1.0
/// @brief Provides the interface to the HLVM memory operations.
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_MEMORYOPS_H
#define HLVM_AST_MEMORYOPS_H

#include <hlvm/AST/Operator.h>

namespace hlvm
{

class Variable;
class Constant;
class ConstantValue;
class Function;

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for allocating memory from the heap. The type of this operator indicates the
/// element size for the allocation. The single operand must be of an integral
/// type and indicates the number of elements to allocate.
/// @brief AST Memory Allocation Operator
class AllocateOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    AllocateOp() : UnaryOperator(AllocateOpID) {}
    virtual ~AllocateOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const AllocateOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(AllocateOpID); }

  /// @}
  /// @name Data
  /// @{
  protected:
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for deallocating memory from the heap. This operator requires one operand
/// which must be the value returned from a previous AllocateOp.
/// @brief AST Memory Deallocation Operator
class DeallocateOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    DeallocateOp() : UnaryOperator(DeallocateOpID) {}
    virtual ~DeallocateOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const DeallocateOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(DeallocateOpID); }

  /// @}
  /// @name Data
  /// @{
  protected:
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for loading a value from a memory location. This operator takes a single
/// operand which must resolve to the address of a memory location, either 
/// global or local (stack). The result of the operator is the value of the
/// memory object, whatever type it may be.
/// @brief AST Memory Load Operator
class LoadOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    LoadOp() : UnaryOperator(LoadOpID) {}
    virtual ~LoadOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const LoadOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(LoadOpID); }

  /// @}
  /// @name Mutators
  /// @{
  public:

  /// @}
  /// @name Data
  /// @{
  protected:
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for storing a value into a memory location. The first operand 
/// resolves to the storage location into which the value is stored. The second
/// operand provides the value to store. The operator returns the value stored.
/// @brief AST Memory Set Operator
class StoreOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    StoreOp() : BinaryOperator(StoreOpID) {}
    virtual ~StoreOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const StoreOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(StoreOpID); }

  /// @}
  /// @name Mutators
  /// @{
  public:

  /// @}
  /// @name Data
  /// @{
  protected:
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for defining an automatic (local) variable on a function's stack. The scope
/// of such a variable is the block that contains it from the point at which
/// the declaration occurs, onward.  Automatic variables are allocated
/// automatically on a function's stack. When execution leaves the block in 
/// which the variable is defined, the variable is automatically deallocated. 
/// Automatic variables are not Linkables and do not participate in linkage
/// at all. They don't exist until a Function is activated. Automatic variables
/// are operators because they provide the value of their initializer. Automatic
/// variables may be declared to be constant in which case they must have an
/// initializer and their value is immutable.
/// @brief AST Automatic Variable Operator
class AutoVarOp : public NilaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    AutoVarOp() : NilaryOperator(AutoVarOpID), name(), initializer(0) {}
    virtual ~AutoVarOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string& getName() const { return name; }
    bool hasInitializer() const { return initializer != 0; }
    ConstantValue* getInitializer() const { return initializer; }
    bool isZeroInitialized() const { return initializer == 0; }
    static inline bool classof(const AutoVarOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(AutoVarOpID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setName(const std::string& n) { name = n; }
    void setInitializer(ConstantValue* C) { initializer = C; }

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string name;
    ConstantValue* initializer;
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for obtaining a value. The operator has no operands but has a property 
/// that is the Value to be referenced. The Value must be an AutoVarOp or one
/// of the Linkable (Variable, Constant, Function).  This operator
/// bridges between non-operator Values and Operators. The result of this
/// operator is the address of the memory object.  Typically this operator is
/// used as the operand of a LoadOp or StoreOp.
/// @see Variable AutoVarOp Operator Value Linkable LoadOp StoreOp
/// @brief AST Reference Operator
class ReferenceOp : public NilaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    ReferenceOp() : NilaryOperator(ReferenceOpID) {}
    virtual ~ReferenceOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const Value* getReferent() const { return referent; }
    static inline bool classof(const ReferenceOp*) { return true; }
    static inline bool classof(const Node* N) { 
      return N->is(ReferenceOpID); 
    }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setReferent(const Value* ref) { referent = ref; }

  /// @}
  /// @name Data
  /// @{
  protected:
    const Value* referent;
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for indexing into a ContainerType.  The Index operator can have many
/// operands but in all cases requires at least two.  The first operand must
/// resolve to the address of a memory location, such as returned by the
/// ReferenceOp. The second and subsequent operands must all be of integer type.
/// They specify which elements of the memory object should be indexed. This
/// operator is the means by which the elements of memory objects of type 
/// PointerType, ArrayType, VectorType, StructureType, and ContinuationType can
/// be accessed. The resulting value of the operator is the address of the
/// corresponding memory location. In the case of StructureType and 
/// ContinuationType elements, the corresponding index indicates the field, in
/// declaration order, that is accessed. Field numbering begins at zero.
/// @brief AST Index Operator
class IndexOp : public MultiOperator
{
  /// @name Constructors
  /// @{
  protected:
    IndexOp() : MultiOperator(IndexOpID) {}
    virtual ~IndexOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const IndexOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(IndexOpID); }

  /// @}
  /// @name Mutators
  /// @{
  public:

  /// @}
  /// @name Data
  /// @{
  protected:
  /// @}
  friend class AST;
};
} // hlvm

#endif
namespace hlvm {

}
