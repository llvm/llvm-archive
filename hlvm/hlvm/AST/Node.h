//===-- Abstract Node Class -------------------------------------*- C++ -*-===//
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
#include <hlvm/AST/Locator.h>
#include <vector>

namespace hlvm
{

class Type;
class Documentation;
class AST;

/// This enumeration is used to identify the primitive types. Each of these 
/// types is a node with a NodeID but they are treated specially because of
/// their frequency of use. 
enum PrimitiveTypes {
};

/// This enumeration is used to identify a specific node. Its organization is
/// very specific and dependent on the class hierarchy. In order to use these
/// values as ranges for class identification (classof methods), we need to 
/// group things by inheritance rather than by function. 
enum NodeIDs 
{
  NoTypeID = 0,            ///< Use this for an invalid type ID.
  // Primitive Types (no child nodes)
  VoidTypeID = 1,          ///< The Void Type (The Null Type)
  BooleanTypeID,           ///< The Boolean Type (A simple on/off boolean value)
  CharacterTypeID,         ///< The Character Type (UTF-16 encoded character)
  OctetTypeID,             ///< The Octet Type (8 bits uninterpreted)
  UInt8TypeID,             ///< Unsigned 8-bit integer quantity
  UInt16TypeID,            ///< Unsigned 16-bit integer quantity
  UInt32TypeID,            ///< Unsigned 32-bit integer quantity
  UInt64TypeID,            ///< Unsigned 64-bit integer quantity
  UInt128TypeID,           ///< Unsigned 128-bit integer quantity
  SInt8TypeID,             ///< Signed 8-bit integer quantity
  SInt16TypeID,            ///< Signed 16-bit integer quantity
  SInt32TypeID,            ///< Signed 32-bit integer quantity
  SInt64TypeID,            ///< Signed 64-bit integer quantity
  SInt128TypeID,           ///< Signed 128-bit integer quantity
  Float32TypeID,           ///< 32-bit IEEE single precision 
  Float44TypeID,           ///< 43-bit IEEE extended single precision 
  Float64TypeID,           ///< 64-bit IEEE double precision
  Float80TypeID,           ///< 80-bit IEEE extended double precision
  Float128TypeID,          ///< 128-bit IEEE quad precision
  AnyTypeID,               ///< The Any Type (Union of any type)
  IntegerTypeID,           ///< The Integer Type (A # of bits of integer data)
  RangeTypeID,             ///< The Range Type (A Range of Integer Values)
  EnumerationTypeID,       ///< The Enumeration Type (set of enumerated ids)
  RealTypeID,              ///< The Real Number Type (Any Real Number)
  RationalTypeID,          ///< The Rational Number Type (p/q type number)
  StringTypeID,            ///< The String Type (Array of UTF-16 chars + length)

  // Container Types
  AliasTypeID,             ///< A new name for an existing type
  PointerTypeID,           ///< The Pointer Type (Pointer To object of Type)
  ArrayTypeID,             ///< The Array Type (Linear array of some type)
  VectorTypeID,            ///< The Vector Type (Packed Fixed Size Vector)
  StructureTypeID,         ///< The Structure Type (Sequence of various types)
  SignatureTypeID,         ///< The Function Signature Type
  ContinuationTypeID,      ///< A Continuation Type (data to continuations)
  OpaqueTypeID,            ///< A placeholder for unresolved types

  // Class Constructs (TBD)
  InterfaceID,             ///< The Interface Type (set of Signatures)
  ClassID,                 ///< The Class Type (OO Class Definition)
  MethodID,                ///< The Method Node (define a method)
  ImplementsID,            ///< Specifies set of Interfaces implemented by class

  // Linkage Items
  VariableID,              ///< The Variable Node (a storage location)
  FunctionID,              ///< The Function Node (a callable function)
  ProgramID,               ///< The Program Node (a program starting point)

  // Container
  BundleID,                ///< The Bundle Node (a group of other declarations)
  BlockID,                 ///< A Block Of Code Nodes
  ImportID,                ///< A bundle's Import declaration

  // Nilary Operators (those taking no operands)
  BreakOpID,               ///< Break out of the enclosing loop
  ConstLiteralIntegerOpID, ///< Constant Literal Integer
  ConstLiteralRealOpID,    ///< Constant Literal Real
  ConstLiteralStringOpID,  ///< Constant Literal String
  PInfOpID,                ///< Constant Positive Infinity Real Value
  NInfOpID,                ///< Constant Negative Infinity Real Value
  NaNOpID,                 ///< Constant Not-A-Number Real Value

