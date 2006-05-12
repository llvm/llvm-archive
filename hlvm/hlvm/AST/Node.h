//===-- hlvm/AST/Node.h - AST Abstract Node Class ---------------*- C++ -*-===//
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
/// @file hlvm/AST/Node.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Node
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_NODE_H
#define HLVM_AST_NODE_H

#include <llvm/Support/Casting.h>
#include <hlvm/AST/Location.h>
#include <vector>

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
    // Primitive Types
    VoidTypeID = 0,     ///< The Void Type (The Null Type)
    AnyTypeID,          ///< The Any Type (Union of any type)
    BooleanTypeID,      ///< The Boolean Type (A simple on/off boolean value)
    CharacterTypeID,    ///< The Character Type (UTF-16 encoded character)
    OctetTypeID,        ///< The Octet Type (8 bits uninterpreted)
    IntegerTypeID,      ///< The Integer Type (A number of bits of integer data)
    RangeTypeID,        ///< The Range Type (A Range of Integer Values)
    RealTypeID,         ///< The Real Number Type (Any Real Number)
    RationalTypeID,     ///< The Rational Number Type (p/q type number)
    StringTypeID,       ///< The String Type (Array of UTF-16 chars + length)

    // Container Types
    PointerTypeID,      ///< The Pointer Type (Pointer To storage of other Type)
    ArrayTypeID,        ///< The Array Type (Linear array of some type)
    VectorTypeID,       ///< The Vector Type (Packed Fixed Size Vector)
    StructureTypeID,    ///< The Structure Type (Sequence of various types)
    FieldID,            ///< Declare name and type of Structure Field
    SignatureTypeID,    ///< The Function Signature Type
    ArgumentID,         ///< Declare name and type of Signature Argument
    ContinuationTypeID, ///< A Continuation Type (data passing to continuations)

    // Class Constructs (TBD)
    InterfaceID,        ///< The Interface Type (set of Signatures)
    ClassID,            ///< The Class Type (Object Oriented Class Definition)
    MethodID,           ///< The Method Node (define a method)
    ImplementsID,       ///< Specifies set of Interfaces implemented by class

    // Linkage Items
    VariableID,         ///< The Variable Node (a storage location)
    FunctionID,         ///< The Function Node (a callable function)
    ProgramID,          ///< The Program Node (a

    // Container
    BundleID,           ///< The Bundle Node (a group of other declarations)
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

    // Scoping Operators
    WithOpID,           ///< Create a shorthand for a Bundle (like using/import)

    // Memory Operators
    LoadOpID,           ///< The Load Operator (load a value from a location)
    StoreOpID,          ///< The Store Operator (store a value to a location)
    AllocateOpID,       ///< The Allocate Memory Operator (get some heap memory)
    FreeOpID,           ///< The Free Memory Operator (free some heap memory)
    ReallocateOpID,     ///< The Reallocate Memory Operator (realloc heap mem)
    StackAllocOpID,     ///< The Stack Allocation Operator (get some stack mem)
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
    
    // Character And String Operators
    MungeOpID,          ///< General Purpose String Editing Operator
    LengthOpID,         ///< Extract Length of a String Operator

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

    // Input/Output Operators
    MapFileOpID,        ///< Map a file to memory (mmap)
    OpenOpID,           ///< Open a stream from a URL
    CloseOpID,          ///< Close a stream previously opened.
    ReadOpID,           ///< Read from a stream
    WriteOpID,          ///< Write to a stream
    PositionOpID,       ///< Position a stream

    // Enumeration Ranges and Limits
    NumNodeIDs,         ///< The number of node identifiers in the enum
    FirstPrimitiveTypeID = VoidTypeID, ///< First Primitive Type
    LastPrimitiveTypeID  = StringTypeID, ///< Last Primitive Type
    FirstContainerTypeID = PointerTypeID, ///< First Container Type
    LastContainerTypeID  = ContinuationTypeID, ///< Last Container Type
    FirstOperatorID = CallOpID, ///< First Operator
    LastOperatorID =  StructureOpID ///< Last Operator
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
      Location loc_;            ///< The source location corresponding to node.
    /// @}
  };
} // AST
} // hlvm
#endif
