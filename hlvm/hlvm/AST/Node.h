//
// Copyright (C) 2006 HLVM Group. All Rights Reserved.
//
// This program is open source software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License (GPL) as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. You should have received a copy of the GPL in a
// file named COPYING that was included with this program; if not, you can
// obtain a copy of the license through the Internet at http://www.fsf.org/
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
////////////////////////////////////////////////////////////////////////////////
/// @file hlvm/AST/Node.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Node
////////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_AST_NODE_H
#define HLVM_AST_NODE_H

#include <llvm/Support/Casting.h>
#include <vector>
#include <string>

/// This namespace is for all HLVM software. It ensures that HLVM software does
/// not collide with any other software. Hopefully HLVM is not a namespace used
/// elsewhere. 
namespace hlvm
{
/// This namespace contains all the AST (Abstract Syntax Tree) module code. All
/// node types of the AST are declared in this namespace.
namespace AST
{
  /// This enumeration is used to identify a specific type
  enum NodeIDs {
    // Types
    VoidTypeID = 0,     ///< The Void Type
    IntegerTypeID,      ///< The Integer Type
    RangeTypeID,        ///< The Range Type
    RealTypeID,         ///< The Real Number Type
    StringTypeID,       ///< The String Type
    PointerTypeID,      ///< The Pointer Type
    ArrayTypeID,        ///< The Array Type
    VectorTypeID,       ///< The Vector Type
    StructureTypeID,    ///< The Structure Type
    SignatureTypeID,    ///< The Function Signature Type
    ClassID,            ///< The Class Type

    // Class Constructs (TBD)

    // Variables
    VariableID,         ///< The Variable Node

    // Containers
    BundleID,           ///< The Bundle Node
    FunctionID,         ///< The Function Node
    ProgramID,          ///< The Program Node
    BlockID,            ///< A Block Of Code Node

    // Control Flow And Invocation Operators
    CallOpID,           ///< The Call Operator
    InvokeOpID,         ///< The Invoke Operator
    DispatchOpID,       ///< The Object Method Dispatch  Operator
    CreateContOpID,     ///< The Create Continutation Operator
    CallWithContOpID,   ///< The Call with Continuation Operator
    ReturnOpID,         ///< The Return A Value Operator
    ThrowOpID,          ///< The Throw And Exception Operator
    JumpToOpID,         ///< The Jump To Labelled Block Operator
    BreakOpID,          ///< The Break Out Of Block Operator
    IfOpID,             ///< The If-Then-Else Operator
    LoopOpID,           ///< The General Purpose Loop Operator
    SelectOpID,         ///< The Select An Alternate Operator

    // Memory Operators
    LoadOpID,           ///< The Load Operator
    StoreOpID,          ///< The Store Operator
    AllocateOpID,       ///< The Allocate Memory Operator
    FreeOpID,           ///< The Free Memory Operator
    ReallocateOpID,     ///< The Reallocate Memory Operator
    ReferenceOpID,      ///< The Reference A Memory Object Operator (for GC)
    DereferenceOpID,    ///< The Dereference A Memory Object Operator (for GC)

    // Arithmetic Operators
    NegateOpID,         ///< The Negation Unary Integer Operator
    ComplementOpID,     ///< The Bitwise Complement Unary Integer Operator
    PreIncrOpID,        ///< The Pre-Increment Unary Integer Operator
    PostIncrOpID,       ///< The Post-Increment Unary Integer Operator
    PreDecrOpID,        ///< The Pre-Decrement Unary Integer Operator
    PostDecrOpID,       ///< The Post-Decrement Unary Integer Operator
    AddOpID,            ///< The Addition Binary Integer Operator
    SubtractOpID,       ///< The Subtraction Binary Integer Operator
    MultiplyOpID,       ///< The Multiplcation Binary Integer Operator
    DivideOpID,         ///< The Division Binary Integer Operator
    ModulusOpID,        ///< The Modulus Binary Integer Operator
    BAndOpID,           ///< The Bitwise And Binary Integer Operator
    BOrOpID,            ///< The Bitwise Or Binary Integer Operator
    BXOrOpID,           ///< The Bitwise XOr Binary Integer Operator

