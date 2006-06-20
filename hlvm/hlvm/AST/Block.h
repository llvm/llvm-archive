//===-- AST Function Class --------------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Block.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Block
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_BLOCK_H
#define HLVM_AST_BLOCK_H

#include <hlvm/AST/Operator.h>
#include <map>

namespace hlvm 
{

class AutoVarOp;

/// This class represents an block of operators in the HLVM Abstract Syntax 
/// Tree.  A block is simply a sequential list of Operator nodes that are
/// executed in sequence. Block itself is an operator. Its result value is
/// the value of the last operator executed. As such, blocks can be nested
/// within blocks. Blocks are used as the operands of the control flow 
/// operators as well.
/// @brief AST Block Node
class Block : public MultiOperator
{
  /// @name Constructors
  /// @{
  protected:
    Block() : MultiOperator(BlockID){}
    virtual ~Block();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const Type* getType() const { return this->back()->getType(); }
    const std::string& getLabel() const { return label; }
    AutoVarOp*   getAutoVar(const std::string& name) const; 
    const Type* getResultType() { return this->back()->getType(); }
    Block* getParentBlock() const;
    static inline bool classof(const Block*) { return true; }
    static inline bool classof(const Node* N) { return N->is(BlockID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setLabel(const std::string& l) { label = l; }
  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);
  /// @}
  /// @name Data
  /// @{
  private:
    typedef std::map<std::string,AutoVarOp*> AutoVarMap;
    std::string label;
    AutoVarMap  autovars;
  /// @}
  friend class AST;
};

} // hlvm 
#endif
