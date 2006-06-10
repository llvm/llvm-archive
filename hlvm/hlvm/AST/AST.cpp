//===-- hlvm/AST/AST.cpp - AST Container Class ------------------*- C++ -*-===//
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
/// @file hlvm/AST/AST.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::AST::AST.
//===----------------------------------------------------------------------===//

#include <hlvm/AST/AST.h>
#include <hlvm/AST/Locator.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/Import.h>
#include <hlvm/AST/Documentation.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Variable.h>
#include <hlvm/AST/Constants.h>
#include <hlvm/AST/Function.h>
#include <hlvm/AST/Program.h>
#include <hlvm/AST/Block.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/InputOutput.h>
#include <hlvm/AST/SymbolTable.h>
#include <hlvm/Base/Assert.h>
#include <hlvm/Base/Pool.h>
#include <llvm/Support/Casting.h>

using namespace hlvm;

namespace 
{

class ASTImpl : public AST
{
  public:
    ASTImpl()
      : types(), vars(), funcs(), unresolvedTypes(), 
        AnyTypeSingleton(0),
        VoidSingleton(0), BooleanSingleton(), CharacterSingleton(0), 
        OctetSingleton(0), UInt8Singleton(0), UInt16Singleton(0), 
        UInt32Singleton(0), UInt64Singleton(0), UInt128Singleton(0),
        SInt8Singleton(0), SInt16Singleton(0), SInt32Singleton(0),
        SInt64Singleton(0), SInt128Singleton(0),  Float32Singleton(0),
        Float44Singleton(0), Float64Singleton(0), Float80Singleton(0),
        Float128Singleton(0), TextTypeSingleton(0), StreamTypeSingleton(0),
        BufferTypeSingleton(0), ProgramTypeSingleton(0)
      {
        pool = Pool::create("ASTPool",0,false,1024,4,0);
      }
    ~ASTImpl();

  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);

  private:
    // Pool pool;
    SymbolTable    types;
    SymbolTable    vars;
    SymbolTable    funcs;
    SymbolTable    unresolvedTypes;
    AnyType*       AnyTypeSingleton;
    VoidType*      VoidSingleton;
    BooleanType*   BooleanSingleton;
    CharacterType* CharacterSingleton;
    OctetType*     OctetSingleton;
    IntegerType*   UInt8Singleton;
    IntegerType*   UInt16Singleton;
    IntegerType*   UInt32Singleton;
    IntegerType*   UInt64Singleton;
    IntegerType*   UInt128Singleton;
    IntegerType*   SInt8Singleton;
    IntegerType*   SInt16Singleton;
    IntegerType*   SInt32Singleton;
    IntegerType*   SInt64Singleton;
    IntegerType*   SInt128Singleton;  
    RealType*      Float32Singleton;
    RealType*      Float44Singleton;
    RealType*      Float64Singleton;
    RealType*      Float80Singleton;
    RealType*      Float128Singleton;
    TextType*      TextTypeSingleton;
    StreamType*    StreamTypeSingleton;
    BufferType*    BufferTypeSingleton;
    SignatureType* ProgramTypeSingleton;

  public:
    Type* resolveType(const std::string& name);
    void addType(Type*);
    virtual void setParent(Node* parent);
    friend class AST;
};

ASTImpl::~ASTImpl()
{
}

void 
ASTImpl::insertChild(Node* child)
{
  hlvmAssert(llvm::isa<Bundle>(child) && "Can't insert that here");
  bundles.push_back(llvm::cast<Bundle>(child));
}

void 
ASTImpl::removeChild(Node* child)
{
  hlvmAssert(llvm::isa<Bundle>(child) && "Can't remove that here");
}

Type*
ASTImpl::resolveType(const std::string& name)
{
  Node* n = types.lookup(name);
  if (n)
    return llvm::cast<Type>(n);
  n = unresolvedTypes.lookup(name);
  if (n)
    return llvm::cast<OpaqueType>(n);
  OpaqueType* ot = this->new_OpaqueType(name);
  unresolvedTypes.insert(ot->getName(), ot);
  return ot;
}

