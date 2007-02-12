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
#include <hlvm/AST/Documentation.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Linkables.h>
#include <hlvm/AST/Constants.h>
#include <hlvm/AST/Block.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/InputOutput.h>
#include <hlvm/AST/Arithmetic.h>
#include <hlvm/AST/BooleanOps.h>
#include <hlvm/AST/RealMath.h>
#include <hlvm/AST/StringOps.h>
#include <hlvm/AST/SymbolTable.h>
#include <hlvm/Base/Assert.h>
#include <hlvm/Base/Pool.h>
#include <llvm/Support/Casting.h>

using namespace hlvm;

namespace hlvm 
{

AST::AST() 
  : Node(TreeTopID), sysid(), pubid(), bundles(), pool(0)
{
  pool = Pool::create("ASTPool",0,false,1024,4,0);
}

AST::~AST()
{
}

void 
AST::insertChild(Node* child)
{
  hlvmAssert(llvm::isa<Bundle>(child) && "Can't insert that here");
#ifdef HLVM_ASSERT
  for (const_iterator I = begin(), E = end(); I != E; ++I)
    hlvmAssert((*I) != child && "Attempt to duplicate insertion of child");
#endif
  bundles.push_back(llvm::cast<Bundle>(child));
}

void 
AST::removeChild(Node* child)
{
  hlvmAssert(llvm::isa<Bundle>(child) && "Can't remove that here");
  //FIXME: bundles.erase(llvm::cast<Bundle>(child));
}

void
AST::setParent(Node* n)
{
  hlvmAssert(!"Can't set parent of root node (AST)!");
}

AST* 
AST::create()
{
  return new AST();
}

void
AST::destroy(AST* ast)
{
  delete ast;
}

URI* 
AST::new_URI(const std::string& uri)
{
  URI* result = URI::create(uri,getPool());
  return result;
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

Documentation* 
AST::new_Documentation(const Locator* loc)
{
  Documentation* result = new Documentation();
  result->setLocator(loc);
  return result;
}

Bundle*
AST::new_Bundle(const std::string& id, const Locator* loc)
{
  Bundle* result = new Bundle();
  result->setLocator(loc);
  result->setName(id);
  result->setParent(this);
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
  Bundle* bundle,
  uint16_t bits, 
  bool isSigned,
  const Locator* loc)
{
  IntegerType* result = new IntegerType(bits,isSigned);
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

RangeType* 
AST::new_RangeType(
  const std::string& id, 
  Bundle* bundle,
  int64_t min, int64_t max, const Locator* loc)
{
  RangeType* result = new RangeType(min,max);
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

EnumerationType* 
AST::new_EnumerationType(
  const std::string& id,
  Bundle* bundle,
  const Locator* loc)
{
  EnumerationType* result = new EnumerationType();
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

RealType* 
AST::new_RealType(
  const std::string& id,  
  Bundle* bundle,
  uint32_t mantissa, 
  uint32_t exponent,
  const Locator* loc)
{
  RealType* result = new RealType();
  result->setMantissa(mantissa);
  result->setExponent(exponent);
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

AnyType* 
AST::new_AnyType(
  const std::string& id, 
  Bundle* bundle,
  const Locator* loc)
{
  AnyType* result = new AnyType();
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

StringType* 
AST::new_StringType(
  const std::string& id, 
  Bundle* bundle,
  const std::string& encoding,
  const Locator* loc)
{
  StringType* result = new StringType(encoding);
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

BooleanType* 
AST::new_BooleanType(
  const std::string& id, 
  Bundle* bundle,
  const Locator* loc)
{
  BooleanType* result = new BooleanType();
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

BufferType* 
AST::new_BufferType(
  const std::string& id, 
  Bundle* bundle,
  const Locator* loc)
{
  BufferType* result = new BufferType();
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

TextType* 
AST::new_TextType(
  const std::string& id, 
  Bundle* bundle,
  const Locator* loc)
{
  TextType* result = new TextType();
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

StreamType* 
AST::new_StreamType(
  const std::string& id,  
  Bundle* bundle,
  const Locator* loc)
{
  StreamType* result = new StreamType();
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

CharacterType* 
AST::new_CharacterType(
  const std::string& id, 
  Bundle* bundle,
  const std::string& encoding,
  const Locator* loc)
{
  CharacterType* result = new CharacterType(encoding);
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

PointerType* 
AST::new_PointerType(
  const std::string& id,
  Bundle* bundle,
  const Type* target,
  const Locator* loc)
{
  PointerType* result = new PointerType();
  result->setLocator(loc);
  result->setName(id);
  result->setElementType(target);
  result->setParent(bundle);
  return result;
}

ArrayType* 
AST::new_ArrayType(
  const std::string& id,
  Bundle* bundle,
  Type* elemType,
  uint64_t maxSize,
  const Locator* loc)
{
  ArrayType* result = new ArrayType();
  result->setLocator(loc);
  result->setName(id);
  result->setElementType(elemType);
  result->setMaxSize(maxSize);
  result->setParent(bundle);
  return result;
}

VectorType* 
AST::new_VectorType(
  const std::string& id,
  Bundle* bundle,
  Type* elemType,
  uint64_t size,
  const Locator* loc)
{
  VectorType* result = new VectorType();
  result->setLocator(loc);
  result->setName(id);
  result->setElementType(elemType);
  result->setSize(size);
  result->setParent(bundle);
  return result;
}

NamedType* 
AST::new_NamedType(
  const std::string& name, 
  const Type* type,
  const Locator* loc
)
{
  NamedType* result = new NamedType();
  result->setLocator(loc);
  result->setName(name);
  result->setType(type);
  return result;
}

StructureType*
AST::new_StructureType(
  const std::string& id, 
  Bundle* bundle,
  const Locator* loc)
{
  StructureType* result = new StructureType();
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

ContinuationType*
AST::new_ContinuationType(
  const std::string& id, 
  Bundle* bundle,
  const Locator* loc)
{
  ContinuationType* result = new ContinuationType();
  result->setLocator(loc);
  result->setName(id);
  result->setParent(bundle);
  return result;
}

SignatureType*
AST::new_SignatureType(
  const std::string& id, 
  Bundle* bundle,
  const Type* ty, 
  bool isVarArgs,
  const Locator* loc)
{
  SignatureType* result = new SignatureType();
  result->setLocator(loc);
  result->setName(id);
  result->setResultType(ty);
  result->setParent(bundle);
  return result;
}

OpaqueType*
AST::new_OpaqueType(
  const std::string& id, 
  bool is_unresolved,
  Bundle* bundle,
  const Locator* loc)
{
  OpaqueType* result = new OpaqueType(id);
  result->setLocator(loc);
  if (is_unresolved)
    result->setIsUnresolved(true);
  else {
    result->setIsUnresolved(false);
    result->setParent(bundle);
  }
  return result;
}

ConstantAny* 
AST::new_ConstantAny(
  const std::string& name,
  Bundle* bundle,
  const Type* type,
  ConstantValue* val,
  const Locator* loc)
{
  ConstantAny* result = new ConstantAny(val);
  result->setLocator(loc);
  result->setName(name);
  result->setType(type);
  result->setParent(bundle);
  return result;
}

ConstantBoolean* 
AST::new_ConstantBoolean(
  const std::string& name,
  Bundle* bundle,
  const Type* type,
  bool t_or_f, 
  const Locator* loc)
{
  ConstantBoolean* result = new ConstantBoolean(t_or_f);
  result->setLocator(loc);
  result->setName(name);
  result->setType(type);
  result->setParent(bundle);
  return result;
}

ConstantCharacter* 
AST::new_ConstantCharacter(
  const std::string& name,
  Bundle* bundle,
  const Type* type,
  const std::string& val,
  const Locator* loc)
{
  ConstantCharacter* result = new ConstantCharacter(val);
  result->setLocator(loc);
  result->setName(name);
  result->setType( type );
  result->setParent(bundle);
  return result;
}

ConstantEnumerator* 
AST::new_ConstantEnumerator(
  const std::string& name,///< The name of the constant
  Bundle* bundle,
  const Type* Ty,         ///< The type of the enumerator
  const std::string& val, ///< The value for the constant
  const Locator* loc      ///< The source locator
)
{
  ConstantEnumerator* result = new ConstantEnumerator(val);
  result->setLocator(loc);
  result->setName(name);
  result->setType(Ty);
  result->setParent(bundle);
  return result;
}

ConstantInteger*
AST::new_ConstantInteger(
  const std::string& name,
  Bundle* bundle,
  const Type* type,
  const std::string&  v, 
  uint16_t base, 
  const Locator* loc)
{
  ConstantInteger* result = new ConstantInteger(base);
  result->setLocator(loc);
  result->setName(name);
  result->setValue(v);
  result->setBase(base);
  result->setType(type);
  result->setParent(bundle);
  return result;
}

ConstantReal*
AST::new_ConstantReal(
  const std::string& name,
  Bundle* bundle,
  const Type* type,
  const std::string& v, 
  const Locator* loc)
{
  ConstantReal* result = new ConstantReal();
  result->setLocator(loc);
  result->setName(name);
  result->setValue(v);
  result->setType(type);
  result->setParent(bundle);
  return result;
}

ConstantString*
AST::new_ConstantString(
  const std::string& name,
  Bundle* bundle,
  const Type* type,
  const std::string& v, 
  const Locator* loc)
{
  ConstantString* result = new ConstantString();
  result->setLocator(loc);
  result->setName(name);
  result->setValue(v);
  result->setType(type);
  result->setParent(bundle);
  return result;
}

ConstantPointer* 
AST::new_ConstantPointer(
  const std::string& name,
  Bundle* bundle,
  const Type* type,
  Constant* referent,
  const Locator* loc
)
{
  ConstantPointer* result = new ConstantPointer(referent);
  result->setLocator(loc);
  result->setName(name);
  result->setType(type);
  result->setParent(bundle);
  return result;
}

ConstantArray* 
AST::new_ConstantArray(
  const std::string& name,
  Bundle* bundle,
  const ArrayType* AT,
  const std::vector<ConstantValue*>& vals,
  const Locator* loc
)
{
  ConstantArray* result = new ConstantArray();
  result->setLocator(loc);
  result->setName(name);
  for (std::vector<ConstantValue*>::const_iterator I = vals.begin(),
       E = vals.end(); I != E; ++I ) 
  {
    result->addConstant(*I);
  }
  result->setType(AT);
  result->setParent(bundle);
  return result;
}

ConstantVector* 
AST::new_ConstantVector(
  const std::string& name,
  Bundle* bundle,
  const VectorType* VT,
  const std::vector<ConstantValue*>& vals,
  const Locator* loc
)
{
  ConstantVector* result = new ConstantVector();
  result->setLocator(loc);
  result->setName(name);
  for (std::vector<ConstantValue*>::const_iterator I = vals.begin(),
       E = vals.end(); I != E; ++I ) 
  {
    result->addConstant(*I);
  }
  result->setType(VT);
  result->setParent(bundle);
  return result;
}

ConstantStructure* 
AST::new_ConstantStructure(
  const std::string& name,
  Bundle* bundle,
  const StructureType* ST,
  const std::vector<ConstantValue*>& vals,
  const Locator* loc
)
{
  ConstantStructure* result = new ConstantStructure();
  result->setLocator(loc);
  result->setName(name);
  hlvmAssert(ST->size() == vals.size());
  StructureType::const_iterator STI = ST->begin();
  for (std::vector<ConstantValue*>::const_iterator I = vals.begin(),
       E = vals.end(); I != E; ++I ) 
  {
    hlvmAssert(STI != ST->end());
    result->addConstant(*I);
    ++STI;
  }
  result->setType(ST);
  result->setParent(bundle);
  return result;
}

ConstantContinuation* 
AST::new_ConstantContinuation(
  const std::string& name,
  Bundle* bundle,
  const ContinuationType* ST,
  const std::vector<ConstantValue*>& vals,
  const Locator* loc
)
{
  ConstantContinuation* result = new ConstantContinuation();
  result->setLocator(loc);
  result->setName(name);
  hlvmAssert(ST->size() == vals.size());
  ContinuationType::const_iterator STI = ST->begin();
  for (std::vector<ConstantValue*>::const_iterator I = vals.begin(),
       E = vals.end(); I != E; ++I ) 
  {
    hlvmAssert(STI != ST->end());
    result->addConstant(*I);
    ++STI;
  }
  result->setType(ST);
  result->setParent(bundle);
  return result;
}

Variable*
AST::new_Variable(
  const std::string& id, 
  Bundle* B,
  const Type* Ty, 
  const Locator* loc)
{
  Variable* result = new Variable();
  result->setName(id);
  result->setType(Ty);
  result->setLocator(loc);
  result->setParent(B);
  return result;
}

Argument*
AST::new_Argument(
  const std::string& name, const Type* Ty, const Locator* loc)
{
  Argument* result = new Argument();
  result->setName(name);
  result->setType(Ty);
  result->setLocator(loc);
  return result;
}

Function*
AST::new_Function(
  const std::string& id, 
  Bundle* B,
  const SignatureType* ty, 
  const Locator* loc)
{
  Function* result = new Function();
  result->setLocator(loc);
  result->setName(id);
  result->setType(ty);
  for (SignatureType::const_iterator I = ty->begin(), E = ty->end(); 
       I != E; ++I ) 
  {
    const Type* Ty = (*I)->getType();
    assert(Ty && "Arguments can't be void type");
    Argument* arg = new_Argument((*I)->getName(),Ty,loc);
    result->addArgument(arg);
  }
  result->setParent(B);
  return result;
}

Program*
AST::new_Program(
  const std::string& id, 
  Bundle* B,
  const Locator* loc)
{
  SignatureType* ty = B->getProgramType();
  Program* result = new Program();
  result->setLocator(loc);
  result->setName(id);
  result->setType(ty);
  result->setLinkageKind(ExternalLinkage);
  for (SignatureType::const_iterator I = ty->begin(), E = ty->end(); 
       I != E; ++I ) 
  {
    const Type* Ty = (*I)->getType();
    assert(Ty && "Arguments can't be void type");
    Argument* arg = new_Argument((*I)->getName(),Ty,loc);
    result->addArgument(arg);
  }
  result->setParent(B);
  return result;
}

Block* 
AST::new_Block( const Locator* loc)
{
  Block* result = new Block();
  result->setLocator(loc);
  return result;
}

AutoVarOp*
AST::new_AutoVarOp(
    const std::string& name, 
    const Type* Ty, 
    const Locator* loc)
{
  hlvmAssert(Ty != 0 && "AutoVarOp must have a Type!");
  AutoVarOp* result = new AutoVarOp();
  result->setType(Ty);
  result->setLocator(loc);
  result->setName(name);
  return result;
}

GetOp* 
AST::new_GetOp(const Value* V, const Locator*loc)
{
  hlvmAssert(V != 0 && "GetOp must have a Value to reference");
  GetOp* result = new GetOp();
  if (llvm::isa<AutoVarOp>(V) || 
      llvm::isa<Argument>(V) ||
      llvm::isa<Constant>(V)) {
    result->setType(llvm::cast<Value>(V)->getType());
  } else
    hlvmAssert(!"Invalid referent type");
  result->setLocator(loc);
  result->setReferent(V);
  result->setType(V->getType());
  return result;
}

GetFieldOp* 
AST::new_GetFieldOp(
  Operator* op,
  const std::string& name,
  const Locator* loc
)
{
  GetFieldOp* result = new GetFieldOp();
  result->setLocator(loc);
  result->setOperand(0,op);
  result->setFieldName(name);
  result->setType(result->getFieldType());
  return result;
}

GetIndexOp* 
AST::new_GetIndexOp(
  Operator* op1,
  Operator* op2,
  const Locator* loc
)
{
  GetIndexOp* result = new GetIndexOp();
  result->setLocator(loc);
  result->setOperand(0,op1);
  result->setOperand(1,op2);
  result->setType(result->getIndexedType());
  return result;
}

ConvertOp*
AST::new_ConvertOp(Operator* V, const Type* Ty, const Locator* loc)
{
  hlvmAssert(V != 0 && "ConvertOp must have a Value to convert");
  hlvmAssert(Ty != 0 && "ConvertOp must have a type to convert the value");
  ConvertOp* result = new ConvertOp();
  result->setType(Ty);
  result->setOperand(0,V);
  result->setLocator(loc);
  return result;
}

template<class OpClass> OpClass* 
AST::new_NilaryOp(
  const Type* Ty,    ///< Result type of the operator
  const Locator* loc ///< The source locator
)
{
  OpClass* result = new OpClass();
  result->setLocator(loc);
  result->setType(Ty);
  return result;
}

template<class OpClass> OpClass* 
AST::new_NilaryOp(
  Bundle* bundle,    ///< Bundle, for type resolution
  const Locator* loc ///< The source locator
)
{
  return new_NilaryOp<OpClass>(static_cast<Type*>(0),loc);
}

/// Provide a template function for creating a unary operator
template<class OpClass> OpClass* 
AST::new_UnaryOp(
  const Type* Ty,    ///< Result type of the operator
  Operator* oprnd1,  ///< The first operand
  const Locator* loc ///< The source locator
)
{
  hlvmAssert(oprnd1 != 0 && "Invalid Operand for UnaryOp");
  OpClass* result = new OpClass();
  result->setLocator(loc);
  result->setType(Ty);
  result->setOperand(0,oprnd1);
  return result;
}

template<class OpClass> OpClass* 
AST::new_UnaryOp(
  Operator* oprnd1,     ///< The first operand
  Bundle* B,         ///< The bundle, for type lookup
  const Locator* loc ///< The source locator
)
{
  return new_UnaryOp<OpClass>(oprnd1->getType(),oprnd1,loc);
}

/// Provide a template function for creating a binary operator
template<class OpClass> OpClass* 
AST::new_BinaryOp(
  const Type* Ty,    ///< Result type of the operator
  Operator* oprnd1,  ///< The first operand
  Operator* oprnd2,  ///< The second operand
  const Locator* loc ///< The source locator
)
{
  hlvmAssert(oprnd1 != 0 && "Invalid Operand for BinaryOp");
  hlvmAssert(oprnd2 != 0 && "Invalid Operand for BinaryOp");
  OpClass* result = new OpClass();
  result->setLocator(loc);
  result->setType(Ty);
  result->setOperand(0,oprnd1);
  result->setOperand(1,oprnd2);
  return result;
}

/// Provide a template function for creating a binary operator
template<class OpClass> OpClass* 
AST::new_BinaryOp(
  Operator* oprnd1,  ///< The first operand
  Operator* oprnd2,  ///< The second operand
  Bundle* B,         ///< The bundle, for type lookup
  const Locator* loc ///< The source locator
)
{
  return new_BinaryOp<OpClass>(oprnd1->getType(),oprnd1,oprnd2,loc);
}

/// Provide a template function for creating a ternary operator
template<class OpClass> OpClass* 
AST::new_TernaryOp(
  const Type* Ty,    ///< Result type of the operator
  Operator* oprnd1,     ///< The first operand
  Operator* oprnd2,     ///< The second operand
  Operator* oprnd3,     ///< The third operand
  const Locator* loc ///< The source locator
)
{
  hlvmAssert(oprnd1 != 0 && "Invalid Operand for TernaryOp");
  hlvmAssert(oprnd2 != 0 && "Invalid Operand for TernUnaryOp");
  hlvmAssert(oprnd3 != 0 && "Invalid Operand for TernUnaryOp");
  OpClass* result = new OpClass();
  result->setLocator(loc);
  result->setType(Ty);
  result->setOperand(0,oprnd1);
  result->setOperand(1,oprnd2);
  result->setOperand(2,oprnd3);
  return result;
}

template<class OpClass> OpClass* 
AST::new_TernaryOp(
  Operator* oprnd1,     ///< The first operand
  Operator* oprnd2,     ///< The second operand
  Operator* oprnd3,     ///< The third operand
  Bundle* B,
  const Locator* loc ///< The source locator
)
{
  return new_TernaryOp<OpClass>(oprnd1->getType(),oprnd1,oprnd2,oprnd3,loc);
}

template<class OpClass> OpClass* 
AST::new_MultiOp(
  const Type* Ty,         ///< Result type of the operator
  const std::vector<Operator*>& oprnds,
  const Locator* loc
) 
{
  OpClass* result = new OpClass();
  result->setLocator(loc);
  result->setType(Ty);
  result->addOperands(oprnds);
  return result;
}

template<class OpClass> OpClass* 
AST::new_MultiOp(
  const std::vector<Operator*>& oprnds,
  Bundle* B,
  const Locator* loc
)
{
  hlvmAssert(!oprnds.empty() && "No operands?");
  return new_MultiOp<OpClass>(oprnds[0]->getType(),oprnds,loc);
}

// Arithmetic Operators
template NegateOp*
AST::new_UnaryOp<NegateOp>(
    const Type* Ty, Operator* op1, const Locator* loc);
template NegateOp*
AST::new_UnaryOp<NegateOp>(Operator* op1, Bundle* B, const Locator* loc);

template ComplementOp*
AST::new_UnaryOp<ComplementOp>(
    const Type* Ty, Operator* op1, const Locator* loc);
template ComplementOp*
AST::new_UnaryOp<ComplementOp>(Operator* op1, Bundle* B, const Locator* loc);

template PreIncrOp*
AST::new_UnaryOp<PreIncrOp>(
    const Type* Ty, Operator* op1, const Locator* loc);
template<> PreIncrOp*
AST::new_UnaryOp<PreIncrOp>(Operator* op1, Bundle* B, const Locator* loc)
{
  const Type* Ty = llvm::cast<GetOp>(op1)->getReferentType();
  return new_UnaryOp<PreIncrOp>(Ty,op1,loc);
}

template PreDecrOp*
AST::new_UnaryOp<PreDecrOp>(
    const Type* Ty, Operator* op1, const Locator* loc);
template<> PreDecrOp*
AST::new_UnaryOp<PreDecrOp>(Operator* op1, Bundle* B, const Locator* loc)
{
  const Type* Ty = llvm::cast<GetOp>(op1)->getReferentType();
  return new_UnaryOp<PreDecrOp>(Ty,op1,loc);
}

template PostIncrOp*
AST::new_UnaryOp<PostIncrOp>(const Type* Ty, Operator* op1, const Locator* loc);
template<> PostIncrOp*
AST::new_UnaryOp<PostIncrOp>(Operator* op1, Bundle* B, const Locator* loc)
{
  const Type* Ty = llvm::cast<GetOp>(op1)->getReferentType();
  return new_UnaryOp<PostIncrOp>(Ty,op1,loc);
}

template PostDecrOp*
AST::new_UnaryOp<PostDecrOp>(const Type* Ty, Operator* op1, const Locator* loc);
template<> PostDecrOp*
AST::new_UnaryOp<PostDecrOp>(Operator* op1, Bundle* B, const Locator* loc)
{
  const Type* Ty = llvm::cast<GetOp>(op1)->getReferentType();
  return new_UnaryOp<PostDecrOp>(Ty,op1,loc);
}

template SizeOfOp*
AST::new_UnaryOp<SizeOfOp>(const Type* Ty, Operator* op1, const Locator* loc);
template SizeOfOp*
AST::new_UnaryOp<SizeOfOp>(Operator* op1, Bundle* B, const Locator* loc);

template LengthOp*
AST::new_UnaryOp<LengthOp>(const Type* Ty, Operator* op1, const Locator* loc);
template LengthOp*
AST::new_UnaryOp<LengthOp>(Operator* op1, Bundle* B, const Locator* loc);

template AddOp*
AST::new_BinaryOp<AddOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template AddOp*
AST::new_BinaryOp<AddOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

template SubtractOp*
AST::new_BinaryOp<SubtractOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template SubtractOp*
AST::new_BinaryOp<SubtractOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

template MultiplyOp*
AST::new_BinaryOp<MultiplyOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template MultiplyOp*
AST::new_BinaryOp<MultiplyOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

template DivideOp*
AST::new_BinaryOp<DivideOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template DivideOp*
AST::new_BinaryOp<DivideOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

template ModuloOp*
AST::new_BinaryOp<ModuloOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template ModuloOp*
AST::new_BinaryOp<ModuloOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

template BAndOp*
AST::new_BinaryOp<BAndOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template BAndOp*
AST::new_BinaryOp<BAndOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

template BOrOp*
AST::new_BinaryOp<BOrOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template BOrOp*
AST::new_BinaryOp<BOrOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

template BXorOp*
AST::new_BinaryOp<BXorOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template BXorOp*
AST::new_BinaryOp<BXorOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

template BNorOp*
AST::new_BinaryOp<BNorOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template BNorOp*
AST::new_BinaryOp<BNorOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

// Real Operators
template IsPInfOp*
AST::new_UnaryOp<IsPInfOp>(const Type* Ty, Operator* op1, const Locator* loc);
template IsPInfOp*
AST::new_UnaryOp<IsPInfOp>(Operator* op1, Bundle* B, const Locator* loc);

template IsNInfOp*
AST::new_UnaryOp<IsNInfOp>(const Type* Ty, Operator* op1, const Locator* loc);
template IsNInfOp*
AST::new_UnaryOp<IsNInfOp>(Operator* op1, Bundle* B, const Locator* loc);

template IsNanOp*
AST::new_UnaryOp<IsNanOp>(const Type* Ty, Operator* op1, const Locator* loc);
template IsNanOp*
AST::new_UnaryOp<IsNanOp>(Operator* op1, Bundle* B, const Locator* loc);

template TruncOp*
AST::new_UnaryOp<TruncOp>(const Type* Ty, Operator* op1, const Locator* loc);
template TruncOp*
AST::new_UnaryOp<TruncOp>(Operator* op1, Bundle* B, const Locator* loc);

template RoundOp*
AST::new_UnaryOp<RoundOp>(const Type* Ty, Operator* op1, const Locator* loc);
template RoundOp*
AST::new_UnaryOp<RoundOp>(Operator* op1, Bundle* B, const Locator* loc);

template FloorOp*
AST::new_UnaryOp<FloorOp>(const Type* Ty, Operator* op1, const Locator* loc);
template FloorOp*
AST::new_UnaryOp<FloorOp>(Operator* op1, Bundle* B, const Locator* loc);

template CeilingOp*
AST::new_UnaryOp<CeilingOp>(const Type* Ty, Operator* op1, const Locator* loc);
template CeilingOp*
AST::new_UnaryOp<CeilingOp>(Operator* op1, Bundle* B, const Locator* loc);

template LogEOp*
AST::new_UnaryOp<LogEOp>(const Type* Ty, Operator* op1, const Locator* loc);
template LogEOp*
AST::new_UnaryOp<LogEOp>(Operator* op1, Bundle* B, const Locator* loc);

template Log2Op*
AST::new_UnaryOp<Log2Op>(const Type* Ty, Operator* op1, const Locator* loc);
template Log2Op*
AST::new_UnaryOp<Log2Op>(Operator* op1, Bundle* B, const Locator* loc);

template Log10Op*
AST::new_UnaryOp<Log10Op>(const Type* Ty, Operator* op1, const Locator* loc);
template Log10Op*
AST::new_UnaryOp<Log10Op>(Operator* op1, Bundle* B, const Locator* loc);

template SquareRootOp*
AST::new_UnaryOp<SquareRootOp>(const Type* Ty, Operator* op1, const Locator* loc);
template SquareRootOp*
AST::new_UnaryOp<SquareRootOp>(Operator* op1, Bundle* B, const Locator* loc);

template CubeRootOp*
AST::new_UnaryOp<CubeRootOp>(const Type* Ty, Operator* op1, const Locator* loc);
template CubeRootOp*
AST::new_UnaryOp<CubeRootOp>(Operator* op1, Bundle* B, const Locator* loc);

template FactorialOp*
AST::new_UnaryOp<FactorialOp>(const Type* Ty, Operator* op1, const Locator* loc);
template FactorialOp*
AST::new_UnaryOp<FactorialOp>(Operator* op1, Bundle* B, const Locator* loc);

template PowerOp*
AST::new_BinaryOp<PowerOp>(const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template PowerOp*
AST::new_BinaryOp<PowerOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

template RootOp*
AST::new_BinaryOp<RootOp>(const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template RootOp*
AST::new_BinaryOp<RootOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

template GCDOp*
AST::new_BinaryOp<GCDOp>(const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template GCDOp*
AST::new_BinaryOp<GCDOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

template LCMOp*
AST::new_BinaryOp<LCMOp>(const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template LCMOp*
AST::new_BinaryOp<LCMOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

// Boolean Operators
template NotOp*
AST::new_UnaryOp<NotOp>(const Type* Ty, Operator* op1, const Locator* loc);
template<> NotOp*
AST::new_UnaryOp<NotOp>(Operator* op1, Bundle* B, const Locator* loc)
{
  Type* Ty = B->getIntrinsicType(boolTy);
  return AST::new_UnaryOp<NotOp>(Ty,op1,loc);
}

template AndOp*
AST::new_BinaryOp<AndOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> AndOp*
AST::new_BinaryOp<AndOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc)
{
  Type* Ty = B->getIntrinsicType(boolTy);
  return AST::new_BinaryOp<AndOp>(Ty,op1,op2,loc);
}

template OrOp*
AST::new_BinaryOp<OrOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> OrOp*
AST::new_BinaryOp<OrOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc)
{
  Type* Ty = B->getIntrinsicType(boolTy);
  return AST::new_BinaryOp<OrOp>(Ty,op1,op2,loc);
}

template NorOp*
AST::new_BinaryOp<NorOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> NorOp*
AST::new_BinaryOp<NorOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc)
{
  Type* Ty = B->getIntrinsicType(boolTy);
  return AST::new_BinaryOp<NorOp>(Ty,op1,op2,loc);
}

template XorOp*
AST::new_BinaryOp<XorOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> XorOp*
AST::new_BinaryOp<XorOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc)
{
  Type* Ty = B->getIntrinsicType(boolTy);
  return AST::new_BinaryOp<XorOp>(Ty,op1,op2,loc);
}

template LessThanOp*
AST::new_BinaryOp<LessThanOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> LessThanOp*
AST::new_BinaryOp<LessThanOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc)
{
  Type* Ty = B->getIntrinsicType(boolTy);
  return AST::new_BinaryOp<LessThanOp>(Ty,op1,op2,loc);
}

template GreaterThanOp* 
AST::new_BinaryOp<GreaterThanOp>(
    const Type* Ty, Operator* op1, Operator* op2,const Locator* loc);
template<> GreaterThanOp* 
AST::new_BinaryOp<GreaterThanOp>(Operator* op1, Operator* op2,Bundle* B, const Locator* loc)
{
  Type* Ty = B->getIntrinsicType(boolTy);
  return AST::new_BinaryOp<GreaterThanOp>(Ty,op1,op2,loc);
}

template LessEqualOp* 
AST::new_BinaryOp<LessEqualOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> LessEqualOp* 
AST::new_BinaryOp<LessEqualOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc)
{
  Type* Ty = B->getIntrinsicType(boolTy);
  return AST::new_BinaryOp<LessEqualOp>(Ty,op1,op2,loc);
}

template GreaterEqualOp* 
AST::new_BinaryOp<GreaterEqualOp>(
    const Type* Ty, Operator* op1,Operator* op2, const Locator* loc);
template<> GreaterEqualOp* 
AST::new_BinaryOp<GreaterEqualOp>(Operator* op1,Operator* op2, Bundle* B, const Locator* loc)
{
  Type* Ty = B->getIntrinsicType(boolTy);
  return AST::new_BinaryOp<GreaterEqualOp>(Ty,op1,op2,loc);
}

template EqualityOp*
AST::new_BinaryOp<EqualityOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> EqualityOp*
AST::new_BinaryOp<EqualityOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc)
{
  Type* Ty = B->getIntrinsicType(boolTy);
  return AST::new_BinaryOp<EqualityOp>(Ty,op1,op2,loc);
}

template InequalityOp*
AST::new_BinaryOp<InequalityOp>(
    const Type* Ty, Operator* op1,Operator* op2,const Locator* loc);
template<> InequalityOp*
AST::new_BinaryOp<InequalityOp>(Operator* op1,Operator* op2,Bundle* B, const Locator* loc)
{
  Type* Ty = B->getIntrinsicType(boolTy);
  return AST::new_BinaryOp<InequalityOp>(Ty,op1,op2,loc);
}

// String Operators
template StrInsertOp*
AST::new_TernaryOp<StrInsertOp>(const Type* Ty, Operator*op1,Operator*op2,Operator*op3,const Locator* loc);
template StrInsertOp*
AST::new_TernaryOp<StrInsertOp>(Operator*op1,Operator*op2,Operator*op3,Bundle* B,const Locator* loc);

template StrEraseOp*
AST::new_TernaryOp<StrEraseOp>(const Type* Ty, Operator*op1,Operator*op2,Operator*op3,const Locator* loc);
template StrEraseOp*
AST::new_TernaryOp<StrEraseOp>(Operator*op1,Operator*op2,Operator*op3,Bundle* B,const Locator* loc);

template StrReplaceOp* 
AST::new_MultiOp<StrReplaceOp>(
    const Type* Ty, const std::vector<Operator*>& ops, const Locator*loc);
template StrReplaceOp* 
AST::new_MultiOp<StrReplaceOp>(const std::vector<Operator*>& ops, Bundle* B, const Locator*loc);

template StrConcatOp*
AST::new_BinaryOp<StrConcatOp>(const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template StrConcatOp*
AST::new_BinaryOp<StrConcatOp>(Operator* op1, Operator* op2, Bundle* B, const Locator* loc);

// Control Flow Operators
template Block* 
AST::new_MultiOp<Block>(
    const Type* Ty, const std::vector<Operator*>& ops, const Locator*loc);
template Block* 
AST::new_MultiOp<Block>(const std::vector<Operator*>& ops, Bundle* B, const Locator*loc);

template SelectOp*
AST::new_TernaryOp<SelectOp>(
    const Type* Ty, Operator*op1,Operator*op2,Operator*op3,const Locator* loc);
template<> SelectOp*
AST::new_TernaryOp<SelectOp>(Operator*op1,Operator*op2,Operator*op3,Bundle* B,const Locator* loc)
{
  return new_TernaryOp<SelectOp>(op2->getType(),op1,op2,op3,loc);
}

template WhileOp*
AST::new_BinaryOp<WhileOp>(const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> WhileOp*
AST::new_BinaryOp<WhileOp>(Operator* op1, Operator* op2,Bundle* B, const Locator* loc)
{
  return new_BinaryOp<WhileOp>(op2->getType(),op1,op2,loc);
}

template UnlessOp*
AST::new_BinaryOp<UnlessOp>(const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> UnlessOp*
AST::new_BinaryOp<UnlessOp>(Operator* op1, Operator* op2,Bundle* B, const Locator* loc)
{
  return new_BinaryOp<UnlessOp>(op2->getType(),op1,op2,loc);
}

template UntilOp*
AST::new_BinaryOp<UntilOp>(const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template UntilOp*
AST::new_BinaryOp<UntilOp>(Operator* op1, Operator* op2,Bundle* B, const Locator* loc);

template LoopOp*
AST::new_TernaryOp<LoopOp>(const Type* Ty, Operator*op1,Operator*op2,Operator*op3,const Locator* loc);

template<> LoopOp*
AST::new_TernaryOp<LoopOp>(Operator*op1,Operator*op2,Operator*op3,Bundle* B,const Locator* loc)
{
  const Type* Ty = op2->getType();
  if (Ty && llvm::isa<Block>(Ty))
    Ty = llvm::cast<Block>(op2)->getType();
  return new_TernaryOp<LoopOp>(Ty,op1,op2,op3,loc);
}

template SwitchOp*
AST::new_MultiOp<SwitchOp>(
    const Type* Ty, const std::vector<Operator*>& ops, const Locator* loc);
template<> SwitchOp*
AST::new_MultiOp<SwitchOp>(const std::vector<Operator*>& ops, Bundle* B,const Locator* loc)
{
  hlvmAssert(ops.size() >= 2 && "Too few operands for SwitchOp");
  const Type* Ty = ops[1]->getType();
  if (Ty && llvm::isa<Block>(Ty))
    Ty = llvm::cast<Block>(ops[1])->getType();
  return new_MultiOp<SwitchOp>(Ty, ops, loc);
}

template BreakOp* 
AST::new_NilaryOp<BreakOp>(const Type*Ty, const Locator*loc);
template BreakOp* 
AST::new_NilaryOp<BreakOp>(Bundle* B, const Locator*loc);

template ContinueOp* 
AST::new_NilaryOp<ContinueOp>(const Type* Ty, const Locator*loc);
template ContinueOp* 
AST::new_NilaryOp<ContinueOp>(Bundle* B, const Locator*loc);

template ReturnOp* 
AST::new_NilaryOp<ReturnOp>(const Type*Ty, const Locator*loc);
template ReturnOp* 
AST::new_NilaryOp<ReturnOp>(Bundle* B, const Locator*loc);

template ResultOp* 
AST::new_UnaryOp<ResultOp>(const Type*Ty, Operator*op1,const Locator*loc);
template ResultOp* 
AST::new_UnaryOp<ResultOp>(Operator*op1,Bundle* B, const Locator*loc);

template CallOp* 
AST::new_MultiOp<CallOp>(const Type*Ty, const std::vector<Operator*>& ops, const Locator*loc);
template<> CallOp* 
AST::new_MultiOp<CallOp>(const std::vector<Operator*>& ops, Bundle* B,const Locator* loc)
{
  GetOp* get = llvm::cast<GetOp>(ops[0]);
  const Function* F = llvm::cast<Function>(get->getReferent());
  return new_MultiOp<CallOp>(F->getResultType(),ops,loc);
}

// Memory Operators
template StoreOp*  
AST::new_BinaryOp<StoreOp>(const Type*, Operator*op1,Operator*op2,const Locator*loc);
template StoreOp*  
AST::new_BinaryOp<StoreOp>(Operator*op1,Operator*op2,Bundle* B, const Locator*loc);

template LoadOp*   
AST::new_UnaryOp<LoadOp>(const Type* Ty, Operator*op1,const Locator*loc);
template<> LoadOp*   
AST::new_UnaryOp<LoadOp>(Operator*op1,Bundle* B, const Locator*loc)
{
  const PointerType* PT = llvm::cast<PointerType>(op1->getType());
  return new_UnaryOp<LoadOp>(PT->getElementType(),op1,loc);
}

// Input/Output Operators
template OpenOp* 
AST::new_UnaryOp<OpenOp>(const Type* Ty, Operator*op1,const Locator*loc);
template<> OpenOp* 
AST::new_UnaryOp<OpenOp>(Operator*op1,Bundle* B, const Locator*loc)
{
  Type* Ty = B->getIntrinsicType(streamTy);
  return new_UnaryOp<OpenOp>(Ty,op1,loc);
}

template WriteOp* 
AST::new_BinaryOp<WriteOp>(
  const Type* Ty, Operator*op1,Operator*op2, const Locator*loc);
template<> WriteOp* 
AST::new_BinaryOp<WriteOp>(Operator*op1,Operator*op2,Bundle* B, const Locator*loc)
{
  Type* Ty = B->getIntrinsicType(u64Ty);
  return new_BinaryOp<WriteOp>(Ty,op1,op2,loc);
}

template CloseOp* 
AST::new_UnaryOp<CloseOp>(const Type* Ty, Operator*op1,const Locator*loc);
template<> CloseOp* 
AST::new_UnaryOp<CloseOp>(Operator*op1,Bundle* B, const Locator*loc)
{
  return new_UnaryOp<CloseOp>(0,op1,loc);
}

Type* 
AST::new_IntrinsicType(
 const std::string& id,  ///< The name for the new type
 Bundle* bundle,         ///< The bundle to put the new type into
 IntrinsicTypes it,      ///< The type of intrinsic to create
 const Locator* loc      ///< The source locator
) 
{
  Type* result = 0;
  switch (it) 
  {
    case boolTy:   result = new BooleanType(); break;
    case bufferTy: result = new BufferType(); break;
    case charTy:   result = new CharacterType("utf-8"); break;
    case doubleTy: result = new RealType(52,11); break;
    case f32Ty:    result = new RealType(23,8); break;
    case f44Ty:    result = new RealType(32,11); break;
    case f64Ty:    result = new RealType(52,11); break;
    case f80Ty:    result = new RealType(64,15); break;
    case f96Ty:    result = new RealType(112,15); break;
    case f128Ty:   result = new RealType(112,15); break;
    case floatTy:  result = new RealType(23,8); break;
    case intTy:    result = new IntegerType(32,true); break;
    case longTy:   result = new IntegerType(64,true); break;
    case octetTy:  result = new IntegerType(8,false); break;
    case qs16Ty:   result = new RationalType(true,8,8); break;
    case qs32Ty:   result = new RationalType(true,16,16); break;
    case qs64Ty:   result = new RationalType(true,32,32); break;
    case qs128Ty:  result = new RationalType(true,64,64); break;
    case qu16Ty:   result = new RationalType(false,8,8); break;
    case qu32Ty:   result = new RationalType(false,16,16); break;
    case qu64Ty:   result = new RationalType(false,32,32); break;
    case qu128Ty:  result = new RationalType(false,64,64); break;
    case r8Ty:     result = new RangeType(INT8_MIN,INT8_MAX); break;
    case r16Ty:    result = new RangeType(INT16_MIN,INT16_MAX); break;
    case r32Ty:    result = new RangeType(INT32_MIN,INT32_MAX); break;
    case r64Ty:    result = new RangeType(INT64_MIN,INT64_MAX); break;
    case s8Ty:     result = new IntegerType(8,true); break;
    case s16Ty:    result = new IntegerType(16,true); break;
    case s32Ty:    result = new IntegerType(32,true); break;
    case s64Ty:    result = new IntegerType(64,true); break;
    case s128Ty:   result = new IntegerType(128,true); break;
    case textTy:   result = new TextType(); break;
    case shortTy:  result = new IntegerType(16,true); break;
    case streamTy: result = new StreamType(); break;
    case stringTy: result = new StringType("utf-8"); break;
    case u8Ty:     result = new IntegerType(8,false); break;
    case u16Ty:    result = new IntegerType(16,false); break;
    case u32Ty:    result = new IntegerType(32,false); break;
    case u64Ty:    result = new IntegerType(64,false); break;
    case u128Ty:   result = new IntegerType(128,false); break;
    case voidTy:   result = new OpaqueType(id); break;
    default:
      hlvmDeadCode("Invalid Intrinsic");
      break;
  }
  result->setName(id);
  result->setParent(bundle);
  return result;
}

void 
AST::old(Node* to_be_deleted)
{
  delete to_be_deleted;
}

}
