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

namespace hlvm
{

/// This operator represents the load operation for loading a variable into a
/// register.
/// @brief HLVM AST Memory Load Operator
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

/// This operator represents the 
/// @brief HLVM AST String Insert Node
class StoreOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    StoreOp() : BinaryOperator(LoadOpID) {}

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

/// This operator yields the value of a named variable
/// @brief HLVM AST Variable Reference Operator
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
    const std::string getVarName() const { return varName; }
    static inline bool classof(const ReferenceOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(ReferenceOpID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setVarName(const std::string& name) { varName = name; }

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string varName;
  /// @}
  friend class AST;
};
} // hlvm

#endif
namespace hlvm {

}
