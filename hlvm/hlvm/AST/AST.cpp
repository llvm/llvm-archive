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
      : types(), unresolvedTypes(), 
        AnyTypeSingleton(0), StringTypeSingleton(0),
        BooleanSingleton(), CharacterSingleton(0), 
        OctetSingleton(0), UInt8Singleton(0), UInt16Singleton(0), 
        UInt32Singleton(0), UInt64Singleton(0), UInt128Singleton(0),
        SInt8Singleton(0), SInt16Singleton(0), SInt32Singleton(0),
        SInt64Singleton(0), SInt128Singleton(0),  Float32Singleton(0),
        Float44Singleton(0), Float64Singleton(0), Float80Singleton(0),
        Float128Singleton(0), TextTypeSingleton(0), StreamTypeSingleton(0),
        BufferTypeSingleton(0), ProgramTypeSingleton(0),
        BooleanTrueSingleton(0), BooleanFalseSingleton(0)
      {
        pool = Pool::create("ASTPool",0,false,1024,4,0);
      }
    ~ASTImpl();

  protected:
    virtual void insertChild(Node* child);
    virtual void removeChild(Node* child);

  private:
    // Pool pool;
    SymbolTable<Type>    types;
    SymbolTable<Type>    unresolvedTypes;
    AnyType*       AnyTypeSingleton;
    StringType*    StringTypeSingleton;
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
    ConstantBoolean* BooleanTrueSingleton;
    ConstantBoolean* BooleanFalseSingleton;

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
#ifdef HLVM_ASSERT
  for (const_iterator I = begin(), E = end(); I != E; ++I)
    hlvmAssert((*I) != child && "Attempt to duplicate insertion of child");
#endif
  bundles.push_back(llvm::cast<Bundle>(child));
}

