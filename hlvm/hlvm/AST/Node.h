//===-- Abstract Node Class Interface ---------------------------*- C++ -*-===//
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
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Node and basic subclasses
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

/// This enumeration is used to identify the various kinds of Abstract Syntax
/// Tre nodes. Its organization is very specific and dependent on the class 
/// hierarchy. In order to use these values as ranges for class identification 
/// (classof methods), we need to group things by inheritance rather than by 
/// function. Items beginning with "First" or "Last" identify a useful range
/// of node types and do not introduce any value of their own.
/// @brief Identifiers of th AST Node Types.
enum NodeIDs 
{
  NoTypeID = 0,            ///< Use this for an invalid type ID.

  // SUBCLASSES OF NODE
  TreeTopID,               ///< The AST node which is always root of the tree
FirstNodeID = TreeTopID,
  DocumentationID,         ///< XHTML Documentation Node
  DocumentableID,          ///< A node that can have a Documentation Node
FirstDocumentableID = DocumentableID,
  BundleID,                ///< The Bundle Node (a group of other declarations)
  ImportID,                ///< A bundle's Import declaration

  // SUBCLASSES OF TYPE
  // Primitive Types (inherently supported by HLVM)
  VoidTypeID,              ///< The Void Type (The Null Type)
FirstTypeID          = VoidTypeID,
FirstPrimitiveTypeID = VoidTypeID,
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
LastPrimitiveTypeID  = Float128TypeID, 

  // Simple Types (no nested classes)
  AnyTypeID,               ///< The Any Type (Union of any type)
FirstSimpleTypeID    = AnyTypeID,
  IntegerTypeID,           ///< The Integer Type (A # of bits of integer data)
  RangeTypeID,             ///< The Range Type (A Range of Integer Values)
  EnumerationTypeID,       ///< The Enumeration Type (set of enumerated ids)
  RealTypeID,              ///< The Real Number Type (Any Real Number)
  RationalTypeID,          ///< The Rational Number Type (p/q type number)
  OpaqueTypeID,            ///< A placeholder for unresolved types
LastSimpleTypeID     = OpaqueTypeID,  

  // Uniform Container Types
  AliasTypeID,             ///< A new name for an existing type
FirstContainerTypeID = AliasTypeID,
FirstUniformContainerTypeID = AliasTypeID,
  PointerTypeID,           ///< The Pointer Type (Pointer To object of Type)
  ArrayTypeID,             ///< The Array Type (Linear array of some type)
  VectorTypeID,            ///< The Vector Type (Packed Fixed Size Vector)
LastUniformContainerTypeID = VectorTypeID,

  // Disparate Container Types
  StructureTypeID,         ///< The Structure Type (Sequence of various types)
FirstDisparateContainerTypeID = StructureTypeID,
  SignatureTypeID,         ///< The Function Signature Type
  ContinuationTypeID,      ///< A Continuation Type (data to continuations)
LastDisparateContainerTypeID  = ContinuationTypeID,
LastContainerTypeID = ContinuationTypeID,

  // Runtime Types
  BufferTypeID,            ///< A buffer of octets for I/O
FirstRuntimeTypeID = BufferTypeID,
  StreamTypeID,            ///< A stream handle
  TextTypeID,              ///< A UTF-8 or UTF-16 encoded text string
LastRuntimeTypeID = TextTypeID,
LastTypeID = TextTypeID,

  // Class Constructs (TBD)
  InterfaceID,             ///< The Interface Type (set of Signatures)
  ClassID,                 ///< The Class Type (OO Class Definition)
  MethodID,                ///< The Method Node (define a method)
  ImplementsID,            ///< Specifies set of Interfaces implemented by class

  // SUBCLASSES OF VALUE

  // Constants
  ConstantZeroID,          ///< A zero-filled constant of any type
FirstValueID = ConstantZeroID,
FirstConstantID = ConstantZeroID,
  ConstantBooleanID,       ///< A constant boolean value
  ConstantIntegerID,       ///< A constant integer value
  ConstantRealID,          ///< A constant real value
  ConstantTextID,          ///< A constant text value
  ConstantAggregateID,     ///< A constant aggregate for arrays, structures, etc
  ConstantExpressionID,    ///< A constant expression
  SizeOfID,                ///< Size of a type

  // Linkage Items
  VariableID,              ///< The Variable Node (a storage location)
FirstLinkageItemID = VariableID,
  FunctionID,              ///< The Function Node (a callable function)
  ProgramID,               ///< The Program Node (a program starting point)
LastLinkageItemID = ProgramID,
LastConstantID = ProgramID,

  // Operator
  BlockID,                 ///< A Block Of Operators
FirstOperatorID = BlockID,

