//===-- AST Container Class -------------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/AST.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::AST
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_AST_H
#define HLVM_AST_AST_H

#include <hlvm/AST/Node.h>
#include <hlvm/AST/Type.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/RuntimeType.h>
#include <hlvm/AST/Constant.h>
#include <string>
#include <vector>

/// This namespace is for all HLVM software. It ensures that HLVM software does
/// not collide with any other software. Hopefully "hlvm" is not a namespace 
/// used elsewhere.
namespace hlvm
{

class Bundle;   
class Documentation;
class Block;
class Function; 
class Program; 
class Import;
class Locator; 
class Variable; 
class ConstantInteger;
class ConstantReal;
class ConstantText;
class ConstantZero;
class Pool;
class ReturnOp;
class StoreOp;
class LoadOp;
class OpenOp;
class CloseOp;
class WriteOp;

/// This class is used to hold or contain an Abstract Syntax Tree. It forms the
/// root node of a multi-way tree of other nodes. As such, its parent node is
/// null and this is only true of the AST node.  AST provides a number of 
/// facilities for management of the tree as a whole. It also provides all the 
/// factory functions for creating AST nodes.  
/// @brief AST Tree Root Class
class AST : public Node
{
  /// @name Types
  /// @{
  public:
    typedef std::vector<Bundle*> BundleList;
    typedef BundleList::iterator   iterator;
    typedef BundleList::const_iterator const_iterator;

  /// @}
  /// @name Constructors
  /// @{
  public:
    static AST* create();
    static void destroy(AST* ast);

  protected:
    AST() : Node(TreeTopID), sysid(), pubid(), bundles(), pool(0) {}
    ~AST();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string& getSystemID() const { return sysid; }
    const std::string& getPublicID() const { return pubid; }
    Pool* getPool() const { return pool; }
    SignatureType* getProgramType() const;

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setSystemID(const std::string& id) { sysid = id; }
    void setPublicID(const std::string& id) { pubid = id; }
    void addBundle(Bundle* b) { bundles.push_back(b); }

  /// @}
  /// @name Lookup
  /// @{
  public:
    Type* resolveType(const std::string& name);

  /// @}
  /// @name Iterators
  /// @{
  public:
    iterator           begin()       { return bundles.begin(); }
    const_iterator     begin() const { return bundles.begin(); }
    iterator           end  ()       { return bundles.end(); }
    const_iterator     end  () const { return bundles.end(); }
    size_t             size () const { return bundles.size(); }
    bool               empty() const { return bundles.empty(); }
    Bundle*            front()       { return bundles.front(); }
    const Bundle*      front() const { return bundles.front(); }
    Bundle*            back()        { return bundles.back(); }
    const Bundle*      back()  const { return bundles.back(); }

  /// @}
  /// @name Factories
  /// @{
  public:
    /// Get one of the primitive types directly by its identifier
    Type* getPrimitiveType(NodeIDs kind);

    /// Create a new Locator object. Locators indicate where in the source
    /// a particular AST node is located. Locators can be very general (just
    /// the URI) or very specific (the exact range of bytes in the file). The
    /// Locator returned can be used with any of the other factory methods 
    /// in this class.
    Locator* new_Locator(
      const URI* uri,        ///< The URI of the source
      uint32_t line = 0,     ///< The line number of the location
      uint32_t col = 0,      ///< The column number of the location
      uint32_t line2 =0,     ///< The ending line number of the location range
      uint32_t col2 = 0      ///< The ending column number of the location range
    );

    /// Create a new Documentation node. A documentation node contains the
    /// documentation that accompanies the program. 
    Documentation* new_Documentation(
      const Locator* loc = 0 ///< The source locator
    );