void 
ASTImpl::removeChild(Node* child)
{
  hlvmAssert(llvm::isa<Bundle>(child) && "Can't remove that here");
  //FIXME: bundles.erase(llvm::cast<Bundle>(child));
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
AST::getProgramType()
{
  ASTImpl* ast = const_cast<ASTImpl*>(static_cast<const ASTImpl*>(this));
  if (!ast->ProgramTypeSingleton) {
    ast->ProgramTypeSingleton = new SignatureType();
    ast->ProgramTypeSingleton->setLocator(loc);
    ast->ProgramTypeSingleton->setName("ProgramType");
    Type* intType = ast->getPrimitiveType(SInt32TypeID);
    ast->ProgramTypeSingleton->setResultType(intType);
    Parameter* argc = new_Parameter("argc",intType);
    ast->ProgramTypeSingleton->addParameter(argc);
    const PointerType* argv_type = getPointerTo(getPointerTo(
      ast->getPrimitiveType(StringTypeID)));
    Parameter* argv = new_Parameter("argv",argv_type);
    ast->ProgramTypeSingleton->addParameter(argv);
  }
  return ast->ProgramTypeSingleton;
}

PointerType* 
AST::getPointerTo(const Type* Ty)
{
  hlvmAssert(Ty != 0);
  ASTImpl* ast = const_cast<ASTImpl*>(static_cast<const ASTImpl*>(this));
  std::string ptr_name = Ty->getName() + "*";
  Node* n = ast->types.lookup(ptr_name);
  if (n && llvm::isa<PointerType>(n))
    return llvm::cast<PointerType>(n);

  // Okay, type doesn't exist already, create it a new
  PointerType* PT = new PointerType();
  PT->setElementType(Ty);
  PT->setName(ptr_name);
  ast->types.insert(ptr_name,Ty);
  return PT;
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
  uint16_t bits, 
  bool isSigned,
  const Locator* loc)
{
  IntegerType* result = new IntegerType(IntegerTypeID,bits,isSigned);
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

StringType* 
AST::new_StringType(const std::string& id, const Locator* loc)
{
  StringType* result = new StringType();
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

BufferType* 
AST::new_BufferType(const std::string& id, const Locator* loc)
{
  BufferType* result = new BufferType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

TextType* 
AST::new_TextType( const std::string& id, const Locator* loc)
{
  TextType* result = new TextType();
  result->setLocator(loc);
  result->setName(id);
  static_cast<ASTImpl*>(this)->addType(result);
  return result;
}

StreamType* 
AST::new_StreamType(const std::string& id,  const Locator* loc)
{
  StreamType* result = new StreamType();
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
  const Locator* loc)
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

ConstantAny* 
AST::new_ConstantAny(
  const std::string& name,
  ConstantValue* val,
  const Locator* loc)
{
  ConstantAny* result = new ConstantAny(val);
  result->setLocator(loc);
  result->setName(name);
  result->setType(getPrimitiveType(AnyTypeID));
  return result;
}

ConstantBoolean* 
AST::new_ConstantBoolean(
  const std::string& name,
  bool t_or_f, 
  const Locator* loc)
{
  ConstantBoolean* result = new ConstantBoolean(t_or_f);
  result->setLocator(loc);
  result->setName(name);
  result->setType(getPrimitiveType(BooleanTypeID));
  return result;
}

ConstantCharacter* 
AST::new_ConstantCharacter(
  const std::string& name,
  const std::string& val,
  const Locator* loc)
{
  ConstantCharacter* result = new ConstantCharacter(val);
  result->setLocator(loc);
  result->setName(name);
  result->setType( getPrimitiveType(CharacterTypeID) );
  return result;
}

ConstantEnumerator* 
AST::new_ConstantEnumerator(
  const std::string& name,///< The name of the constant
  const std::string& val, ///< The value for the constant
  const Type* Ty,         ///< The type of the enumerator
  const Locator* loc      ///< The source locator
)
{
  ConstantEnumerator* result = new ConstantEnumerator(val);
  result->setLocator(loc);
  result->setName(name);
  result->setType(Ty);
  return result;
}

ConstantOctet* 
AST::new_ConstantOctet(
  const std::string& name,
  unsigned char val,
  const Locator* loc)
{
  ConstantOctet* result = new ConstantOctet(val);
  result->setLocator(loc);
  result->setName(name);
  result->setType( getPrimitiveType(OctetTypeID) );
  return result;
}

ConstantInteger*
AST::new_ConstantInteger(
  const std::string& name,
  const std::string&  v, 
  uint16_t base, 
  const Type* Ty, 
  const Locator* loc)
{
  ConstantInteger* result = new ConstantInteger(base);
  result->setLocator(loc);
  result->setName(name);
  result->setValue(v);
  result->setBase(base);
  result->setType(Ty);
  return result;
}

ConstantReal*
AST::new_ConstantReal(
  const std::string& name,
  const std::string& v, 
  const Type* Ty, 
  const Locator* loc)
{
  ConstantReal* result = new ConstantReal();
  result->setLocator(loc);
  result->setName(name);
  result->setValue(v);
  result->setType(Ty);
  return result;
}

ConstantString*
AST::new_ConstantString(
  const std::string& name,
  const std::string& v, 
  const Locator* loc)
{
  ConstantString* result = new ConstantString();
  result->setLocator(loc);
  result->setName(name);
  result->setValue(v);
  result->setType( getPrimitiveType(StringTypeID) );
  return result;
}

ConstantPointer* 
AST::new_ConstantPointer(
  const std::string& name,
  ConstantValue* referent,
  const Locator* loc
)
{
  ConstantPointer* result = new ConstantPointer(referent);
  result->setLocator(loc);
  result->setName(name);
  result->setType( getPointerTo(referent->getType()) );
  return result;
}

ConstantArray* 
AST::new_ConstantArray(
  const std::string& name,
  const std::vector<ConstantValue*>& vals,
  const ArrayType* VT,
  const Locator* loc
)
{
  ConstantArray* result = new ConstantArray();
  result->setLocator(loc);
  result->setName(name);
  for (std::vector<ConstantValue*>::const_iterator I = vals.begin(),
       E = vals.end(); I != E; ++I ) 
  {
    hlvmAssert((*I)->getType() == VT->getElementType());
    result->addConstant(*I);
  }
  result->setType(VT);
  return result;
}

ConstantVector* 
AST::new_ConstantVector(
  const std::string& name,
  const std::vector<ConstantValue*>& vals,
  const VectorType* AT,
  const Locator* loc
)
{
  ConstantVector* result = new ConstantVector();
  result->setLocator(loc);
  result->setName(name);
  for (std::vector<ConstantValue*>::const_iterator I = vals.begin(),
       E = vals.end(); I != E; ++I ) 
  {
    hlvmAssert((*I)->getType() == AT->getElementType());
    result->addConstant(*I);
  }
  result->setType(AT);
  return result;
}

ConstantStructure* 
AST::new_ConstantStructure(
  const std::string& name,
  const std::vector<ConstantValue*>& vals,
  const StructureType* ST,
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
  return result;
}

Variable*
AST::new_Variable(const std::string& id, const Type* Ty, const Locator* loc)
{
  Variable* result = new Variable();
  result->setName(id);
  result->setType(Ty);
  result->setLocator(loc);
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
  const std::string& id, const SignatureType* ty, const Locator* loc)
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
    Argument* arg = new_Argument((*I)->getName(),0,loc);
    result->addArgument(arg);
  }
  return result;
}

Program*
AST::new_Program(const std::string& id, const Locator* loc)
{
  SignatureType* ty = getProgramType();
  ASTImpl* ast = static_cast<ASTImpl*>(this);

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
    ConstantValue* op1,
    const Locator* loc)
{
  hlvmAssert(Ty != 0 && "AutoVarOp must have a Type!");
  AutoVarOp* result = new AutoVarOp();
  result->setType(Ty);
  result->setLocator(loc);
  result->setInitializer(op1);
  result->setName(name);
  return result;
}

ReferenceOp* 
AST::new_ReferenceOp(const Value* V, const Locator*loc)
{
  hlvmAssert(V != 0 && "ReferenceOp must have a Value to reference");
  ReferenceOp* result = new ReferenceOp();
  const Type* refType = V->getType();
  if (llvm::isa<ConstantValue>(V) || llvm::isa<Argument>(V)) {
    result->setType(refType);
  } else if (llvm::isa<AutoVarOp>(V) || llvm::isa<Constant>(V)) {
    PointerType* PT = getPointerTo(refType);
    result->setType(PT);
  } else {
    hlvmAssert(!"Invalid referent type");
  }
  result->setLocator(loc);
  result->setReferent(V);
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
  const Locator* loc ///< The source locator
)
{
  return new_NilaryOp<OpClass>(0,loc);
}

/// Provide a template function for creating a unary operator
template<class OpClass> OpClass* 
AST::new_UnaryOp(
  const Type* Ty,    ///< Result type of the operator
  Operator* oprnd1,     ///< The first operand
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
  const Locator* loc ///< The source locator
)
{
  return new_UnaryOp<OpClass>(oprnd1->getType(),oprnd1,loc);
}

/// Provide a template function for creating a binary operator
template<class OpClass> OpClass* 
AST::new_BinaryOp(
  const Type* Ty,    ///< Result type of the operator
  Operator* oprnd1,     ///< The first operand
  Operator* oprnd2,     ///< The second operand
  const Locator* loc ///< The source locator
)
{
  hlvmAssert(oprnd1 != 0 && "Invalid Operand for BinaryOp");
  hlvmAssert(oprnd2 != 0 && "Invalid Operand for BinUnaryOp");
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
  Operator* oprnd1,     ///< The first operand
  Operator* oprnd2,     ///< The second operand
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
AST::new_UnaryOp<NegateOp>(Operator* op1, const Locator* loc);

template ComplementOp*
AST::new_UnaryOp<ComplementOp>(
    const Type* Ty, Operator* op1, const Locator* loc);
template ComplementOp*
AST::new_UnaryOp<ComplementOp>(Operator* op1, const Locator* loc);

template PreIncrOp*
AST::new_UnaryOp<PreIncrOp>(
    const Type* Ty, Operator* op1, const Locator* loc);
template PreIncrOp*
AST::new_UnaryOp<PreIncrOp>(Operator* op1, const Locator* loc);

template PreDecrOp*
AST::new_UnaryOp<PreDecrOp>(
    const Type* Ty, Operator* op1, const Locator* loc);
template PreDecrOp*
AST::new_UnaryOp<PreDecrOp>(Operator* op1, const Locator* loc);

template PostIncrOp*
AST::new_UnaryOp<PostIncrOp>(
    const Type* Ty, Operator* op1, const Locator* loc);
template PostIncrOp*
AST::new_UnaryOp<PostIncrOp>(Operator* op1, const Locator* loc);

template PostDecrOp*
AST::new_UnaryOp<PostDecrOp>(
    const Type* Ty, Operator* op1, const Locator* loc);
template PostDecrOp*
AST::new_UnaryOp<PostDecrOp>(Operator* op1, const Locator* loc);

template AddOp*
AST::new_BinaryOp<AddOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template AddOp*
AST::new_BinaryOp<AddOp>(Operator* op1, Operator* op2, const Locator* loc);

template SubtractOp*
AST::new_BinaryOp<SubtractOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template SubtractOp*
AST::new_BinaryOp<SubtractOp>(Operator* op1, Operator* op2, const Locator* loc);

template MultiplyOp*
AST::new_BinaryOp<MultiplyOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template MultiplyOp*
AST::new_BinaryOp<MultiplyOp>(Operator* op1, Operator* op2, const Locator* loc);

template DivideOp*
AST::new_BinaryOp<DivideOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template DivideOp*
AST::new_BinaryOp<DivideOp>(Operator* op1, Operator* op2, const Locator* loc);

template ModuloOp*
AST::new_BinaryOp<ModuloOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template ModuloOp*
AST::new_BinaryOp<ModuloOp>(Operator* op1, Operator* op2, const Locator* loc);

template BAndOp*
AST::new_BinaryOp<BAndOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template BAndOp*
AST::new_BinaryOp<BAndOp>(Operator* op1, Operator* op2, const Locator* loc);

template BOrOp*
AST::new_BinaryOp<BOrOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template BOrOp*
AST::new_BinaryOp<BOrOp>(Operator* op1, Operator* op2, const Locator* loc);

template BXorOp*
AST::new_BinaryOp<BXorOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template BXorOp*
AST::new_BinaryOp<BXorOp>(Operator* op1, Operator* op2, const Locator* loc);

template BNorOp*
AST::new_BinaryOp<BNorOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template BNorOp*
AST::new_BinaryOp<BNorOp>(Operator* op1, Operator* op2, const Locator* loc);

// Boolean Operators
template NotOp*
AST::new_UnaryOp<NotOp>(const Type* Ty, Operator* op1, const Locator* loc);
template<> NotOp*
AST::new_UnaryOp<NotOp>(Operator* op1, const Locator* loc)
{
  return AST::new_UnaryOp<NotOp>(getPrimitiveType(BooleanTypeID),op1,loc);
}

template AndOp*
AST::new_BinaryOp<AndOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> AndOp*
AST::new_BinaryOp<AndOp>(Operator* op1, Operator* op2, const Locator* loc)
{
  return AST::new_BinaryOp<AndOp>(getPrimitiveType(BooleanTypeID),op1,op2,loc);
}

template OrOp*
AST::new_BinaryOp<OrOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> OrOp*
AST::new_BinaryOp<OrOp>(Operator* op1, Operator* op2, const Locator* loc)
{
  return AST::new_BinaryOp<OrOp>(getPrimitiveType(BooleanTypeID),op1,op2,loc);
}

template NorOp*
AST::new_BinaryOp<NorOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> NorOp*
AST::new_BinaryOp<NorOp>(Operator* op1, Operator* op2, const Locator* loc)
{
  return AST::new_BinaryOp<NorOp>(getPrimitiveType(BooleanTypeID),op1,op2,loc);
}

template XorOp*
AST::new_BinaryOp<XorOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> XorOp*
AST::new_BinaryOp<XorOp>(Operator* op1, Operator* op2, const Locator* loc)
{
  return AST::new_BinaryOp<XorOp>(getPrimitiveType(BooleanTypeID),op1,op2,loc);
}

template LessThanOp*
AST::new_BinaryOp<LessThanOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> LessThanOp*
AST::new_BinaryOp<LessThanOp>(Operator* op1, Operator* op2, const Locator* loc)
{
  return AST::new_BinaryOp<LessThanOp>(getPrimitiveType(BooleanTypeID),op1,op2,loc);
}

template GreaterThanOp* 
AST::new_BinaryOp<GreaterThanOp>(
    const Type* Ty, Operator* op1, Operator* op2,const Locator* loc);
template<> GreaterThanOp* 
AST::new_BinaryOp<GreaterThanOp>(Operator* op1, Operator* op2,const Locator* loc)
{
  return AST::new_BinaryOp<GreaterThanOp>(getPrimitiveType(BooleanTypeID),op1,op2,loc);
}

template LessEqualOp* 
AST::new_BinaryOp<LessEqualOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> LessEqualOp* 
AST::new_BinaryOp<LessEqualOp>(Operator* op1, Operator* op2, const Locator* loc)
{
  return AST::new_BinaryOp<LessEqualOp>(getPrimitiveType(BooleanTypeID),op1,op2,loc);
}

template GreaterEqualOp* 
AST::new_BinaryOp<GreaterEqualOp>(
    const Type* Ty, Operator* op1,Operator* op2, const Locator* loc);
template<> GreaterEqualOp* 
AST::new_BinaryOp<GreaterEqualOp>(Operator* op1,Operator* op2, const Locator* loc)
{
  return AST::new_BinaryOp<GreaterEqualOp>(getPrimitiveType(BooleanTypeID),op1,op2,loc);
}

template EqualityOp*
AST::new_BinaryOp<EqualityOp>(
    const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template<> EqualityOp*
AST::new_BinaryOp<EqualityOp>(Operator* op1, Operator* op2, const Locator* loc)
{
  return AST::new_BinaryOp<EqualityOp>(getPrimitiveType(BooleanTypeID),op1,op2,loc);
}

template InequalityOp*
AST::new_BinaryOp<InequalityOp>(
    const Type* Ty, Operator* op1,Operator* op2,const Locator* loc);
template<> InequalityOp*
AST::new_BinaryOp<InequalityOp>(Operator* op1,Operator* op2,const Locator* loc)
{
  return AST::new_BinaryOp<InequalityOp>(getPrimitiveType(BooleanTypeID),op1,op2,loc);
}


// Control Flow Operators
template Block* 
AST::new_MultiOp<Block>(
    const Type* Ty, const std::vector<Operator*>& ops, const Locator*loc);
template Block* 
AST::new_MultiOp<Block>(const std::vector<Operator*>& ops, const Locator*loc);

template SelectOp*
AST::new_TernaryOp<SelectOp>(
    const Type* Ty, Operator*op1,Operator*op2,Operator*op3,const Locator* loc);
template<> SelectOp*
AST::new_TernaryOp<SelectOp>(Operator*op1,Operator*op2,Operator*op3,const Locator* loc)
{
  return new_TernaryOp<SelectOp>(op2->getType(),op1,op2,op3,loc);
}

template WhileOp*
AST::new_BinaryOp<WhileOp>(const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template WhileOp*
AST::new_BinaryOp<WhileOp>(Operator* op1, Operator* op2,const Locator* loc);

template UnlessOp*
AST::new_BinaryOp<UnlessOp>(const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template UnlessOp*
AST::new_BinaryOp<UnlessOp>(Operator* op1, Operator* op2,const Locator* loc);

template UntilOp*
AST::new_BinaryOp<UntilOp>(const Type* Ty, Operator* op1, Operator* op2, const Locator* loc);
template UntilOp*
AST::new_BinaryOp<UntilOp>(Operator* op1, Operator* op2,const Locator* loc);

template LoopOp*
AST::new_TernaryOp<LoopOp>(const Type* Ty, Operator*op1,Operator*op2,Operator*op3,const Locator* loc);

template<> LoopOp*
AST::new_TernaryOp<LoopOp>(Operator*op1,Operator*op2,Operator*op3,const Locator* loc)
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
AST::new_MultiOp<SwitchOp>(const std::vector<Operator*>& ops, const Locator* loc)
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
AST::new_NilaryOp<BreakOp>(const Locator*loc);

template ContinueOp* 
AST::new_NilaryOp<ContinueOp>(const Type* Ty, const Locator*loc);
template ContinueOp* 
AST::new_NilaryOp<ContinueOp>(const Locator*loc);

template ReturnOp* 
AST::new_NilaryOp<ReturnOp>(const Type*Ty, const Locator*loc);
template ReturnOp* 
AST::new_NilaryOp<ReturnOp>(const Locator*loc);

template ResultOp* 
AST::new_UnaryOp<ResultOp>(const Type*Ty, Operator*op1,const Locator*loc);
template ResultOp* 
AST::new_UnaryOp<ResultOp>(Operator*op1,const Locator*loc);

template CallOp* 
AST::new_MultiOp<CallOp>(const Type*Ty, const std::vector<Operator*>& ops, const Locator*loc);
template CallOp* 
AST::new_MultiOp<CallOp>(const std::vector<Operator*>& ops, const Locator* loc);

// Memory Operators
template StoreOp*  
AST::new_BinaryOp<StoreOp>(const Type*, Operator*op1,Operator*op2,const Locator*loc);
template<> StoreOp*  
AST::new_BinaryOp<StoreOp>(Operator*op1,Operator*op2,const Locator*loc)
{
  return new_BinaryOp<StoreOp>(0,op1,op2,loc);
}

template LoadOp*   
AST::new_UnaryOp<LoadOp>(const Type* Ty, Operator*op1,const Locator*loc);
template<> LoadOp*   
AST::new_UnaryOp<LoadOp>(Operator*op1,const Locator*loc)
{
  hlvmAssert(llvm::isa<PointerType>(op1->getType()) && 
      "LoadOp Requires PointerType operand");
  const Type* Ty = llvm::cast<PointerType>(op1->getType())->getElementType();
  return new_UnaryOp<LoadOp>(Ty, op1, loc);
}

// Input/Output Operators
template OpenOp* 
AST::new_UnaryOp<OpenOp>(const Type* Ty, Operator*op1,const Locator*loc);
template<> OpenOp* 
AST::new_UnaryOp<OpenOp>(Operator*op1,const Locator*loc)
{
  return new_UnaryOp<OpenOp>(getPrimitiveType(StreamTypeID),op1,loc);
}

template WriteOp* 
AST::new_BinaryOp<WriteOp>(
  const Type* Ty, Operator*op1,Operator*op2, const Locator*loc);
template<> WriteOp* 
AST::new_BinaryOp<WriteOp>(Operator*op1,Operator*op2,const Locator*loc)
{
  return new_BinaryOp<WriteOp>(getPrimitiveType(UInt64TypeID),op1,op2,loc);
}

template CloseOp* 
AST::new_UnaryOp<CloseOp>(const Type* Ty, Operator*op1,const Locator*loc);
template<> CloseOp* 
AST::new_UnaryOp<CloseOp>(Operator*op1,const Locator*loc)
{
  return new_UnaryOp<CloseOp>(0,op1,loc);
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
    case StringTypeID:
      if (!ast->StringTypeSingleton) {
        ast->StringTypeSingleton = new StringType();
        ast->StringTypeSingleton->setName("string");
      }
      return ast->StringTypeSingleton;
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
        ast->UInt8Singleton->setName("u8");
      }
      return ast->UInt8Singleton;
    case UInt16TypeID:
      if (!ast->UInt16Singleton) {
        ast->UInt16Singleton = new IntegerType(UInt16TypeID,16,false);
        ast->UInt16Singleton->setName("u16");
      }
      return ast->UInt16Singleton;
    case UInt32TypeID:
      if (!ast->UInt32Singleton) {
        ast->UInt32Singleton = new IntegerType(UInt32TypeID,32,false);
        ast->UInt32Singleton->setName("u32");
      }
      return ast->UInt32Singleton;
    case UInt64TypeID:
      if (!ast->UInt64Singleton) {
        ast->UInt64Singleton = new IntegerType(UInt64TypeID,64,false);
        ast->UInt64Singleton->setName("u64");
      }
      return ast->UInt64Singleton;
    case UInt128TypeID:
      if (!ast->UInt128Singleton) {
        ast->UInt128Singleton = new IntegerType(UInt128TypeID,128,false);
        ast->UInt128Singleton->setName("u128");
      }
      return ast->UInt128Singleton;
    case SInt8TypeID:
      if (!ast->SInt8Singleton) {
        ast->SInt8Singleton = new IntegerType(SInt8TypeID,8,true);
        ast->SInt8Singleton->setName("s8");
      }
      return ast->SInt8Singleton;
    case SInt16TypeID:
      if (!ast->SInt16Singleton) {
        ast->SInt16Singleton = new IntegerType(SInt16TypeID,16,true);
        ast->SInt16Singleton->setName("s16");
      }
      return ast->SInt16Singleton;
    case SInt32TypeID:
      if (!ast->SInt32Singleton) {
        ast->SInt32Singleton = new IntegerType(SInt32TypeID,32,true);
        ast->SInt32Singleton->setName("s32");
      }
      return ast->SInt32Singleton;
    case SInt64TypeID:
      if (!ast->SInt64Singleton) {
        ast->SInt64Singleton = new IntegerType(SInt64TypeID,64,true);
        ast->SInt64Singleton->setName("s64");
      }
      return ast->SInt64Singleton;
    case SInt128TypeID:
      if (!ast->SInt128Singleton) {
        ast->SInt128Singleton = new IntegerType(SInt128TypeID,128,true);
        ast->SInt128Singleton->setName("s128");
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
        ast->TextTypeSingleton->setName("text");
      }
      return ast->TextTypeSingleton;
    case StreamTypeID:
      if (!ast->StreamTypeSingleton) {
        ast->StreamTypeSingleton = new StreamType();
        ast->StreamTypeSingleton->setName("stream");
      }
      return ast->StreamTypeSingleton;
    case BufferTypeID:
      if (!ast->BufferTypeSingleton) {
        ast->BufferTypeSingleton = new BufferType();
        ast->BufferTypeSingleton->setName("buffer");
      }
      return ast->BufferTypeSingleton;
    default:
      hlvmDeadCode("Invalid Primitive");
      break;
  }
  return 0;
}

}