  // Control Flow Unary Operators
  ReturnOpID,              ///< The Return A Value Operator
  ThrowOpID,               ///< The Throw an Exception Operator
  JumpToOpID,              ///< The Jump To Labelled Block Operator

  // Integer Arithmetic Unary Operators
  NotOpID,                 ///< Not Unary Boolean Operator
  NegateOpID,              ///< Negation Unary Integer Operator
  ComplementOpID,          ///< Bitwise Complement Unary Integer Operator
  PreIncrOpID,             ///< Pre-Increment Unary Integer Operator
  PostIncrOpID,            ///< Post-Increment Unary Integer Operator
  PreDecrOpID,             ///< Pre-Decrement Unary Integer Operator
  PostDecrOpID,            ///< Post-Decrement Unary Integer Operator

  // Real Arithmetic Unary Operators
  IsPInfOpID,              ///< Real Number Positive Infinity Test Operator
  IsNInfOpID,              ///< Real Number Negative Infinity Test Operator
  IsNaNOpID,               ///< Real Number Not-A-Number Test Operator
  TruncOpID,               ///< Real Number Truncation Operator
  RoundOpID,               ///< Real Number Rounding Operator
  FloorOpID,               ///< Real Number Floor Operator
  CeilingOpID,             ///< Real Number Ceiling Operator
  LogEOpID,                ///< Real Number Base e (Euler's Number) logarithm 
  Log2OpID,                ///< Real Number Base 2 logarithm Operator
  Log10OpID,               ///< Real Number Base 10 logarithm Operator
  SqRootOpID,              ///< Real Number Square Root Operator
  FactorialOpID,           ///< Real Number Factorial Operator

  // Memory Unary Operators
  LoadOpID,                ///< The Load Operator (load a value from a location)
  AllocateOpID,            ///< The Allocate Memory Operator (get heap memory)
  FreeOpID,                ///< The Free Memory Operator (free heap memory)
  StackAllocOpID,          ///< The Stack Allocation Operator (get stack mem)
  ReferenceOpID,           ///< The Reference Memory Object Operator (for GC)
  DereferenceOpID,         ///< The Dereference Memory Object Operator (for GC)

  // Other Unary Operators
  TellOpID,                ///< Tell the position of a stream
  CloseOpID,               ///< Close a stream previously opened.
  LengthOpID,              ///< Extract Length of a String Operator
  WithOpID,                ///< Scoping Operator (shorthand for a Bundle) 

  // Arithmetic Binary Operators
  AddOpID,                 ///< Addition Binary Operator
  SubtractOpID,            ///< Subtraction Binary Operator
  MultiplyOpID,            ///< Multiplcation Binary Operator
  DivideOpID,              ///< Division Binary Operator
  ModulusOpID,             ///< Modulus Binary Operator
  BAndOpID,                ///< Bitwise And Binary Operator
  BOrOpID,                 ///< Bitwise Or Binary Operator
  BXOrOpID,                ///< Bitwise XOr Binary Operator

  // Boolean Binary Operators
  AndOpID,                 ///< And Binary Boolean Operator
  OrOpID,                  ///< Or Binary Boolean Operator
  NorOpID,                 ///< Nor Binary Boolean Operator
  XorOpID,                 ///< Xor Binary Boolean Operator
  LTOpID,                  ///< <  Binary Comparison Operator
  GTOpID,                  ///< >  Binary Comparison Operator
  LEOpID,                  ///< <= Binary Comparison Operator
  GEOpID,                  ///< >= Binary Comparison Operator
  EQOpID,                  ///< == Binary Comparison Operator
  NEOpID,                  ///< != Binary Comparison Operator

  // Real Arithmetic Binary Operators
  PowerOpID,               ///< Real Number Power Operator
  RootOpID,                ///< Real Number Arbitrary Root Operator
  GCDOpID,                 ///< Real Number Greatest Common Divisor Operator
  LCMOpID,                 ///< Real Number Least Common Multiplicator Operator
  
  // Memory Binary Operators
  ReallocateOpID,          ///< The Reallocate Memory Operator 
  StoreOpID,               ///< The Store Operator (store a value to a location)

