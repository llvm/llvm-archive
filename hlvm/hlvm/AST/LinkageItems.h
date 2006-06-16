//===-- AST Linkage Items Interface -----------------------------*- C++ -*-===//
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
/// @file hlvm/AST/LinkageItems.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/10
/// @since 0.1.0
/// @brief Declares the interface to all subclasses of LinkageItem
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_LINKAGEITEMS_H
#define HLVM_AST_LINKAGEITEMS_H

#include <hlvm/AST/LinkageItem.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Block.h>

namespace hlvm 
{

class Type; // Forward declare
class Constant;

/// This class provides an Abstract Syntax Tree node that represents a 
/// global Variable.  A Variable can only be declared as a component of a 
/// Bundle. It is visible throughout the Bundle that declares it and may 
/// be a candidate for linkage with other Bundles. A Variable is a storage 
/// location, with an address, of a specific type. Global variables may have
/// a constant value in which case HLVM will ensure that the value of the
/// global variable is immutable. Variables can be of any type except VoidType.
/// @see LinkageItem
/// @see Bundle
/// @brief AST Variable Node
class Variable : public LinkageItem
{
  /// @name Constructors
  /// @{
  protected:
    Variable() : LinkageItem(VariableID) {}
  public:
    virtual ~Variable();

  /// @}
  /// @name Accessors
  /// @{
  public:
    bool isConstant() const { return flags & 0x0008; }
    Constant* getInitializer() { return init; }
    static inline bool classof(const Variable*) { return true; }
    static inline bool classof(const Node* N) { return N->is(VariableID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setIsConstant(bool v) { flags |= 0x0008; }
    void setInitializer(Constant* C) { init = C; }

  /// @}
  /// @name Data
  /// @{
  protected:
    Constant* init;
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a Function.
/// A Function is a callable block of code that accepts parameters and 
/// returns a result.  This is the basic unit of code in HLVM. A Function
/// has a name, a set of formal arguments, a return type, and, optionally, a 
/// Block of code to execute. The name of a function is used for linking 
/// purposes. The formal arguments and return type are encapsulated in the
/// Function's associated SignatureType. If a Block is associated with the
/// Function then the function is defined and the Block defines the computation
/// the Function provides. If a Block is not associated with the Function, then
/// the function is undefined and serves as a reference to a function in another
/// Bundle.
/// @see Block
/// @see Bundle
/// @see SignatureType
/// @brief AST Function Node
class Function : public LinkageItem
{
  /// @name Constructors
  /// @{
  public:
    Function(
      NodeIDs id = FunctionID
    ) : LinkageItem(id), block(0) {}
    virtual ~Function();

  /// @}
  /// @name Accessors
  /// @{
  public:
    Block* getBlock() const { return block; }
    const SignatureType* getSignature() const;
    static inline bool classof(const Function*) { return true; }
    static inline bool classof(const Node* N) { return N->isFunction(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    virtual void insertChild(Node* kid);
    virtual void removeChild(Node* kid);
    void setSignature(SignatureType* sig) { type = sig; }
    void setBlock(Block* blk) { blk->setParent(this); }

  /// @}
  /// @name Data
  /// @{
  protected:
    Block * block;                   ///< The code block to be executed
  /// @}
  friend class AST;
};

class Block; // Forward declare
class SignatureType;  // Forward declare

/// This class provides an Abstract Syntax Tree node that represents a Program 
/// in HLVM. A Program is an entry point for running HLVM programs. It is 
/// simply a Function with a specific signature. It represents the first
/// function to be executed by the runtime after option processing.  To be
/// executable, a Bundle must have at least one Program node in it. 
/// The Program node exists to simply ensure that the signature of the function
/// is correct and to serve as a way to identify Program's quickly.
/// @see Function
/// @see Bundle
/// @see SignatureType
/// @brief AST Program Node
class Program : public Function
{
  /// @name Constructors
  /// @{
  protected:
    Program() : Function(ProgramID) {}
      
    virtual ~Program();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const Program*) { return true; }
    static inline bool classof(const Node* N) { return N->is(ProgramID); }

  /// @}
  /// @name Data
  /// @{
  private:
    LinkageItem::setLinkageKind;
  /// @}
  friend class AST;
};

} // end hlvm namespace
#endif
