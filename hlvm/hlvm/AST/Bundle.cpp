//===-- hlvm/AST/Bundle.cpp - AST Bundle Class ------------------*- C++ -*-===//
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
/// @file hlvm/AST/Bundle.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::AST::Bundle.
//===----------------------------------------------------------------------===//

#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/Linkables.h>
#include <hlvm/AST/IntrinsicTypesTokenizer.h>
#include <hlvm/Base/Assert.h>
#include <llvm/Support/Casting.h>

using namespace llvm; 

namespace hlvm {

Bundle::~Bundle() { }

void 
Bundle::insertChild(Node* kid)
{
  hlvmAssert(kid && "Null child!");
  if (Type* ty = dyn_cast<Type>(kid)) {
    if (Type* n = unresolvedTypes.lookup(ty->getName())) {
      OpaqueType* ot = llvm::cast<OpaqueType>(n);
      // FIXME: we should really keep a use list in the type object so this is
      // more efficient, but it will do for now.
      for (tlist_iterator I = tlist_begin(), E = tlist_end(); I != E; ++I) {
        (*I)->resolveTypeTo(ot,ty);
      }
      for (clist_iterator I = clist_begin(), E = clist_end(); I != E; ++I ) {
        (*I)->resolveTypeTo(ot,ty);
      }
      unresolvedTypes.erase(ot);
      // getRoot()->old(ot);
    }
    if (!ttable.lookup(ty->getName())) {
      tlist.push_back(ty);
      ttable.insert(ty);
    }
  } else if (Constant* C = dyn_cast<Constant>(kid)) {
    clist.push_back(C);
    // Constants without names are permitted, but not Linkables
    if (isa<Linkable>(C) || C->hasName())
      ctable.insert(C);
  } else
    hlvmAssert("Don't know how to insert that in a Bundle");
}

void
Bundle::removeChild(Node* kid)
{
  hlvmAssert(kid && "Null child!");
  if (const Type* Ty = dyn_cast<Type>(kid)) {
    for (tlist_iterator I = tlist_begin(), E = tlist_end(); I != E; ++I )
      if (*I == Ty) { tlist.erase(I); break; }
    ttable.erase(Ty->getName());
  } else if (Constant* C = dyn_cast<Constant>(kid)) {
    // This is sucky slow, but we probably won't be removing nodes that much.
    for (clist_iterator I = clist_begin(), E = clist_end(); I != E; ++I )
      if (*I == C) { clist.erase(I); break; }
    ctable.erase(C->getName());
  } else 
    hlvmAssert(!"That node isn't my child");
}

SignatureType* 
Bundle::getProgramType()
{
  Type *type = getType("ProgramType");
  if (!type) {
    AST* ast = getRoot();
    Type* intType = getIntrinsicType(s32Ty);
    SignatureType* sig = 
      ast->new_SignatureType("ProgramType",this,intType,false);
    sig->setIsIntrinsic(true);
    Parameter* argc = ast->new_Parameter("argc",intType);
    PointerType* arg_type = getPointerTo(getIntrinsicType(stringTy));
    arg_type->setIsIntrinsic(true);
    PointerType* argv_type = getPointerTo(arg_type);
    argv_type->setIsIntrinsic(true);
    Parameter* argv = ast->new_Parameter("argv",argv_type);
    sig->addParameter(argc);
    sig->addParameter(argv);
    return sig;
  } else if (SignatureType* sig = llvm::dyn_cast<SignatureType>(type))
    return sig;
  else
    hlvmAssert(!"Invalid use of ProgramType");
}

IntrinsicTypes 
Bundle::getIntrinsicTypesValue(const std::string& name)
{
  int token = HLVM_AST::IntrinsicTypesTokenizer::recognize(name.c_str());
  if (token == HLVM_AST::TKN_NONE)
    return NoIntrinsicType;
  switch (token) {
    case HLVM_AST::TKN_bool:            return boolTy; break;
    case HLVM_AST::TKN_buffer:          return bufferTy; break;
    case HLVM_AST::TKN_char:            return charTy; break;
    case HLVM_AST::TKN_double:          return doubleTy; break;
    case HLVM_AST::TKN_f32:             return f32Ty; break;
    case HLVM_AST::TKN_f44:             return f44Ty; break;
    case HLVM_AST::TKN_f64:             return f64Ty; break;
    case HLVM_AST::TKN_f80:             return f80Ty; break;
    case HLVM_AST::TKN_f96:             return f96Ty; break;
    case HLVM_AST::TKN_f128:            return f128Ty; break;
    case HLVM_AST::TKN_float:           return floatTy; break;
    case HLVM_AST::TKN_int:             return intTy; break;
    case HLVM_AST::TKN_long:            return longTy; break;
    case HLVM_AST::TKN_octet:           return octetTy; break;
    case HLVM_AST::TKN_qs16:            return qs16Ty; break;
    case HLVM_AST::TKN_qs32:            return qs32Ty; break;
    case HLVM_AST::TKN_qs64:            return qs64Ty; break;
    case HLVM_AST::TKN_qs128:           return qs128Ty; break;
    case HLVM_AST::TKN_qu16:            return qu16Ty; break;
    case HLVM_AST::TKN_qu32:            return qu32Ty; break;
    case HLVM_AST::TKN_qu64:            return qu64Ty; break;
    case HLVM_AST::TKN_qu128:           return qu128Ty; break;
    case HLVM_AST::TKN_r8:              return r8Ty; break;
    case HLVM_AST::TKN_r16:             return r16Ty; break;
    case HLVM_AST::TKN_r32:             return r32Ty; break;
    case HLVM_AST::TKN_r64:             return r64Ty; break;
    case HLVM_AST::TKN_s8:              return s8Ty; break;
    case HLVM_AST::TKN_s16:             return s16Ty; break;
    case HLVM_AST::TKN_s32:             return s32Ty; break;
    case HLVM_AST::TKN_s64:             return s64Ty; break;
    case HLVM_AST::TKN_s128:            return s128Ty; break;
    case HLVM_AST::TKN_short:           return shortTy; break;
    case HLVM_AST::TKN_stream:          return streamTy; break;
    case HLVM_AST::TKN_string:          return stringTy; break;
    case HLVM_AST::TKN_text:            return textTy; break;
    case HLVM_AST::TKN_u8:              return u8Ty; break;
    case HLVM_AST::TKN_u16:             return u16Ty; break;
    case HLVM_AST::TKN_u32:             return u32Ty; break;
    case HLVM_AST::TKN_u64:             return u64Ty; break;
    case HLVM_AST::TKN_u128:            return u128Ty; break;
    case HLVM_AST::TKN_void:            return voidTy; break;
    default:  return NoIntrinsicType;
  }
}

void
Bundle::getIntrinsicName(IntrinsicTypes id, std::string& name)
{
  switch (id) 
  {
    case boolTy:            name = "bool" ; break;
    case bufferTy:          name = "buffer"; break;
    case charTy:            name = "char"; break;
    case doubleTy:          name = "double"; break;
    case f32Ty:             name = "f32"; break;
    case f44Ty:             name = "f44" ; break;
    case f64Ty:             name = "f64" ; break;
    case f80Ty:             name = "f80"; break;
    case f96Ty:             name = "f96"; break;
    case f128Ty:            name = "f128"; break;
    case floatTy:           name = "float"; break;
    case intTy:             name = "int"; break;
    case longTy:            name = "long"; break;
    case octetTy:           name = "octet"; break;
    case qs16Ty:            name = "qs16"; break;
    case qs32Ty:            name = "qs32"; break;
    case qs64Ty:            name = "qs64"; break;
    case qs128Ty:           name = "qs128"; break;
    case qu16Ty:            name = "qs16"; break;
    case qu32Ty:            name = "qu32"; break;
    case qu64Ty:            name = "qu64"; break;
    case qu128Ty:           name = "qs128"; break;
    case r8Ty:              name = "r8"; break;
    case r16Ty:             name = "r16"; break;
    case r32Ty:             name = "r32"; break;
    case r64Ty:             name = "r64"; break;
    case s8Ty:              name = "s8"; break;
    case s16Ty:             name = "s16"; break;
    case s32Ty:             name = "s32"; break;
    case s64Ty:             name = "s64"; break;
    case s128Ty:            name = "s128"; break;
    case shortTy:           name = "short"; break;
    case streamTy:          name = "stream"; break;
    case stringTy:          name = "string"; break;
    case textTy:            name = "text"; break;
    case u8Ty:              name = "u8"; break;
    case u16Ty:             name = "u16"; break;
    case u32Ty:             name = "u32"; break;
    case u64Ty:             name = "u64"; break;
    case u128Ty:            name = "u128"; break;
    case voidTy:            name = "void"; break;
    default:
      hlvmDeadCode("Invalid Primitive");
      name = "unknown";
      break;
  }
}

Type* 
Bundle::getIntrinsicType(IntrinsicTypes id)
{
  std::string name;
  getIntrinsicName(id, name);
  Type* Ty = getType(name);
  if (!Ty) {
    Ty = getRoot()->new_IntrinsicType(name, this, id);
    Ty->setIsIntrinsic(true);
  }
  return Ty;
}

Type*
Bundle::getIntrinsicType(const std::string& name)
{
  IntrinsicTypes it = getIntrinsicTypesValue(name);
  if (it == NoIntrinsicType)
    return 0;
  return getIntrinsicType(it);
}

PointerType* 
Bundle::getPointerTo(const Type* Ty)
{
  hlvmAssert(Ty != 0);
  std::string ptr_name = Ty->getName() + "*";
  Type* t = getType(ptr_name);
  if (!t || !llvm::isa<PointerType>(t))
    t = getRoot()->new_PointerType(ptr_name,this,Ty);
  return llvm::cast<PointerType>(t);
}

Type*
Bundle::getType(const std::string& name)
{
  if (Type* t = ttable.lookup(name))
    return t;
  if (Type* t = unresolvedTypes.lookup(name))
    return t;
  return 0;
}

Type*
Bundle::getOrCreateType(const std::string& name)
{
  Type* t = getType(name);
  if (!t) {
    t = getRoot()->new_OpaqueType(name,true,this);
    unresolvedTypes.insert(t);
  }
  return t;
}

Constant*  
Bundle::getConst(const std::string& name) const
{
  if (Constant* result = ctable.lookup(name))
    return llvm::cast<Constant>(result);
  return 0;
}

Import::~Import()
{
}

}
