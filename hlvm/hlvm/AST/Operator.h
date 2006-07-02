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
class Bundle;
class Function;
class Block;
class LoopOp;

/// This class is the abstract base class in the Abstract Syntax Tree for all
/// operators. An Operator is an instruction to the virtual machine to take
/// some action. Operators form the content of a Block.  As this is the base
/// class of all operators, the Operator class only provides the functionality
/// that is common to all operators: getting the number of operands
/// (getNumOperands), getting the Value of an operand (getOperand), and  
/// setting 
/// the Value of an operand (setOperand). Since Operand is a Value, this implies
/// two things: (1) Operators can be the operand of other operators and (2) eery
/// Operator has a type.
/// @see Value
/// @see Block
/// @brief AST Abstract Operator Node
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
    virtual size_t  getNumOperands() const = 0;
    virtual Operator* getOperand(unsigned opnum) const = 0;

    /// Return the function containing this operator
    Function* getContainingFunction();

    /// Return the block containing this operator
    Block* getContainingBlock();

    /// Return the loop operator containing this operator. This can return
    /// any of the loop constructs (Loop, While, Unless, etc.) so its result
    /// type is Operator*.
    Operator* getContainingLoop();

    /// Return the bundle that contains the function that contains the block
    /// that contains this operator.
    Bundle* getContainingBundle();

    /// Determine if this is a classof some other type.
    static inline bool classof(const Operator*) { return true; }
    static inline bool classof(const Node* N) { return N->isOperator(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    virtual void setOperand(unsigned opnum, Operator* oprnd) = 0;

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree base class for all Operators
/// that have no operands.
/// @brief AST Operator With No Operands
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
    virtual size_t  getNumOperands() const;
    virtual Operator* getOperand(unsigned opnum) const;
    static inline bool classof(const NilaryOperator*) { return true; }
    static inline bool classof(const Node* N) { return N->isNilaryOperator(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    virtual void setOperand(unsigned opnum, Operator* oprnd);

  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree base class for Operators that 
/// take a single operand.
/// @brief AST Operator With One Operand
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
    Operator* getOperand() const { return op1; }
    virtual size_t  getNumOperands() const;
    virtual Operator* getOperand(unsigned opnum) const;
    static inline bool classof(const UnaryOperator*) { return true; }
    static inline bool classof(const Node* N) { return N->isUnaryOperator(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    virtual void setOperand(unsigned opnum, Operator* oprnd);
  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);
    virtual void setOperand(Operator* oprnd) { op1 = oprnd; }

  /// @}
  /// @name Data
  /// @{
  protected:
    Operator* op1;
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree base class for all operators
/// that have two operands. The operands may be of any Type.
/// @brief AST Operator With Two Operands
class BinaryOperator : public Operator
{
  /// @name Constructors
  /// @{
  protected:
    BinaryOperator(NodeIDs opID) : Operator(opID) { ops[0] = ops[1] = 0; }
    virtual ~BinaryOperator();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual size_t  getNumOperands() const;
    virtual Operator* getOperand(unsigned opnum) const;
    static inline bool classof(const BinaryOperator*) { return true; }
    static inline bool classof(const Node* N) { return N->isBinaryOperator(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    virtual void setOperand(unsigned opnum, Operator* oprnd);
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

/// This class provides an Abstract Syntax Tree base class for all operators
/// that have three operands. The operands may be of any Type.
/// @brief AST Operator With Three Operands
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
    virtual size_t  getNumOperands() const;
    virtual Operator* getOperand(unsigned opnum) const;
    static inline bool classof(const TernaryOperator*) { return true; }
    static inline bool classof(const Node* N) { return N->isTernaryOperator(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    virtual void setOperand(unsigned opnum, Operator* oprnd);
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

/// This class provides an Abstract Syntax Tree base class for all operators
/// that have multiple operands. The operands may be of any Type. Although the
/// interface to this class permits any number of operands, in practice the
/// number of allowed operands for a given MultiOperator subclass is limited. 
/// The subclass's insertChild and removeChild method overrides will enforce
/// the correct arity for that subclass. 
/// @brief AST Operator With Multiple Operands
class MultiOperator : public Operator
{
  /// @name Types
  /// @{
  public:
    typedef std::vector<Operator*> OprndList;
    typedef OprndList::iterator iterator;
    typedef OprndList::const_iterator const_iterator;

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
    virtual size_t  getNumOperands() const;
    virtual Operator* getOperand(unsigned opnum) const;
    static inline bool classof(const MultiOperator*) { return true; }
    static inline bool classof(const Node* N) { return N->isMultiOperator(); }

  /// @}
  /// @name Iterators
  /// @{
  public:
    iterator        begin()       { return ops.begin(); }
    const_iterator  begin() const { return ops.begin(); }
    iterator        end  ()       { return ops.end(); }
    const_iterator  end  () const { return ops.end(); }
    size_t          size () const { return ops.size(); }
    bool            empty() const { return ops.empty(); }
    Operator*       front()       { return ops.front(); }
    const Operator* front() const { return ops.front(); }
    Operator*       back()        { return ops.back(); }
    const Operator* back()  const { return ops.back(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    virtual void setOperand(unsigned opnum, Operator* oprnd);
    void addOperand(Operator* v) { v->setParent(this); }
    void addOperands(const OprndList& new_ops); 
  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);

  /// @}
  /// @name Data
  /// @{
  protected:
    OprndList ops; ///< The operands of this Operator
  /// @}
  friend class AST;
};

} // end hlvm namespace
#endif