  // Other Binary Operators
  OpenOpID,                ///< Open a stream from a URL
  ReadOpID,                ///< Read from a stream
  WriteOpID,               ///< Write to a stream
  CreateContOpID,          ///< The Create Continutation Operator

  // Ternary Operators
  IfOpID,                  ///< The If-Then-Else Operator
  StrInsertOpID,           ///< Insert(str,where,what)
  StrEraseOpID,            ///< Erase(str,at,len)
  StrReplaceOpID,          ///< Replace(str,at,len,what)
  PositionOpID,            ///< Position a stream (stream,where,relative-to)

  // Multi Operators
  CallOpID,                ///< The Call Operator (n operands)
  InvokeOpID,              ///< The Invoke Operator (n operands)
  DispatchOpID,            ///< The Object Method Dispatch Operator (n operands)
  CallWithContOpID,        ///< The Call with Continuation Operator (n operands)
  SelectOpID,              ///< The Select An Alternate Operator (n operands)
  LoopOpID,                ///< The General Purpose Loop Operator (5 operands)

  // Miscellaneous Nodes
  DocumentationID,         ///< XHTML Documentation Node
  TreeTopID,               ///< The AST node which is always root of the tree

  // Enumeration Ranges and Limits
  NumNodeIDs,              ///< The number of node identifiers in the enum
  FirstPrimitiveTypeID = VoidTypeID, ///< First Primitive Type
  LastPrimitiveTypeID  = Float128TypeID, ///< Last Primitive Type
  FirstSimpleTypeID    = AnyTypeID,
  LastSimpleTypeID     = StringTypeID,
  FirstContainerTypeID = PointerTypeID, ///< First Container Type
  LastContainerTypeID  = ContinuationTypeID, ///< Last Container Type
  FirstTypeID          = VoidTypeID,
  LastTypeID           = OpaqueTypeID,
  FirstOperatorID      = BreakOpID, ///< First Operator
  LastOperatorID       = LoopOpID,  ///< Last Operator
  FirstNilaryOpID      = BreakOpID,
  LastNilaryOpID       = NaNOpID,
  FirstUnaryOpID       = ReturnOpID,
  LastUnaryOpID        = WithOpID,
  FirstBinaryOpID      = AddOpID,
  LastBinaryOpID       = CreateContOpID,
  FirstTernaryOpID     = IfOpID,
  LastTernaryOpID      = PositionOpID,
  FirstMultiOpID       = CallOpID,
  LastMultiOpID        = LoopOpID
};

class ParentNode;

/// This class is the base class of HLVM Abstract Syntax Tree (AST). All 
/// other AST nodes are subclasses of this class.
/// @brief Abstract base class of all HLVM AST node classes
class Node
{
  /// @name Constructors
  /// @{
  protected:
    Node(NodeIDs ID) : id(ID), parent(0), loc() {}
  public:
    virtual ~Node();

  /// @}
  /// @name Accessors
  /// @{
  public:
    inline AST* getRoot(); 

    /// Get the type of node
    inline NodeIDs getID() const { return NodeIDs(id); }

    /// Get the parent node
    inline Node* getParent() const { return parent; }

    /// Get the flags
    inline unsigned getFlags() const { return flags; }

    /// Get the Locator
    inline const Locator& getLocator() const { return loc; }

    /// Determine if the node is a specific kind
    inline bool is(NodeIDs kind) const { return id == unsigned(kind); }

    /// Determine if the node is a Type
    inline bool isType() const {
      return id >= FirstTypeID && id <= LastTypeID;
    }

    inline bool isIntegralType()  const { 
      return (id >= UInt8TypeID && id <= UInt128TypeID) ||
             (id == IntegerTypeID) || (id == RangeTypeID) || 
             (id == EnumerationTypeID);
    }

    /// Determine if the node is a primitive type
    inline bool isPrimitiveType() const {
      return id >= FirstPrimitiveTypeID && id <= LastPrimitiveTypeID;
    }

    /// Determine if the node is a simple type
    inline bool isSimpleType() const {
      return id >= FirstSimpleTypeID && id <= LastSimpleTypeID;
    }

    /// Determine if the node is a container type
    inline bool isContainerType() const {
      return id >= FirstContainerTypeID && id <= LastContainerTypeID;
    }

