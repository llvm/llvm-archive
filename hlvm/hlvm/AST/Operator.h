//===-- AST Operator Class --------------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Operator.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Operator
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_OPERATOR_H
#define HLVM_AST_OPERATOR_H

#include <hlvm/AST/Node.h>

namespace hlvm 
{

class Type; 

/// This class is the abstract superclass for all Operators. It provides the
/// methods and virtual signature that is common to all Operator nodes.
/// @brief HLVM AST Abstract Operator Node
class Operator : public Value
{
  /// @name Constructors
  /// @{
  protected:
    Operator(NodeIDs opID) : Value(opID)  {}
    virtual ~Operator();

  /// @}
  /// @name Accessors
  /// @{
  public:
    /// Get a specific operand of this operator.
    virtual size_t  numOperands() const = 0;
    virtual Operator* getOperand(unsigned opnum) const = 0;

    /// Determine if this is a classof some other type.
    static inline bool classof(const Operator*) { return true; }
    static inline bool classof(const Node* N) { return N->isOperator(); }

  /// @}
  friend class AST;
};

// An operator that takes no operands
class NilaryOperator : public Operator
{
  /// @name Constructors
  /// @{
  protected:
    NilaryOperator(NodeIDs opID ) : Operator(opID) {}
    virtual ~NilaryOperator();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual size_t  numOperands() const;
    virtual Operator* getOperand(unsigned opnum) const;
    static inline bool classof(const NilaryOperator*) { return true; }
    static inline bool classof(const Node* N) { return N->isNilaryOperator(); }

  /// @}
  /// @name Mutators
  /// @{
  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);
  /// @}
  friend class AST;
};

class UnaryOperator : public Operator
{
  /// @name Constructors
  /// @{
  protected:
    UnaryOperator(NodeIDs opID) : Operator(opID), op1(0) {}
    virtual ~UnaryOperator();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual size_t  numOperands() const;
    virtual Operator* getOperand(unsigned opnum) const;
    static inline bool classof(const UnaryOperator*) { return true; }
    static inline bool classof(const Node* N) { return N->isUnaryOperator(); }

  /// @}
  /// @name Mutators
  /// @{
  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);

  /// @}
  /// @name Data
  /// @{
  protected:
    Operator* op1;
  /// @}
  friend class AST;
};

class BinaryOperator : public Operator
{
  /// @name Constructors
  /// @{
  protected:
    BinaryOperator(NodeIDs opID) : Operator(opID)
    { ops[0] = ops[1] = 0; }
    virtual ~BinaryOperator();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual size_t  numOperands() const;
    virtual Operator* getOperand(unsigned opnum) const;
    static inline bool classof(const BinaryOperator*) { return true; }
    static inline bool classof(const Node* N) { return N->isBinaryOperator(); }

  /// @}
  /// @name Mutators
  /// @{
  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);

  /// @}
  /// @name Data
  /// @{
  protected:
    Operator* ops[2];
  /// @}
  friend class AST;
};

class TernaryOperator : public Operator
{
  /// @name Constructors
  /// @{
  protected:
    TernaryOperator(NodeIDs opID) : Operator(opID)
    { ops[0] = ops[1] = ops[2] = 0; }
    virtual ~TernaryOperator();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual size_t  numOperands() const;
    virtual Operator* getOperand(unsigned opnum) const;
    static inline bool classof(const TernaryOperator*) { return true; }
    static inline bool classof(const Node* N) { return N->isTernaryOperator(); }

  /// @}
  /// @name Mutators
  /// @{
  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);

  /// @}
  /// @name Data
  /// @{
  protected:
    Operator* ops[3];
  /// @}
  friend class AST;
};

class MultiOperator : public Operator
{
  /// @name Types
  /// @{
  public:
    typedef std::vector<Operator*> OpList;
    typedef OpList::iterator iterator;
    typedef OpList::const_iterator const_iterator;

  /// @}
  /// @name Constructors
  /// @{
  protected:
    MultiOperator(NodeIDs opID) : Operator(opID), ops() {}
    virtual ~MultiOperator();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual size_t  numOperands() const;
    virtual Operator* getOperand(unsigned opnum) const;
    static inline bool classof(const MultiOperator*) { return true; }
    static inline bool classof(const Node* N) { return N->isMultiOperator(); }

  /// @}
  /// @name Iterators
  /// @{
  public:
    iterator       begin()       { return ops.begin(); }
    const_iterator begin() const { return ops.begin(); }
    iterator       end  ()       { return ops.end(); }
    const_iterator end  () const { return ops.end(); }
    size_t         size () const { return ops.size(); }
    bool           empty() const { return ops.empty(); }
    Operator*      front()       { return ops.front(); }
    const Operator*front() const { return ops.front(); }
    Operator*      back()        { return ops.back(); }
    const Operator*back()  const { return ops.back(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void addOperand(Operator* op) { op->setParent(this); }
  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);

  /// @}
  /// @name Data
  /// @{
  protected:
    OpList ops; ///< The operands of this Operator
  /// @}
  friend class AST;
};

} // hlvm 
#endif