    // Boolean Operators
    AndOpID,            ///< The And Binary Boolean Operator
    OrOpID,             ///< The Or Binary Boolean Operator
    NorOpID,            ///< The Nor Binary Boolean Operator
    XorOpID,            ///< The Xor Binary Boolean Operator
    NotOpID,            ///< The Not Unary Boolean Operator
    LTOpID,             ///< The less-than Binary Boolean Operator
    GTOpID,             ///< The greater-than Binary Boolean Operator
    LEOpID,             ///< The less-than-or-equal Binary Boolean Operator
    GEOpID,             ///< The greather-than-or-equal Binary Boolean Operator
    EQOpID,             ///< The esual Binary Boolean Operator
    NEOpID,             ///< The not-equal Binary Comparison Operator

    // Real Arithmetic Operators
    IsPInfOpID,         ///< Real Number Positive Infinity Test Operator
    IsNInfOpID,         ///< Real Number Negative Infinity Test Operator
    IsNaNOpID,          ///< Real Number Not-A-Number Test Operator
    TruncOpID,          ///< Real Number Truncation Operator
    RoundOpID,          ///< Real Number Rounding Operator
    FloorOpID,          ///< Real Number Floor Operator
    CeilingOpID,        ///< Real Number Ceiling Operator
    PowerOpID,          ///< Real Number Power Operator
    LogEOpID,           ///< Real Number Base e (Euler's Number) logarithm 
    Log2OpID,           ///< Real Number Base 2 logarithm Operator
    Log10OpID,          ///< Real Number Base 10 logarithm Operator
    SqRootOpID,         ///< Real Number Square Root Operator
    RootOpID,           ///< Real Number Arbitrary Root Operator
    FactorialOpID,      ///< Real Number Factorial Operator
    GCDOpID,            ///< Real Number Greatest Common Divisor Operator
    LCMOpID,            ///< Real Number Least Common Multiplicator Operator
    
    // Constant Value Operators
    IntOpID,            ///< Constant Integer Value
    RealOpID,           ///< Constant Real Value
    PInfOpID,           ///< Constant Positive Infinity Real Value
    NInfOpID,           ///< Constant Negative Infinity Real Value
    NaNOpID,            ///< Constant Not-A-Number Real Value
    StringOpID,         ///< Constant String Value
    ArrayOpID,          ///< Constant Array Value
    VectorOpID,         ///< Constant Vector Value
    StructureOpID,      ///< Constant Structure Value

    // Enumeration Ranges and Limits
    NumNodeIDs,         ///< The number of type identifiers in the enum
    FirstPrimitiveTypeID = VoidTypeID,
    LastPrimitiveTypeID  = StringTypeID,
    FirstContainerTypeID = PointerTypeID,
    LastContainerTypeID  = SignatureTypeID,
    FirstOperatorID = CallOpID,
    LastOperatorID =  StructureOpID
  };

  class Type;

  /// A NamedType is simply a pair involving a name and a pointer to a Type.
  /// This is so frequently needed, it is declared here for convenience.
  typedef std::pair<std::string,Type*> NamedType;

  /// This class is the base class of HLVM Abstract Syntax Tree (AST). All 
  /// other AST nodes are subclasses of this class.
  /// @brief Abstract base class of all HLVM AST node classes
  class Node
  {
    /// @name Constructors
    /// @{
    public:
      Node(NodeIDs id, Node* parent = 0, const std::string& name = "") 
        : id_(id), parent_(parent), kids_(), name_(name) {}
      virtual ~Node();
#ifndef _NDEBUG
      virtual void dump() const;
#endif

    /// @}
    /// @name Accessors
    /// @{
    public:
      inline bool isType() const { 
        return id_ >= FirstPrimitiveTypeID && id_ <= LastContainerTypeID;
      }
      inline bool isOperator() const { 
        return id_ >= FirstOperatorID && id_ <= LastOperatorID;
      }
      inline bool isBlock() const { return id_ == BlockID; }
      inline bool isBundle() const { return id_ == BundleID; }
      inline bool isFunction() const { return id_ == FunctionID; }
      inline bool isProgram() const { return id_ == ProgramID; }
      inline bool isVariable() const { return id_ == VariableID; }
      static inline bool classof(const Node*) { return true; }

    /// @}
    /// @name Data
    /// @{
    protected:
      NodeIDs id_;              ///< Identification of the node kind.
      Node* parent_;            ///< The node that owns this node.
      std::vector<Node> kids_;  ///< The vector of children nodes.
      std::string name_;        ///< The name of this node.
    /// @}
  };
} // AST
} // hlvm
#endif