  // Nilary Operators (those taking no operands)
  BreakOpID,               ///< Break out of the enclosing loop
FirstNilaryOperatorID = BreakOpID,
  ContinueOpID,            ///< Continue from start of enclosing block
  PInfOpID,                ///< Constant Positive Infinity Real Value
  NInfOpID,                ///< Constant Negative Infinity Real Value
  NaNOpID,                 ///< Constant Not-A-Number Real Value
  ReferenceOpID,           ///< Obtain pointer to local/global variable
LastNilaryOperatorID = ReferenceOpID,

  // Control Flow Unary Operators
  NoOperatorID,            ///< The "do nothing" NoOp Operators
FirstUnaryOperatorID = NoOperatorID,
  ReturnOpID,              ///< The Return A Value Operator
  ThrowOpID,               ///< The Throw an Exception Operator

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
  IsNanOpID,               ///< Real Number Not-A-Number Test Operator
  TruncOpID,               ///< Real Number Truncation Operator
  RoundOpID,               ///< Real Number Rounding Operator
  FloorOpID,               ///< Real Number Floor Operator
  CeilingOpID,             ///< Real Number Ceiling Operator
  LogEOpID,                ///< Real Number Base e (Euler's Number) logarithm 
  Log2OpID,                ///< Real Number Base 2 logarithm Operator
  Log10OpID,               ///< Real Number Base 10 logarithm Operator
  SquareRootOpID,          ///< Real Number Square Root Operator
  CubeRootOpID,            ///< Real Number Cube Root Operator
  FactorialOpID,           ///< Real Number Factorial Operator

  // Memory Unary Operators
  LoadOpID,                ///< The Load Operator (load a value from a location)
  AllocateOpID,            ///< The Allocate Memory Operator 
  DeallocateOpID,          ///< The Deallocate Memory Operator
  AutoVarOpID,             ///< Declaration of an automatic (stack) variable

  // Input/Output Unary Operators
  TellOpID,                ///< Tell the position of a stream
  CloseOpID,               ///< Close a stream previously opened.

  // Other Unary Operators
  LengthOpID,              ///< Extract Length of a Text/Array/Vector
LastUnaryOperatorID = LengthOpID,

  // Arithmetic Binary Operators
  AddOpID,                 ///< Addition Binary Operator
FirstBinaryOperatorID = AddOpID,
  SubtractOpID,            ///< Subtraction Binary Operator
  MultiplyOpID,            ///< Multiplcation Binary Operator
  DivideOpID,              ///< Division Binary Operator
  ModuloOpID,              ///< Modulus Binary Operator
  BAndOpID,                ///< Bitwise And Binary Operator
  BOrOpID,                 ///< Bitwise Or Binary Operator
  BXorOpID,                ///< Bitwise XOr Binary Operator
  BNorOpID,                ///< Bitwise Nor Binary Operator

  // Boolean Binary Operators
  AndOpID,                 ///< And Binary Boolean Operator
  OrOpID,                  ///< Or Binary Boolean Operator
  NorOpID,                 ///< Nor Binary Boolean Operator
  XorOpID,                 ///< Xor Binary Boolean Operator
  LessThanOpID,            ///< <  Binary Comparison Operator
  GreaterThanOpID,         ///< >  Binary Comparison Operator
  LessEqualOpID,           ///< <= Binary Comparison Operator
  GreaterEqualOpID,        ///< >= Binary Comparison Operator
  EqualityOpID,            ///< == Binary Comparison Operator
  InequalityOpID,          ///< != Binary Comparison Operator

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
LastBinaryOperatorID = CreateContOpID,

  // Ternary Operators
  SelectOpID,                  ///< The select an alternate operator
FirstTernaryOperatorID = SelectOpID,
  StrInsertOpID,           ///< Insert(str,where,what)
  StrEraseOpID,            ///< Erase(str,at,len)
  StrReplaceOpID,          ///< Replace(str,at,len,what)
  PositionOpID,            ///< Position a stream (stream,where,relative-to)
  LoopOpID,                ///< The General Purpose Loop Operator
LastTernaryOperatorID = LoopOpID,

  // Multi Operators
  CallOpID,                ///< The Call Operator (n operands)
FirstMultiOperatorID = CallOpID,
  InvokeOpID,              ///< The Invoke Operator (n operands)
  DispatchOpID,            ///< The Object Method Dispatch Operator (n operands)
  CallWithContOpID,        ///< The Call with Continuation Operator (n operands)
  IndexOpID,               ///< The Index Operator for indexing an array
  SwitchOpID,              ///< The Switch Operator (n operands)
LastMultiOperatorID = SwitchOpID,
LastOperatorID = SwitchOpID,
LastValueID = SwitchOpID,
LastDocumentableID = SwitchOpID,
LastNodeID = SwitchOpID,

  NumNodeIDs               ///< The number of node identifiers in the enum
};