void
ASTImpl::addType(Type* ty)
{
  Node* n = unresolvedTypes.lookup(ty->getName());
  if (n) {
    OpaqueType* ot = llvm::cast<OpaqueType>(n);
    // FIXME: Replace all uses of "ot" with "ty"
    unresolvedTypes.erase(ot);
  }
  types.insert(ty->getName(),ty);
}

void
ASTImpl::setParent(Node* n)
{
  hlvmAssert(!"Can't set parent of root node (AST)");
}

}

namespace hlvm 
{

AST* 
AST::create()
{
  return new ASTImpl();
}

void
AST::destroy(AST* ast)
{
  delete static_cast<ASTImpl*>(ast);
}

AST::~AST()
{
}

Type* 
AST::resolveType(const std::string& name)
{
  return static_cast<const ASTImpl*>(this)->resolveType(name);
}

SignatureType* 
AST::getProgramType() const
{
  ASTImpl* ast = const_cast<ASTImpl*>(static_cast<const ASTImpl*>(this));
  if (!ast->ProgramTypeSingleton) {
    ast->ProgramTypeSingleton = new SignatureType();
    ast->ProgramTypeSingleton->setLocator(loc);
    ast->ProgramTypeSingleton->setName("ProgramType");
    Type* intType = ast->getPrimitiveType(SInt32TypeID);
    ast->ProgramTypeSingleton->setResultType(intType);
    ArrayType* arg_array = ast->new_ArrayType("arg_array",
      ast->new_TextType("string",loc),0,loc);
    PointerType* args = ast->new_PointerType("arg_array_ptr",arg_array,loc);
    Argument* param = ast->new_Argument("args",args,loc);
    ast->ProgramTypeSingleton->addArgument(param);
  }
  return ast->ProgramTypeSingleton;
}

Locator*
AST::new_Locator(const URI* uri, uint32_t line, uint32_t col, uint32_t line2,
    uint32_t col2)
{
  hlvmAssert(uri != 0);
  if (line != 0)
    if (col != 0)
      if (line2 != 0 && col2 != 0)
        return new RangeLocator(uri,line,col,line2,col2);
      else
        return new LineColumnLocator(uri,line,col);
    else 
      return new LineLocator(uri,line);
  else
    return new URILocator(uri);
  hlvmDeadCode("Invalid Locator construction");
  return 0;
}

Bundle*
AST::new_Bundle(const std::string& id, const Locator* loc)
{
  Bundle* result = new Bundle();
  result->setLocator(loc);
  result->setName(id);
  return result;
}

Import*
AST::new_Import(const std::string& pfx, const Locator* loc)
{
  Import* result = new Import();
  result->setLocator(loc);
  result->setPrefix(pfx);
  return result;
}

IntegerType* 
AST::new_IntegerType(
  const std::string& id, 
  uint64_t bits, 
  bool isSigned,
  const Locator* loc)
{
  IntegerType* result = new IntegerType(IntegerTypeID);
  result->setBits(bits);
  result->setSigned(isSigned);
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

RangeType* 
AST::new_RangeType(const std::string& id, int64_t min, int64_t max, const Locator* loc)
{
  RangeType* result = new RangeType();
  result->setLocator(loc);
  result->setName(id);
  result->setMin(min);
  result->setMax(max);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

EnumerationType* 
AST::new_EnumerationType(
  const std::string& id,
  const Locator* loc)
{
  EnumerationType* result = new EnumerationType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

RealType* 
AST::new_RealType(
  const std::string& id,  
  uint32_t mantissa, 
  uint32_t exponent,
  const Locator* loc)
{
  RealType* result = new RealType(RealTypeID);
  result->setMantissa(mantissa);
  result->setExponent(exponent);
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

AnyType* 
AST::new_AnyType(const std::string& id, const Locator* loc)
{
  AnyType* result = new AnyType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

BooleanType* 
AST::new_BooleanType(const std::string& id, const Locator* loc)
{
  BooleanType* result = new BooleanType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

CharacterType* 
AST::new_CharacterType(const std::string& id, const Locator* loc)
{
  CharacterType* result = new CharacterType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

OctetType* 
AST::new_OctetType(const std::string& id, const Locator* loc)
{
  OctetType* result = new OctetType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

VoidType* 
AST::new_VoidType(const std::string& id, const Locator* loc)
{
  VoidType* result = new VoidType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

PointerType* 
AST::new_PointerType(
  const std::string& id,
  Type* target,
  const Locator* loc)
{
  PointerType* result = new PointerType();
  result->setLocator(loc);
  result->setName(id);
  result->setElementType(target);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

ArrayType* 
AST::new_ArrayType(
  const std::string& id,
  Type* elemType,
  uint64_t maxSize,
  const Locator* loc)
{
  ArrayType* result = new ArrayType();
  result->setLocator(loc);
  result->setName(id);
  result->setElementType(elemType);
  result->setMaxSize(maxSize);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

VectorType* 
AST::new_VectorType(
  const std::string& id,
  Type* elemType,
  uint64_t size,
  const Locator* loc)
{
  VectorType* result = new VectorType();
  result->setLocator(loc);
  result->setName(id);
  result->setElementType(elemType);
  result->setSize(size);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

AliasType* 
AST::new_AliasType(const std::string& id, Type* referrant, const Locator* loc)
{
  AliasType* result = new AliasType();
  result->setLocator(loc);
  result->setName(id);
  result->setElementType(referrant);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

StructureType*
AST::new_StructureType(const std::string& id, const Locator* loc)
{
  StructureType* result = new StructureType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

SignatureType*
AST::new_SignatureType(
  const std::string& id, 
  const Type* ty, 
  bool isVarArgs,
  const Locator* loc)
{
  SignatureType* result = new SignatureType();
  result->setLocator(loc);
  result->setName(id);
  result->setResultType(ty);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

OpaqueType*
AST::new_OpaqueType(const std::string& id, const Locator* loc)
{
  OpaqueType* result = new OpaqueType(id);
  result->setLocator(loc);
  return result;
}

ConstantInteger*
AST::new_ConstantInteger(uint64_t v, Type* Ty, const Locator* loc)
{
  ConstantInteger* result = new ConstantInteger();
  result->setLocator(loc);
  result->setValue(v);
  result->setType(Ty);
  return result;
}

ConstantReal*
AST::new_ConstantReal(double v, Type* Ty, const Locator* loc)
{
  ConstantReal* result = new ConstantReal();
  result->setLocator(loc);
  result->setValue(v);
  result->setType(Ty);
  return result;
}

ConstantText*
AST::new_ConstantText(const std::string& v, const Locator* loc)
{
  ConstantText* result = new ConstantText();
  result->setLocator(loc);
  result->setValue(v);
  result->setType( getPrimitiveType(TextTypeID) );
  return result;
}

ConstantZero*
AST::new_ConstantZero(const Type* Ty, const Locator* loc)
{
  ConstantZero* result = new ConstantZero();
  result->setLocator(loc);
  result->setType(Ty);
  return result;
}

Variable*
AST::new_Variable(const std::string& id, const Type* Ty, const Locator* loc)
{
  Variable* result = new Variable();
  result->setLocator(loc);
  result->setType(Ty);
  result->setName(id);
  return result;
}

Function*
AST::new_Function(const std::string& id, const Locator* loc)
{
  Function* result = new Function();
  result->setLocator(loc);
  result->setName(id);
  return result;
}

Program*
AST::new_Program(const std::string& id, const Locator* loc)
{
  SignatureType* program_type = getProgramType();
  ASTImpl* ast = static_cast<ASTImpl*>(this);

  Program* result = new Program();
  result->setLocator(loc);
  result->setName(id);
  result->setSignature(program_type);
  result->setLinkageKind(ExternalLinkage);
  return result;
}

Block*
AST::new_Block(const std::string& label, const Locator* loc)
{
  Block* result = new Block();
  result->setLabel(label);
  result->setLocator(loc);
  return result;
}

template<class OpClass> 
OpClass* 
AST::new_NilaryOp(
  const Locator* loc ///< The source locator
)
{
  OpClass* result = new OpClass();
  result->setLocator(loc);
  return result;
}

/// Provide a template function for creating a unary operator
template<class OpClass>
OpClass* 
AST::new_UnaryOp(
  Value* oprnd1,     ///< The first operand
  const Locator* loc ///< The source locator
)
{
  OpClass* result = new OpClass();
  result->setLocator(loc);
  result->setOperand(0,oprnd1);
  return result;
}

/// Provide a template function for creating a binary operator
template<class OpClass>
OpClass* 
AST::new_BinaryOp(
  Value* oprnd1,     ///< The first operand
  Value* oprnd2,     ///< The second operand
  const Locator* loc ///< The source locator
)
{
  OpClass* result = new OpClass();
  result->setLocator(loc);
  result->setOperand(0,oprnd1);
  result->setOperand(1,oprnd2);
  return result;
}

/// Provide a template function for creating a ternary operator
template<class OpClass>
OpClass* 
AST::new_TernaryOp(
  Value* oprnd1,     ///< The first operand
  Value* oprnd2,     ///< The second operand
  Value* oprnd3,     ///< The third operand
  const Locator* loc ///< The source locator
)
{
  OpClass* result = new OpClass();
  result->setLocator(loc);
  result->setOperand(0,oprnd1);
  result->setOperand(1,oprnd2);
  result->setOperand(2,oprnd3);
  return result;
}

// Control Flow Operators
template ReturnOp* 
AST::new_UnaryOp<ReturnOp>(Value*op1,const Locator*loc);
// Memory Operators
template StoreOp*  
AST::new_BinaryOp<StoreOp>(Value*op1,Value*op2,const Locator*loc);
template LoadOp*   
AST::new_UnaryOp<LoadOp>(Value*op1,const Locator*loc);
template AutoVarOp*
AST::new_UnaryOp<AutoVarOp>(Value*op1,const Locator* loc);
template ReferenceOp* 
AST::new_NilaryOp<ReferenceOp>(const Locator*loc);
// Input/Output Operators
template OpenOp* 
AST::new_UnaryOp<OpenOp>(Value*op1,const Locator*loc);
template WriteOp* 
AST::new_BinaryOp<WriteOp>(Value*op1,Value*op2,const Locator*loc);
template CloseOp* 
AST::new_UnaryOp<CloseOp>(Value*op1,const Locator*loc);

Documentation* 
AST::new_Documentation(const Locator* loc)
{
  Documentation* result = new Documentation();
  result->setLocator(loc);
  return result;
}

Argument* 
AST::new_Argument(const std::string& id, Type* ty , const Locator* loc)
{
  Argument* result = new Argument();
  result->setLocator(loc);
  result->setName(id);
  result->setElementType(ty);
  return result;
}

BufferType* 
AST::new_BufferType( const std::string& id, const Locator* loc)
{
  BufferType* result = new BufferType();
  result->setName(id);
  result->setLocator(loc);
  return result;
}

StreamType* 
AST::new_StreamType( const std::string& id, const Locator* loc)
{
  StreamType* result = new StreamType();
  result->setName(id);
  result->setLocator(loc);
  return result;
}

TextType*
AST::new_TextType(const std::string& id, const Locator* loc)
{
  TextType* result = new TextType();
  result->setName(id);
  result->setLocator(loc);
  return result;
}

Type* 
AST::getPrimitiveType(NodeIDs pid)
{
  ASTImpl* ast = static_cast<ASTImpl*>(this);
  switch (pid) 
  {
    case AnyTypeID:
      if (!ast->AnyTypeSingleton) {
        ast->AnyTypeSingleton = new AnyType();
        ast->AnyTypeSingleton->setName("any");
      }
      return ast->AnyTypeSingleton;
    case VoidTypeID:
      if (!ast->VoidSingleton) {
        ast->VoidSingleton = new VoidType();
        ast->VoidSingleton->setName("void");
      }
      return ast->VoidSingleton;
    case BooleanTypeID:
      if (!ast->BooleanSingleton) {
        ast->BooleanSingleton = new BooleanType();
        ast->BooleanSingleton->setName("bool");
      }
      return ast->BooleanSingleton;
    case CharacterTypeID:
      if (!ast->CharacterSingleton) {
        ast->CharacterSingleton = new CharacterType();
        ast->CharacterSingleton->setName("char");
      }
      return ast->CharacterSingleton;
    case OctetTypeID:
      if (!ast->OctetSingleton) {
        ast->OctetSingleton = new OctetType();
        ast->OctetSingleton->setName("octet");
      }
      return ast->OctetSingleton;
    case UInt8TypeID:
      if (!ast->UInt8Singleton) {
        ast->UInt8Singleton = new IntegerType(UInt8TypeID,8,false);
        ast->UInt8Singleton->setName("uint8_t");
      }
      return ast->UInt8Singleton;
    case UInt16TypeID:
      if (!ast->UInt16Singleton) {
        ast->UInt16Singleton = new IntegerType(UInt16TypeID,16,false);
        ast->UInt16Singleton->setName("uint16_t");
      }
      return ast->UInt16Singleton;
    case UInt32TypeID:
      if (!ast->UInt32Singleton) {
        ast->UInt32Singleton = new IntegerType(UInt32TypeID,32,false);
        ast->UInt32Singleton->setName("uint32_t");
      }
      return ast->UInt32Singleton;
    case UInt64TypeID:
      if (!ast->UInt64Singleton) {
        ast->UInt64Singleton = new IntegerType(UInt64TypeID,64,false);
        ast->UInt64Singleton->setName("uint64_t");
      }
      return ast->UInt64Singleton;
    case UInt128TypeID:
      if (!ast->UInt128Singleton) {
        ast->UInt128Singleton = new IntegerType(UInt128TypeID,128,false);
        ast->UInt128Singleton->setName("uint128_t");
      }
      return ast->UInt128Singleton;
    case SInt8TypeID:
      if (!ast->SInt8Singleton) {
        ast->SInt8Singleton = new IntegerType(SInt8TypeID,8,false);
        ast->SInt8Singleton->setName("int8_t");
      }
      return ast->SInt8Singleton;
    case SInt16TypeID:
      if (!ast->SInt16Singleton) {
        ast->SInt16Singleton = new IntegerType(SInt16TypeID,16,false);
        ast->SInt16Singleton->setName("int16_t");
      }
      return ast->SInt16Singleton;
    case SInt32TypeID:
      if (!ast->SInt32Singleton) {
        ast->SInt32Singleton = new IntegerType(SInt32TypeID,32,false);
        ast->SInt32Singleton->setName("int32_t");
      }
      return ast->SInt32Singleton;
    case SInt64TypeID:
      if (!ast->SInt64Singleton) {
        ast->SInt64Singleton = new IntegerType(SInt64TypeID,64,false);
        ast->SInt64Singleton->setName("int64_t");
      }
      return ast->SInt64Singleton;
    case SInt128TypeID:
      if (!ast->SInt128Singleton) {
        ast->SInt128Singleton = new IntegerType(SInt128TypeID,128,false);
        ast->SInt128Singleton->setName("int128_t");
      }
      return ast->SInt128Singleton;
    case Float32TypeID:
      if (!ast->Float32Singleton) {
        ast->Float32Singleton = new RealType(Float32TypeID,23,8);
        ast->Float32Singleton->setName("f32");
      }
      return ast->Float32Singleton;
    case Float44TypeID:
      if (!ast->Float44Singleton) {
        ast->Float44Singleton = new RealType(Float44TypeID,32,11);
        ast->Float44Singleton->setName("f44");
      }
      return ast->Float44Singleton;
    case Float64TypeID:
      if (!ast->Float64Singleton) {
        ast->Float64Singleton = new RealType(Float64TypeID,52,11);
        ast->Float64Singleton->setName("f64");
      }
      return ast->Float64Singleton;
    case Float80TypeID:
      if (!ast->Float80Singleton) {
        ast->Float80Singleton = new RealType(Float80TypeID,64,15);
        ast->Float80Singleton->setName("f80");
      }
      return ast->Float80Singleton;
    case Float128TypeID:
      if (!ast->Float128Singleton) {
        ast->Float128Singleton = new RealType(Float128TypeID,112,15);
        ast->Float128Singleton->setName("f128");
      }
      return ast->Float128Singleton;
    case TextTypeID:
      if (!ast->TextTypeSingleton) {
        ast->TextTypeSingleton = new TextType();
      }
      return ast->TextTypeSingleton;
    case StreamTypeID:
      if (!ast->StreamTypeSingleton) {
        ast->StreamTypeSingleton = new StreamType();
      }
      return ast->StreamTypeSingleton;
    case BufferTypeID:
      if (!ast->BufferTypeSingleton) {
        ast->BufferTypeSingleton = new BufferType();
      }
      return ast->BufferTypeSingleton;
    default:
      hlvmDeadCode("Invalid Primitive");
      break;
  }
  return 0;
}

}