    inline bool isAnyType() const { return id == AnyTypeID; }
    inline bool isBooleanType() const { return id == BooleanTypeID; }
    inline bool isCharacterType() const { return id == CharacterTypeID; }
    inline bool isOctetType() const { return id == OctetTypeID; }
    inline bool isIntegerType() const { return id == IntegerTypeID; }
    inline bool isRangeType() const { return id == RangeTypeID; }
    inline bool isRealType() const { return id == RealTypeID; }
    inline bool isRationalType() const { return id == RationalTypeID; }
    inline bool isPointerType() const { return id == PointerTypeID; }
    inline bool isArrayType() const { return id == ArrayTypeID; }
    inline bool isVectorType() const { return id == VectorTypeID; }
    inline bool isStructureType() const { return id == StructureTypeID; }
    inline bool isSignatureType() const { return id == SignatureTypeID; }
    inline bool isVoidType() const { return id == VoidTypeID; }

    /// Determine if the node is any of the Operators
    inline bool isOperator() const { 
      return id >= FirstOperatorID && id <= LastOperatorID;
    }
    inline bool isNilaryOperator() const {
      return id >= FirstNilaryOpID && id <= LastNilaryOpID;
    }
    inline bool isUnaryOperator() const {
      return id >= FirstUnaryOpID && id <= LastUnaryOpID;
    }
    inline bool isBinaryOperator() const {
      return id >= FirstBinaryOpID && id <= LastBinaryOpID;
    }
    inline bool isTernaryOperator() const {
      return id >= FirstTernaryOpID && id <= LastTernaryOpID;
    }
    inline bool isMultiOperator() const {
      return id >= FirstMultiOpID && id <= LastMultiOpID;
    }

    /// Determine if the node is a Documentable Node
    bool isDocumentable() const { return isValue() || isType() || isBundle();}

    /// Determine if the node is a LinkageItem
    bool isLinkageItem() const {
      return isFunction() || isVariable();
    }

    /// Determine if the node is a Value
    bool isValue() const { return isLinkageItem() || isOperator(); }

    /// Determine if the node is a Block
    inline bool isBlock() const { return id == BlockID; }
    /// Determine if the node is a Bundle
    inline bool isBundle() const { return id == BundleID; }
    /// Determine if the node is a Function
    inline bool isFunction() const 
      { return id == FunctionID || id == ProgramID;}
    /// Determine if the node is a Program
    inline bool isProgram() const { return id == ProgramID; }
    /// Determine if the node is a Variable
    inline bool isVariable() const { return id == VariableID; }

    /// Provide support for isa<X> and friends
    static inline bool classof(const Node*) { return true; }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setLocator(const Locator& l) { loc = l; }
    void setFlags(unsigned f); 
    virtual void setParent(Node* parent);

  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);
  /// @}
  /// @name Utilities
  /// @{
  public:
#ifndef _NDEBUG
    virtual void dump() const;
#endif

  /// @}
  /// @name Data
  /// @{
  protected:
    unsigned id : 8;         ///< Really a value in NodeIDs
    unsigned flags : 24;     ///< 24 boolean flags, subclass dependent interp.
    Node* parent;            ///< The node that owns this node.
    Locator loc;             ///< The source location corresponding to node.
  /// @}
  friend class AST;
};

class Documentable : public Node
{
  /// @name Constructors
  /// @{
  protected:
    Documentable(NodeIDs id) : Node(id), doc(0) {}
  public:
    virtual ~Documentable();

  /// @}
  /// @name Accessors
  /// @{
  public:
    /// Get the name of the node
    inline Documentation* getDoc() { return doc; }

    static inline bool classof(const Documentable*) { return true; }
    static inline bool classof(const Node* N) { return N->isDocumentable(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setDoc(Documentation* d) { doc = d; }

  /// @}
  /// @name Data
  /// @{
  protected:
    Documentation* doc;///< All named nodes can have documentation
  /// @}
  friend class AST;
};

class Value : public Documentable
{
  /// @name Constructors
  /// @{
  protected:
    Value(NodeIDs id) : Documentable(id), type(0)  {}
  public:
    virtual ~Value();

  /// @}
  /// @name Accessors
  /// @{
  public:
    // Get the type of the value
    inline const Type* getType() const { return type; }

    static inline bool classof(const Value*) { return true; }
    static inline bool classof(const Node* N) { return N->isValue(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setType(const Type* t) { type = t; }

  /// @}
  /// @name Data
  /// @{
  protected:
    const Type* type; ///< The type of this node.
  /// @}
  friend class AST;
};

} // hlvm
#endif