    /// Create a new Bundle node. A bundle is the general container of other AST
    /// nodes. Bundles are also the unit of loading and linking.
    Bundle* new_Bundle(
      const std::string& id, ///< The name of the bundle
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a new Import node. An import node may be attached to a bundle to
    /// indicate that the declarations of some external bundle are needed in
    /// order to satisfy the definitions of the current bundle.
    Import* new_Import(
      const std::string& id,  ///< The name of the import
      const Locator* loc = 0 ///< 
    );
    /// Create a new Function node. 
    Function* new_Function(
      const std::string& id, ///< The name of the function
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a new Argument node. Arguments are used as the formal argument
    /// value to a function.
    Argument* new_Argument(
      const std::string& id, ///< The name of the function argument
      Type* ty,              ///< The type of the function argument 
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a new Program node. Programs are like functions except that their
    /// signature is fixed and they represent the entry point to a complete
    /// program. Unlike other languages, you can have multiple Program nodes
    /// (entry points) in the same compilation unit.
    Program* new_Program(
      const std::string& id, ///< The name of the program
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a new Block node. A block is a sequence of operators.
    Block* new_Block(
      const std::string& id, ///< The name (label) of the block
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a new IntegerType node. This is a general interface for creating
    /// integer types. By default it creates a signed 32-bit integer.
    IntegerType* new_IntegerType(
      const std::string& id,  ///< The name of the type
      uint64_t bits = 32,     ///< The number of bits
      bool isSigned = true,    ///< The signedness
      const Locator* loc = 0  ///< The locator of the declaration
    );
    /// Create a new RangeType node. RangeType nodes are integer nodes that
    /// perform range checking to ensure the assigned values are kept in range
    RangeType* new_RangeType(
      const std::string& id,  ///< The name of the type
      int64_t min,            ///< The minimum value accepted in range
      int64_t max,            ///< The maximum value accepted in range
      const Locator*loc = 0   ///< The locator of the declaration
    );
    /// Create a new EnumerationType node. EnumerationType nodes are RangeType
    /// nodes that associate specific enumerated names for the values of the
    /// enumeration.
    EnumerationType* new_EnumerationType(
      const std::string& id,   ///< The name of the type
      const Locator*loc = 0    ///< The locator of the declaration
    );
    /// Create a new RealType node. This is the generalized interface for 
    /// construction real number types. By default it creates a 64-bit double
    /// precision floating point type.
    RealType* new_RealType(
      const std::string& id,  ///< The name of the type
      uint32_t mantissa = 52, ///< The bits in the mantissa (fraction)
      uint32_t exponent = 11, ///< The bits in the exponent
      const Locator* loc = 0   ///< The locator 
    );
    /// Create a new AnyType node. An AnyType node is a type that can hold a
    /// value of any other HLVM type. 
    AnyType* new_AnyType(
      const std::string& id, ///< The name of the type
      const Locator* loc = 0  ///< The source locator 
    );
    /// Create a new BooleanType node. A BooleanType has a simple binary value,
    /// true or false.
    BooleanType* new_BooleanType(
      const std::string& id,  ///< The name of the type
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new BufferType node. A BufferType is a runtime type that is
    /// used to buffer input and output.
    BufferType* new_BufferType(
      const std::string& id,  ///< The name of the type
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new TextType node. A TextType is a runtime type that is
    /// used to represent unicode strings of text.
    TextType* new_TextType(
      const std::string& id,  ///< The name of the type
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new StreamType node. A StreamType is a runtime type that is
    /// used as a handle for input/output streams.
    StreamType* new_StreamType(
      const std::string& id,  ///< The name of the type
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new CharacterType node. A CharacterType represents a single 
    /// textual character in UTF-16 encoding.
    CharacterType* new_CharacterType(
      const std::string& id,  ///< The name of the type
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new OctetType node. An OctetType represents an 8-bit 
    /// non-numerical quantity. You can't do arithmetic with octets.
    OctetType* new_OctetType(
      const std::string& id,  ///< The name of the type
      const Locator*loc = 0   ///< The source locator
    );
    /// Create a new VoidType node. A VoidType represents a zero-length entity
    /// that has no address. In other words, its nothing, zilch, nada, empty,
    /// vacuum, etc. As you there's lots of ways to name the concept of 
    /// nothing and that's why this method exists.
    VoidType* new_VoidType(
      const std::string& id,  ///< The name of the type
      const Locator* loc =0   ///< The source locator     
    );
    /// Create a new PointerType node. A PointerType just refers to a location
    /// of some other type.
    PointerType* new_PointerType(
      const std::string& id,  ///< The name of the pointer type
      Type* target,           ///< The referent type
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new ArrayType node. An ArrayType is a sequential arrangement of
    /// memory locations of uniform type. Arrays can be dynamically expanded
    /// or shrunk, but not beyond the maxSize parameter. 
    ArrayType* new_ArrayType(
      const std::string& id,  ///< The name of the array type
      Type* elemType,         ///< The element type
      uint64_t maxSize,       ///< The maximum number of elements in the array
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new VectorType node. A VectorType is a sequential arrangement
    /// of memory locations of uniform type and constant size. Unlike Arrays,
    /// a vector's size is always constant.
    VectorType* new_VectorType(
      const std::string& id,  ///< The name of the vector type
      Type* elemType,         ///< The element type
      uint64_t size,          ///< The number of elements in the vector
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new AliasType node. An AliasType node is simply a way of giving
    /// a new name to another type. Since type naming equates to type
    /// equivalence in HLVM, this node allows two different types to be
    /// equivalent without having the same name.
    AliasType* new_AliasType(
      const std::string& id,  ///< The name of the alias
      Type* referrant,        ///< The type for which this type is an alias
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new StructureType node. A StructureType is a type that is an
    /// ordered sequential arrangement of memory locations of various but 
    /// definite types.
    StructureType* new_StructureType(
      const std::string& id,  ///< The name of the structure type
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new SignatureType node. A SignatureType specifies the type of
    /// a Function. It identifies the names and types of the arguments of a
    /// function and the type of its result value.
    SignatureType* new_SignatureType(
      const std::string& id,  ///< The name of the function signature type
      const Type *resultType, ///< The result type of the function
      bool isVarArgs = false, ///< Indicates variable number of arguments
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new OpaqueType node. An OpaqueType is used as a place holder
    /// for situations where the full type is either not known or should not
    /// be exposed. You cannot create an object of OpaqueType but you can 
    /// obtain its location.
    OpaqueType* new_OpaqueType(
      const std::string& id, ///< The name of the opaque type
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a new 128 bit primitive floating point type.
    RealType* new_f128(
      const std::string& id,  ///< The name of the 128-bit floating point type
      const Locator* loc = 0  ///< The source locator
    ) { return new_RealType(id,112,15,loc); }
    /// Create a new 80 bit primitive floating point type.
    RealType* new_f80(
      const std::string& id,  ///< The name of the 80-bit floating point type
      const Locator* loc = 0  ///< The source locator
    ) { return new_RealType(id,64,15,loc); }
    /// Create a new 64 bit primitive floating point type.
    RealType* new_f64(
      const std::string& id,  ///< The name of the 64-bit floating point type
      const Locator* loc = 0  ///< The source locator
    ) { return new_RealType(id,52,11,loc); }
    /// Create a new 44 bit primitive floating point type.
    RealType* new_f44(
      const std::string& id,  ///< The name of the 44-bit floating point type
      const Locator* loc = 0  ///< The source locator
    ) { return new_RealType(id,32,11,loc); }
    /// Create a new 32 bit primitive floating point type.
    RealType* new_f32(
      const std::string& id,  ///< The name of teh 32-bit floating point type
      const Locator* loc = 0  ///< The source locator
    ) { return new_RealType(id,23,8,loc); }
    /// Create a new 128 bit primitive signed integer point type.
    IntegerType* new_s128(
      const std::string& id,  ///< The name of the 128-bit signed integer type
      const Locator* loc = 0  ///< The source locator
    ) { return new_IntegerType(id,128,true,loc); }
    /// Create a new 64 bit primitive signed integer point type.
    IntegerType* new_s64(
      const std::string& id,  ///< The name of the 64-bit signed integer type
      const Locator* loc = 0  ///< The source locator
    ) { return new_IntegerType(id,64,true,loc); }
    /// Create a new 32 bit primitive signed integer point type.
    IntegerType* new_s32(
      const std::string& id,  ///< The name of teh 32-bit signed integer type
      const Locator* loc = 0  ///< The source locator
    ) { return new_IntegerType(id,32,true,loc); }
    /// Create a new 16 bit primitive signed integer point type.
    IntegerType* new_s16(
      const std::string& id,  ///< The name of the 16-bit signed integer type
      const Locator* loc = 0  ///< THe source locator
    ) { return new_IntegerType(id,16,true,loc); }
    /// Create a new 8 bit primitive signed integer point type.
    IntegerType* new_s8(
      const std::string& id,  ///< The name of the 8-bit signed integer type
      const Locator* l = 0    ///< The source locator
    ) { return new_IntegerType(id,8,true,loc); }
    /// Create a new 128 bit primitive unsigned integer point type.
    IntegerType* new_u128(
      const std::string& id,  ///< The name of the 128-bit unsigned integer type
      const Locator* l = 0    ///< The source locator
    ) { return new_IntegerType(id,128,false,loc); }
    /// Create a new 64 bit primitive unsigned integer point type.
    IntegerType* new_u64(
      const std::string& id,  ///< The name of the 64-bit unsigned integer type
      const Locator* l = 0    ///< The source locator
    ) { return new_IntegerType(id,64,false,loc); }
    /// Create a new 32 bit primitive unsigned integer point type.
    IntegerType* new_u32(
      const std::string& id,  ///< The name of the 32-bit unsigned integer type
      const Locator* l = 0    ///< The source locator
    ) { return new_IntegerType(id,32,false,loc); }
    /// Create a new 16 bit primitive unsigned integer point type.
    IntegerType* new_u16(
      const std::string& id,  ///< The name of the 16-bit unsigned integer type
      const Locator* l = 0    ///< The source locator
    ) { return new_IntegerType(id,16,false,loc); }
    /// Create a new 8 bit primitive unsigned integer point type.
    IntegerType* new_u8(
      const std::string& id,  ///< The name of the 8-bit unsigned integer type
      const Locator* l = 0    ///< The source locator
    ) { return new_IntegerType(id,8,false,loc); }
    /// Create a new Variable node. A Variable node represents a storage
    /// location. Variables can be declared in Bundles, Functions and Blocks.
    /// Their life span is the lifespan of the container in which they are
    /// declared.
    Variable* new_Variable(
      const std::string& id,  ///< The name of the variable
      const Type* ty,         ///< The type of the variable
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new ConstantZero node
    ConstantZero* new_ConstantZero(
      const Type* Ty,         ///< The type for the constant zero
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new ConstantInteger node.
    ConstantInteger* new_ConstantInteger(
      uint64_t value,         ///< The value of the ConstantInteger
      Type* Ty,               ///< The type of the integer
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new ConstantInteger node.
    ConstantReal* new_ConstantReal(
      double value,           ///< The value of the ConstantReal
      Type* Ty,               ///< The type of the real
      const Locator* loc = 0  ///< The source locator
    );
    /// Create a new ConstantText node.
    ConstantText* new_ConstantText(
      const std::string& value, ///< The value of the ConstantText
      const Locator* loc = 0    ///< The source locator
    );
    /// Create a unary ConstantExpression Node.
    Constant* new_UnaryCE(
      NodeIDs id,            ///< The operator for the constant expression
      Constant* C,           ///< The constant operand of the unary operator
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a binary ConstantExpression Node.
    Constant* new_BinaryCE(
      NodeIDs id,            ///< The operator for the constant expression
      Constant* C1,          ///< The first operand of the binary operator
      Constant* C2,          ///< The second operand of the binary operator
      const Locator* loc = 0 ///< The source locator
    );

    /// Provide a template function for creating a nilary operator
    template<class OpClass>
    OpClass* new_NilaryOp(
      const Locator* loc = 0 ///< The source locator
    );

    /// Provide a template function for creating a unary operator
    template<class OpClass>
    OpClass* new_UnaryOp(
      Value* oprnd1,         ///< The first operand
      const Locator* loc = 0 ///< The source locator
    );

    /// Provide a template function for creating a binary operator
    template<class OpClass>
    OpClass* new_BinaryOp(
      Value* oprnd1,         ///< The first operand
      Value* oprnd2,         ///< The second operand
      const Locator* loc = 0 ///< The source locator
    );

    /// Provide a template function for creating a ternary operator
    template<class OpClass>
    OpClass* new_TernaryOp(
      Value* oprnd1,         ///< The first operand
      Value* oprnd2,         ///< The second operand
      Value* oprnd3,         ///< The third operand
      const Locator* loc = 0 ///< The source locator
    );

    /// Create a new ReturnOp node. The ReturnOp is an operator that returns
    /// immediately from the enclosing function, possibly with a result value.
    ReturnOp* new_ReturnOp(
      Value* val,            ///< The value to return
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a new StoreOp node.
    StoreOp* new_StoreOp(
      Value* var,            ///< The variable whose value is assigned
      Value* val,            ///< The value to assign to the variable
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a new LoadOp node.
    LoadOp* new_LoadOp(
      Value* var,            ///< The variable from which the value is loaded
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a new OpenOp node.
    OpenOp* new_OpenOp(
      Value* uri,            ///< The URI saying what to open and how
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a new WriteOp node.
    WriteOp* new_WriteOp(
      Value* strm,           ///< The stream to write
      Value* buffer,         ///< The buffer that should be written
      Value* len,            ///< The length of the buffer
      const Locator* loc = 0 ///< The source locator
    );
    /// Create a new CloseOp node.
    CloseOp* new_CloseOp(
      const Value* strm,     ///< The stream to close
      const Locator* loc = 0 ///< The source locator
    );

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string sysid;
    std::string pubid;
    BundleList bundles;
    Pool* pool;
  /// @}
};

} // env hlvm namespace
#endif