/// This class is the abstract base class of all the Abstract Syntax Tree (AST)
/// node types. All other AST nodes are subclasses of this class. This class
/// must only provide functionality that is common to all AST Node subclasses.
/// It provides for class identification, insertion of nodes, management of a
/// set of flags, 
/// @brief Abstract Base Class of All HLVM AST Nodes
class Node
{
  /// @name Constructors
  /// @{
  protected:
    Node(NodeIDs ID) : id(ID), flags(0), parent(0), loc(0) {}
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
    inline const Locator* getLocator() const { return loc; }

    /// Determine if the node is a specific kind
    inline bool is(NodeIDs kind) const { return id == unsigned(kind); }

    /// Determine if the node is a Type
    inline bool isType() const {
      return id >= FirstTypeID && 
             id <= LastTypeID; }

    inline bool isIntegerType() const {
      return (id >= UInt8TypeID && id <= SInt128TypeID) || id == IntegerTypeID;}

    inline bool isIntegralType()  const { 
      return isIntegerType() || (id == RangeTypeID) || 
        (id == EnumerationTypeID) || (id == BooleanTypeID);  }

    inline bool isRealType() const {
      return (id >= Float32TypeID && id <= Float128TypeID) ||
             (id == RealTypeID); }

    /// Determine if the node is a primitive type
    inline bool isPrimitiveType() const {
      return id >= FirstPrimitiveTypeID && 
             id <= LastPrimitiveTypeID; }

    /// Determine if the node is a simple type
    inline bool isSimpleType() const {
      return id >= FirstSimpleTypeID && 
             id <= LastSimpleTypeID;
    }

    /// Determine if the node is a uniform container type
    inline bool isUniformContainerType() const {
      return id >= FirstUniformContainerTypeID && 
             id <= LastUniformContainerTypeID;
    }
    /// Determine if the node is a disparate container type
    inline bool isDisparateContainerType() const {
      return id >= FirstDisparateContainerTypeID && 
             id <= LastDisparateContainerTypeID; }

    /// Determine if the node is a runtime type
    inline bool isRuntimeType() const {
      return id >= FirstRuntimeTypeID &&
             id <= LastRuntimeTypeID; }

    /// Determine if the node is a container type
    inline bool isContainerType() const {
      return id >= FirstContainerTypeID && id <= LastContainerTypeID;
    }
    /// Determine if the node is one of the Constant values.
    inline bool isConstant() const {
      return id >= FirstConstantID && id <= LastConstantID; }

    /// Determine if the node is a LinkageItem
    inline bool isLinkageItem() const {
      return id >= FirstLinkageItemID && id <= LastLinkageItemID; }

    /// Determine if the node is any of the Operators
    inline bool isOperator() const { 
      return id >= FirstOperatorID && id <= LastOperatorID; }

    inline bool isNilaryOperator() const {
      return id >= FirstNilaryOperatorID && id <= LastNilaryOperatorID; }

    inline bool isUnaryOperator() const {
      return id >= FirstUnaryOperatorID && id <= LastUnaryOperatorID; }

    inline bool isBinaryOperator() const {
      return id >= FirstBinaryOperatorID && id <= LastBinaryOperatorID; }

    inline bool isTernaryOperator() const {
      return id >= FirstTernaryOperatorID && id <= LastTernaryOperatorID; }

    inline bool isMultiOperator() const {
      return id >= FirstMultiOperatorID && id <= LastMultiOperatorID; }

    /// Determine if the node is a Documentable Node
    bool isDocumentable() const { 
      return id >= FirstDocumentableID && id <= LastDocumentableID; }

    /// Determine if the node is a Value
    bool isValue() const { 
      return id >= FirstValueID && id <= LastValueID; }

    inline bool isFunction() const 
      { return id == FunctionID || id == ProgramID; }

    /// Provide support for isa<X> and friends
    static inline bool classof(const Node*) { return true; }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setLocator(const Locator* l) { loc = l; }
    void setFlags(uint16_t f) { flags = f; }
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
    uint16_t id;         ///< Really a value in NodeIDs
    uint16_t flags;      ///< 16 flags, subclass dependent interpretation
    Node* parent;        ///< The node that owns this node.
    const Locator* loc;  ///< The source location corresponding to node.
  /// @}
  friend class AST;
};

/// This class is an abstract base class in the Abstract Syntax Tree for any 
/// node type that can be documented. That is, it provides a facility for
/// attaching a Documentation node. This is the base class of most definitions
/// in the AST. 
/// @see Documentation
/// @brief AST Documentable Node
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

/// This class is an abstract base class in the Abstract Syntax Tree for things
/// that have a value at runtime. Every Value has a Type. All Operators, 
/// LinkageItems, and Constants are values.
/// @see Type
/// @see LinkageItem
/// @see Operator
/// @see Constant
/// @brief AST Value Node
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
