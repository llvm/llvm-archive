//===-- HLVM AST Memory Operations ------------------------------*- C++ -*-===//
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
#include <hlvm/AST/Constant.h>

namespace hlvm
{

class Variable;

/// This operator loads the value of a memory location.
/// @brief HLVM Memory Load Operator
class LoadOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    LoadOp() : UnaryOperator(LoadOpID) {}

  public:
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

/// This operator stores a value into a memory location. The first operand 
/// resolves to the storage location into which the value is stored. The second
/// operand provides the value to store.
/// @brief HLVM Memory Store Operator
class StoreOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    StoreOp() : BinaryOperator(StoreOpID) {}

  public:
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

/// This operator represents a local (stack) variable whose lifespan is the
/// lifespan of the enclosing block. Its value is the initialized value.
/// @brief HLVM AST Automatic Variable Operator
class AutoVarOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    AutoVarOp() : UnaryOperator(AutoVarOpID) {}

  public:
    virtual ~AutoVarOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string& getName() { return name; }
    Constant* getInitializer() { return static_cast<Constant*>(getOperand());}
    static inline bool classof(const AutoVarOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(AutoVarOpID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setName(const std::string& n) { name = n; }
    void setInitializer(Constant* c) { setOperand(c); }

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string name;
  /// @}
  friend class AST;
};

/// This operator yields the value of a named variable, either an automatic
/// variable (function scoped) or global variable (bundle scoped). 
/// @brief HLVM AST Variable Operator
class ReferenceOp : public NilaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    ReferenceOp() : NilaryOperator(ReferenceOpID) {}

  public:
    virtual ~ReferenceOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    Value* getReferent() const { return referent; }
    static inline bool classof(const ReferenceOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(ReferenceOpID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setReferent(Value* ref) { referent = ref; }

  /// @}
  /// @name Data
  /// @{
  protected:
    Value* referent;
  /// @}
  friend class AST;
};

/// This operator indexes into an Array and yields the address of an element of
/// the array.
/// @brief HLVM AST Variable Operator
class IndexOp : public MultiOperator
{
  /// @name Constructors
  /// @{
  protected:
    IndexOp() : MultiOperator(IndexOpID) {}

  public:
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
