//===-- LLVM Code Emitter Implementation ------------------------*- C++ -*-===//
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
/// @file hlvm/CodeGen/LLVMEmitter.cpp
/// @author Reid Spencer <rspencer@x10sys.com>
/// @date 2006/07/08
/// @since 0.2.0
/// @brief Provides the implementation of the LLVM Code Emitter
//===----------------------------------------------------------------------===//

#include <hlvm/CodeGen/LLVMEmitter.h>
#include <hlvm/Base/Assert.h>

using namespace llvm;

namespace {

class LLVMEmitterImpl : public hlvm::LLVMEmitter
{
  /// @name Data
  /// @{
  private:
    // Caches of things to interface with the HLVM Runtime Library
    PointerType*  hlvm_text;          ///< Opaque type for text objects
    Function*     hlvm_text_create;   ///< Create a new text object
    Function*     hlvm_text_delete;   ///< Delete a text object
    Function*     hlvm_text_to_buffer;///< Convert text to a buffer
    PointerType*  hlvm_buffer;        ///< Pointer To octet
    Function*     hlvm_buffer_create; ///< Create a new buffer object
    Function*     hlvm_buffer_delete; ///< Delete a buffer
    PointerType*  hlvm_stream;        ///< Pointer to stream type
    Function*     hlvm_stream_open;   ///< Function for stream_open
    Function*     hlvm_stream_read;   ///< Function for stream_read
    Function*     hlvm_stream_write_buffer; ///< Write buffer to stream
    Function*     hlvm_stream_write_text;   ///< Write text to stream
    Function*     hlvm_stream_write_string; ///< Write string to stream
    Function*     hlvm_stream_close;  ///< Function for stream_close
    Function*     hlvm_f32_ispinf;
    Function*     hlvm_f32_isninf ;
    Function*     hlvm_f32_isnan ;
    Function*     hlvm_f32_trunc ;
    Function*     hlvm_f32_round ;
    Function*     hlvm_f32_floor ;
    Function*     hlvm_f32_ceiling ;
    Function*     hlvm_f32_loge ;
    Function*     hlvm_f32_log2 ;
    Function*     hlvm_f32_log10 ;
    Function*     hlvm_f32_squareroot ;
    Function*     hlvm_f32_cuberoot ;
    Function*     hlvm_f32_factorial ;
    Function*     hlvm_f32_power ;
    Function*     hlvm_f32_root ;
    Function*     hlvm_f32_gcd ;
    Function*     hlvm_f32_lcm;
    Function*     hlvm_f64_ispinf;
    Function*     hlvm_f64_isninf ;
    Function*     hlvm_f64_isnan ;
    Function*     hlvm_f64_trunc ;
    Function*     hlvm_f64_round ;
    Function*     hlvm_f64_floor ;
    Function*     hlvm_f64_ceiling ;
    Function*     hlvm_f64_loge ;
    Function*     hlvm_f64_log2 ;
    Function*     hlvm_f64_log10 ;
    Function*     hlvm_f64_squareroot ;
    Function*     hlvm_f64_cuberoot ;
    Function*     hlvm_f64_factorial ;
    Function*     hlvm_f64_power ;
    Function*     hlvm_f64_root ;
    Function*     hlvm_f64_gcd ;
    Function*     hlvm_f64_lcm;

    FunctionType* hlvm_program_signature; ///< The llvm type for programs

    // Caches of LLVM Intrinsic functions
    Function*     llvm_memcpy;         ///< llvm.memcpy.i64
    Function*     llvm_memmove;        ///< llvm.memmove.i64
    Function*     llvm_memset;         ///< llvm.memset.i64

  /// @}
  /// @name Constructor
  /// @{
  public:
    LLVMEmitterImpl() : hlvm::LLVMEmitter(), 
      hlvm_text(0), hlvm_text_create(0), hlvm_text_delete(0),
      hlvm_text_to_buffer(0), 
      hlvm_buffer(0), hlvm_buffer_create(0), hlvm_buffer_delete(0),
      hlvm_stream(0), hlvm_stream_open(0), hlvm_stream_read(0),
      hlvm_stream_write_buffer(0), hlvm_stream_write_text(0), 
      hlvm_stream_write_string(0), hlvm_stream_close(0), 
      hlvm_f32_ispinf(0), hlvm_f32_isninf(0), hlvm_f32_isnan(0),
      hlvm_f32_trunc(0), hlvm_f32_round(0), hlvm_f32_floor (0),
      hlvm_f32_ceiling(0), hlvm_f32_loge(0), hlvm_f32_log2(0),
      hlvm_f32_log10(0), hlvm_f32_squareroot(0), hlvm_f32_cuberoot(0),
      hlvm_f32_factorial(0), hlvm_f32_power(0), hlvm_f32_root(0),
      hlvm_f32_gcd(0), hlvm_f32_lcm(0), hlvm_f64_ispinf(0), hlvm_f64_isninf(0),
      hlvm_f64_isnan(0), hlvm_f64_trunc(0), hlvm_f64_round(0),
      hlvm_f64_floor(0), hlvm_f64_ceiling(0), hlvm_f64_loge(0),
      hlvm_f64_log2(0), hlvm_f64_log10(0), hlvm_f64_squareroot(0),
      hlvm_f64_cuberoot(0), hlvm_f64_factorial(0), hlvm_f64_power(0),
      hlvm_f64_root(0), hlvm_f64_gcd(0), hlvm_f64_lcm(0),
      hlvm_program_signature(0),
      llvm_memcpy(0), llvm_memmove(0), llvm_memset(0)
    { 
    }

  /// @}
  /// @name Methods
  /// @{
  public:
    const Type* get_hlvm_size() { return Type::Int64Ty; }

    PointerType* get_hlvm_text()
    {
      if (! hlvm_text) {
        // An hlvm_text is a variable length array of signed bytes preceded by
        // an
        // Arglist args;
        // args.push_back(Type::Int32Ty);
        // args.push_back(ArrayType::Get(Type::Int8Ty,0));
        OpaqueType* opq = OpaqueType::get();
        TheModule->addTypeName("hlvm_text_obj", opq);
        hlvm_text = PointerType::get(opq);
        TheModule->addTypeName("hlvm_text", hlvm_text);
      }
      return hlvm_text;
    }

    Function* get_hlvm_text_create()
    {
      if (! hlvm_text_create) {
        Type* result = get_hlvm_text();
        std::vector<const Type*> arg_types;
        arg_types.push_back(PointerType::get(Type::Int8Ty));
        FunctionType* FT = 
          FunctionType::get(result,arg_types,false);
        TheModule->addTypeName("hlvm_text_create",FT);
        hlvm_text_create = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_text_create", TheModule);
      }
      return hlvm_text_create;
    }

    CallInst* call_hlvm_text_create(const hlvm::ArgList& args, const char* nm)
    {
      Function* F = get_hlvm_text_create();
      return new CallInst(F, args, (nm ? nm : "buffer"), TheBlock);
    }

    Function* get_hlvm_text_delete()
    {
      if (! hlvm_text_delete) {
        Type* result = get_hlvm_text();
        std::vector<const Type*> arg_types;
        arg_types.push_back(get_hlvm_text());
        FunctionType* FT = 
          FunctionType::get(result,arg_types,false);
        TheModule->addTypeName("hlvm_text_delete",FT);
        hlvm_text_delete = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_text_delete", TheModule);
      }
      return hlvm_text_delete;
    }

    CallInst* call_hlvm_text_delete(const hlvm::ArgList& args)
    {
      Function* F = get_hlvm_text_delete();
      return new CallInst(F, args, "hlvm_text_delete", TheBlock);
    }

    Function* get_hlvm_text_to_buffer()
    {
      if (! hlvm_text_to_buffer) {
        Type* result = get_hlvm_buffer();
        std::vector<const Type*> arg_types;
        arg_types.push_back(get_hlvm_text());
        FunctionType* FT = FunctionType::get(result,arg_types,false);
        TheModule->addTypeName("hlvm_text_to_buffer_signature",FT);
        hlvm_text_to_buffer = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_text_to_buffer", TheModule);
      }
      return hlvm_text_to_buffer;
    }

    CallInst* call_hlvm_text_to_buffer(const hlvm::ArgList& args, const char* nm)
    {
      Function* F = get_hlvm_text_to_buffer();
      return new CallInst(F, args, (nm ? nm : "buffer"), TheBlock);
    }

    PointerType* get_hlvm_buffer()
    {
      if (! hlvm_buffer) {
        OpaqueType* opq = OpaqueType::get();
        TheModule->addTypeName("hlvm_buffer_obj", opq);
        hlvm_buffer = PointerType::get(opq);
        TheModule->addTypeName("hlvm_buffer", hlvm_buffer);
      }
      return hlvm_buffer;
    }

    Function* get_hlvm_buffer_create()
    {
      if (! hlvm_buffer_create) {
        Type* result = get_hlvm_buffer();
        std::vector<const Type*> arg_types;
        arg_types.push_back(get_hlvm_size());
        FunctionType* FT = FunctionType::get(result,arg_types,false);
        TheModule->addTypeName("hlvm_buffer_create",FT);
        hlvm_buffer_create = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_buffer_create", TheModule);
      }
      return hlvm_buffer_create;
    }

    CallInst* call_hlvm_buffer_create(const hlvm::ArgList& args, const char* nm)
    {
      Function* F = get_hlvm_buffer_create();
      return new CallInst(F, args, (nm ? nm : "buffer"), TheBlock);
    }

    Function* get_hlvm_buffer_delete()
    {
      if (! hlvm_buffer_delete) {
        Type* result = get_hlvm_buffer();
        std::vector<const Type*> arg_types;
        arg_types.push_back(get_hlvm_buffer());
        FunctionType* FT = FunctionType::get(result,arg_types,false);
        TheModule->addTypeName("hlvm_buffer_delete",FT);
        hlvm_buffer_delete = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_buffer_delete", TheModule);
      }
      return hlvm_buffer_delete;
    }

    CallInst* call_hlvm_buffer_delete(const hlvm::ArgList& args)
    {
      Function* F = get_hlvm_buffer_delete();
      return new CallInst(F, args, "", TheBlock);
    }

    PointerType* get_hlvm_stream()
    {
      if (! hlvm_stream) {
        OpaqueType* opq = OpaqueType::get();
        TheModule->addTypeName("hlvm_stream_obj", opq);
        hlvm_stream= PointerType::get(opq);
        TheModule->addTypeName("hlvm_stream", hlvm_stream);
      }
      return hlvm_stream;
    }

    Function* get_hlvm_stream_open()
    {
      if (!hlvm_stream_open) {
        Type* result = get_hlvm_stream();
        std::vector<const Type*> arg_types;
        arg_types.push_back(PointerType::get(Type::Int8Ty));
        FunctionType* FT = FunctionType::get(result,arg_types,false);
        TheModule->addTypeName("hlvm_stream_open_signature",FT);
        hlvm_stream_open = 
          new Function(FT, GlobalValue::ExternalLinkage, 
            "hlvm_stream_open", TheModule);
      }
      return hlvm_stream_open;
    }

    CallInst* call_hlvm_stream_open(const hlvm::ArgList& args, const char* nm)
    {
      Function* F = get_hlvm_stream_open();
      return new CallInst(F, args, (nm ? nm : "stream"), TheBlock);
    }

    Function* get_hlvm_stream_read()
    {
      if (!hlvm_stream_read) {
        const Type* result = get_hlvm_size();
        std::vector<const Type*> arg_types;
        arg_types.push_back(get_hlvm_stream());
        arg_types.push_back(get_hlvm_buffer());
        arg_types.push_back(get_hlvm_size());
        FunctionType* FT = FunctionType::get(result,arg_types,false);
        TheModule->addTypeName("hlvm_stream_read_signature",FT);
        hlvm_stream_read = 
          new Function(FT, GlobalValue::ExternalLinkage,
          "hlvm_stream_read", TheModule);
      }
      return hlvm_stream_read;
    }

    CallInst* call_hlvm_stream_read(const hlvm::ArgList& args, const char* nm)
    {
      Function* F = get_hlvm_stream_read();
      return new CallInst(F, args, (nm ? nm : "readlen"), TheBlock);
    }

    Function* get_hlvm_stream_write_buffer()
    {
      if (!hlvm_stream_write_buffer) {
        const Type* result = get_hlvm_size();
        std::vector<const Type*> arg_types;
        arg_types.push_back(get_hlvm_stream());
        arg_types.push_back(get_hlvm_buffer());
        FunctionType* FT = FunctionType::get(result,arg_types,false);
        TheModule->addTypeName("hlvm_stream_write_buffer_signature",FT);
        hlvm_stream_write_buffer = 
          new Function(FT, GlobalValue::ExternalLinkage,
          "hlvm_stream_write_buffer", TheModule);
      }
      return hlvm_stream_write_buffer;
    }

    CallInst* call_hlvm_stream_write_buffer(const hlvm::ArgList& args, const char* nm)
    {
      Function* F = get_hlvm_stream_write_buffer();
      return new CallInst(F, args, (nm ? nm : "writelen"), TheBlock);
    }

    Function* get_hlvm_stream_write_string()
    {
      if (!hlvm_stream_write_string) {
        const Type* result = get_hlvm_size();
        std::vector<const Type*> arg_types;
        arg_types.push_back(get_hlvm_stream());
        arg_types.push_back(PointerType::get(Type::Int8Ty));
        FunctionType* FT = FunctionType::get(result,arg_types,false);
        TheModule->addTypeName("hlvm_stream_write_string_signature",FT);
        hlvm_stream_write_string = 
          new Function(FT, GlobalValue::ExternalLinkage,
          "hlvm_stream_write_string", TheModule);
      }
      return hlvm_stream_write_string;
    }

    CallInst* call_hlvm_stream_write_string(const hlvm::ArgList& args, const char* nm)
    {
      Function* F = get_hlvm_stream_write_string();
      return new CallInst(F, args, (nm ? nm : "writelen"), TheBlock);
    }

    Function* get_hlvm_stream_write_text()
    {
      if (!hlvm_stream_write_text) {
        const Type* result = get_hlvm_size();
        std::vector<const Type*> arg_types;
        arg_types.push_back(get_hlvm_stream());
        arg_types.push_back(get_hlvm_text());
        FunctionType* FT = FunctionType::get(result,arg_types,false);
        TheModule->addTypeName("hlvm_stream_write_text_signature",FT);
        hlvm_stream_write_text = 
          new Function(FT, GlobalValue::ExternalLinkage,
          "hlvm_stream_write_text", TheModule);
      }
      return hlvm_stream_write_text;
    }

    CallInst* call_hlvm_stream_write_text(const hlvm::ArgList& args, const char* nm)
    {
      Function* F = get_hlvm_stream_write_text();
      return new CallInst(F, args, (nm ? nm : "writelen"), TheBlock);
    }

    Function* get_hlvm_stream_close()
    {
      if (!hlvm_stream_close) {
        const Type* result = Type::VoidTy;
        std::vector<const Type*> arg_types;
        arg_types.push_back(get_hlvm_stream());
        FunctionType* FT = FunctionType::get(result,arg_types,false);
        TheModule->addTypeName("hlvm_stream_close_signature",FT);
        hlvm_stream_close = 
          new Function(FT, GlobalValue::ExternalLinkage,
          "hlvm_stream_close", TheModule);
      }
      return hlvm_stream_close;
    }

    CallInst* call_hlvm_stream_close(const hlvm::ArgList& args)
    {
      Function* F = get_hlvm_stream_close();
      return new CallInst(F, args, "", TheBlock);
    }

    FunctionType* get_hlvm_program_signature()
    {
      if (!hlvm_program_signature) {
        // Get the type of function that all entry points must have
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::Int32Ty);
        arg_types.push_back(
          PointerType::get(PointerType::get(Type::Int8Ty)));
        hlvm_program_signature = 
          FunctionType::get(Type::Int32Ty,arg_types,false);
        TheModule->addTypeName("hlvm_program_signature",hlvm_program_signature);
      }
      return hlvm_program_signature;
    }

    Function* get_llvm_memcpy()
    {
      if (!llvm_memcpy) {
        const Type *SBP = PointerType::get(Type::Int8Ty);
        llvm_memcpy = cast<Function>(TheModule->getOrInsertFunction(
          "llvm.memcpy.i64", Type::VoidTy, SBP, SBP, Type::Int64Ty,
          Type::Int32Ty, NULL));
      }
      return llvm_memcpy;
    }

    Function* get_llvm_memmove()
    {
      if (!llvm_memmove) {
        const Type *SBP = PointerType::get(Type::Int8Ty);
        llvm_memmove = cast<Function>(TheModule->getOrInsertFunction(
          "llvm.memmove.i64", Type::VoidTy, SBP, SBP, Type::Int64Ty, 
          Type::Int32Ty, NULL));
      }
      return llvm_memmove;
    }

    Function* get_llvm_memset()
    {
      if (!llvm_memset) {
        const Type *SBP = PointerType::get(Type::Int8Ty);
        llvm_memset = cast<Function>(TheModule->getOrInsertFunction(
          "llvm.memset.i64", Type::VoidTy, SBP, Type::Int8Ty, Type::Int64Ty, 
          Type::Int32Ty, NULL));
      }
      return llvm_memset;
    }

    Function* get_hlvm_f32_ispinf()
    {
      if (! hlvm_f32_ispinf) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_ispinf",FT);
        hlvm_f32_ispinf = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_ispinf", TheModule);
      }
      return hlvm_f32_ispinf;
    }

    Function* get_hlvm_f32_isninf()
    {
      if (! hlvm_f32_isninf) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_isninf",FT);
        hlvm_f32_isninf = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_isninf", TheModule);
      }
      return hlvm_f32_isninf;
    }

    Function* get_hlvm_f32_isnan()
    {
      if (! hlvm_f32_isnan) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_isnan",FT);
        hlvm_f32_isnan = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_isnan", TheModule);
      }
      return hlvm_f32_isnan;
    }

    Function* get_hlvm_f32_trunc()
    {
      if (! hlvm_f32_trunc) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_trunc",FT);
        hlvm_f32_trunc = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_trunc", TheModule);
      }
      return hlvm_f32_trunc;
    }

    Function* get_hlvm_f32_round()
    {
      if (! hlvm_f32_round) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_round",FT);
        hlvm_f32_round = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_round", TheModule);
      }
      return hlvm_f32_round;
    }

    Function* get_hlvm_f32_floor()
    {
      if (! hlvm_f32_floor) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_floor",FT);
        hlvm_f32_floor = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_floor", TheModule);
      }
      return hlvm_f32_floor;
    }

    Function* get_hlvm_f32_ceiling()
    {
      if (! hlvm_f32_ceiling) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_ceiling",FT);
        hlvm_f32_ceiling = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_ceiling", TheModule);
      }
      return hlvm_f32_ceiling;
    }

    Function* get_hlvm_f32_loge()
    {
      if (! hlvm_f32_loge) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_loge",FT);
        hlvm_f32_loge = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_loge", TheModule);
      }
      return hlvm_f32_loge;
    }

    Function* get_hlvm_f32_log2()
    {
      if (! hlvm_f32_log2) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_log2",FT);
        hlvm_f32_log2 = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_log2", TheModule);
      }
      return hlvm_f32_log2;
    }

    Function* get_hlvm_f32_log10()
    {
      if (! hlvm_f32_log10) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_log10",FT);
        hlvm_f32_log10 = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_log10", TheModule);
      }
      return hlvm_f32_log10;
    }

    Function* get_hlvm_f32_squareroot()
    {
      if (! hlvm_f32_squareroot) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_squareroot",FT);
        hlvm_f32_squareroot = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_squareroot", TheModule);
      }
      return hlvm_f32_squareroot;
    }

    Function* get_hlvm_f32_cuberoot()
    {
      if (! hlvm_f32_cuberoot) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_cuberoot",FT);
        hlvm_f32_cuberoot = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_cuberoot", TheModule);
      }
      return hlvm_f32_cuberoot;
    }

    Function* get_hlvm_f32_factorial()
    {
      if (! hlvm_f32_factorial) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_factorial",FT);
        hlvm_f32_factorial = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_factorial", TheModule);
      }
      return hlvm_f32_factorial;
    }

    Function* get_hlvm_f32_power()
    {
      if (! hlvm_f32_power) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_power",FT);
        hlvm_f32_power = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_power", TheModule);
      }
      return hlvm_f32_ispinf;
    }

    Function* get_hlvm_f32_root()
    {
      if (! hlvm_f32_root) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_root",FT);
        hlvm_f32_root = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_root", TheModule);
      }
      return hlvm_f32_ispinf;
    }

    Function* get_hlvm_f32_gcd()
    {
      if (! hlvm_f32_gcd) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_gcd",FT);
        hlvm_f32_gcd = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_gcd", TheModule);
      }
      return hlvm_f32_ispinf;
    }

    Function* get_hlvm_f32_lcm()
    {
      if (! hlvm_f32_lcm) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::FloatTy);
        arg_types.push_back(Type::FloatTy);
        FunctionType* FT = FunctionType::get(Type::FloatTy,arg_types,false);
        TheModule->addTypeName("hlvm_f32_lcm",FT);
        hlvm_f32_lcm = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f32_lcm", TheModule);
      }
      return hlvm_f32_ispinf;
    }

    CallInst* call_hlvm_f32_ispinf(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_ispinf(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_isninf(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_isninf(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_isnan(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_isnan(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_trunc(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_trunc(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_round(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_round(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_floor(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_floor(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_ceiling(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_ceiling(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_loge(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_loge(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_log2(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_log2(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_log10(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_log10(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_squareroot(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_squareroot(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_cuberoot(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_cuberoot(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_factorial(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_factorial(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_power(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_power(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_root(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_root(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_gcd(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_gcd(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f32_lcm(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f32_lcm(), args, "", TheBlock);
    }

    Function* get_hlvm_f64_ispinf()
    {
      if (! hlvm_f64_ispinf) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_ispinf",FT);
        hlvm_f64_ispinf = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_ispinf", TheModule);
      }
      return hlvm_f64_ispinf;
    }

    Function* get_hlvm_f64_isninf()
    {
      if (! hlvm_f64_isninf) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_isninf",FT);
        hlvm_f64_isninf = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_isninf", TheModule);
      }
      return hlvm_f64_isninf;
    }

    Function* get_hlvm_f64_isnan()
    {
      if (! hlvm_f64_isnan) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_isnan",FT);
        hlvm_f64_isnan = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_isnan", TheModule);
      }
      return hlvm_f64_isnan;
    }

    Function* get_hlvm_f64_trunc()
    {
      if (! hlvm_f64_trunc) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_trunc",FT);
        hlvm_f64_trunc = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_trunc", TheModule);
      }
      return hlvm_f64_trunc;
    }

    Function* get_hlvm_f64_round()
    {
      if (! hlvm_f64_round) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_round",FT);
        hlvm_f64_round = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_round", TheModule);
      }
      return hlvm_f64_round;
    }

    Function* get_hlvm_f64_floor()
    {
      if (! hlvm_f64_floor) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_floor",FT);
        hlvm_f64_floor = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_floor", TheModule);
      }
      return hlvm_f64_floor;
    }

    Function* get_hlvm_f64_ceiling()
    {
      if (! hlvm_f64_ceiling) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_ceiling",FT);
        hlvm_f64_ceiling = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_ceiling", TheModule);
      }
      return hlvm_f64_ceiling;
    }

    Function* get_hlvm_f64_loge()
    {
      if (! hlvm_f64_loge) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_loge",FT);
        hlvm_f64_loge = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_loge", TheModule);
      }
      return hlvm_f64_loge;
    }

    Function* get_hlvm_f64_log2()
    {
      if (! hlvm_f64_log2) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_log2",FT);
        hlvm_f64_log2 = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_log2", TheModule);
      }
      return hlvm_f64_log2;
    }

    Function* get_hlvm_f64_log10()
    {
      if (! hlvm_f64_log10) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_log10",FT);
        hlvm_f64_log10 = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_log10", TheModule);
      }
      return hlvm_f64_log10;
    }

    Function* get_hlvm_f64_squareroot()
    {
      if (! hlvm_f64_squareroot) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_squareroot",FT);
        hlvm_f64_squareroot = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_squareroot", TheModule);
      }
      return hlvm_f64_squareroot;
    }

    Function* get_hlvm_f64_cuberoot()
    {
      if (! hlvm_f64_cuberoot) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_cuberoot",FT);
        hlvm_f64_cuberoot = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_cuberoot", TheModule);
      }
      return hlvm_f64_cuberoot;
    }

    Function* get_hlvm_f64_factorial()
    {
      if (! hlvm_f64_factorial) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_factorial",FT);
        hlvm_f64_factorial = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_factorial", TheModule);
      }
      return hlvm_f64_factorial;
    }

    Function* get_hlvm_f64_power()
    {
      if (! hlvm_f64_power) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_power",FT);
        hlvm_f64_power = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_power", TheModule);
      }
      return hlvm_f64_ispinf;
    }

    Function* get_hlvm_f64_root()
    {
      if (! hlvm_f64_root) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_root",FT);
        hlvm_f64_root = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_root", TheModule);
      }
      return hlvm_f64_ispinf;
    }

    Function* get_hlvm_f64_gcd()
    {
      if (! hlvm_f64_gcd) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_gcd",FT);
        hlvm_f64_gcd = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_gcd", TheModule);
      }
      return hlvm_f64_ispinf;
    }

    Function* get_hlvm_f64_lcm()
    {
      if (! hlvm_f64_lcm) {
        std::vector<const Type*> arg_types;
        arg_types.push_back(Type::DoubleTy);
        arg_types.push_back(Type::DoubleTy);
        FunctionType* FT = FunctionType::get(Type::DoubleTy,arg_types,false);
        TheModule->addTypeName("hlvm_f64_lcm",FT);
        hlvm_f64_lcm = 
          new Function(FT, GlobalValue::ExternalLinkage, 
          "hlvm_f64_lcm", TheModule);
      }
      return hlvm_f64_ispinf;
    }

    CallInst* call_hlvm_f64_ispinf(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_ispinf(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_isninf(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_isninf(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_isnan(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_isnan(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_trunc(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_trunc(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_round(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_round(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_floor(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_floor(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_ceiling(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_ceiling(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_loge(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_loge(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_log2(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_log2(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_log10(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_log10(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_squareroot(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_squareroot(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_cuberoot(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_cuberoot(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_factorial(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_factorial(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_power(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_power(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_root(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_root(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_gcd(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_gcd(), args, "", TheBlock);
    }

    CallInst* call_hlvm_f64_lcm(const hlvm::ArgList& args)
    {
      return new CallInst(get_hlvm_f64_lcm(), args, "", TheBlock);
    }

  /// @}
};

}

namespace hlvm {

LLVMEmitter::LLVMEmitter()
  : TheModule(0), TheFunction(0), TheEntryBlock(0), TheExitBlock(0), 
    EntryInsertionPoint(0), TheBlock(0)
{
}

LLVMEmitter* 
new_LLVMEmitter()
{
  return new LLVMEmitterImpl();
}

Module*
LLVMEmitter::StartModule(const std::string& ID)
{
  hlvmAssert(TheModule == 0);
  return TheModule = new Module(ID);
}

Module*
LLVMEmitter::FinishModule()
{
  hlvmAssert(TheModule != 0);
  Module* result = TheModule;
  TheModule = 0;
  return result;
}

void
LLVMEmitter::StartFunction(Function* F)
{
  hlvmAssert(F != 0 && "Null function?");
  hlvmAssert(F->empty() && "Function already emitted!");

  // Ensure previous state is cleared
  // operands.clear();
  // enters.clear();
  // exits.clear();
  blocks.clear();
  breaks.clear();
  continues.clear();
  // lvars.clear();

  // Set up for new function
  TheFunction = F;

  // Instantiate an entry block for the alloca'd variables. This block
  // is only used for such variables. By placing the alloca'd variables in
  // the entry block, their allocation is free since the stack pointer 
  // must be adjusted anyway, all that happens is that it gets adjusted
  // by a larger amount. 
  TheBlock = TheEntryBlock = new BasicBlock("entry",F);

  // Instantiate a no-op as an insertion point for the entry point stuff such
  // as the alloca variables and argument setup. This allows us to fill and
  // terminate the entry block as usual while still retaining a point for 
  // insertion in the entry block that retains declaration order.
  EntryInsertionPoint = 
    new BitCastInst(Constant::getNullValue(Type::Int32Ty),Type::Int32Ty,
        "entry_point", TheEntryBlock);

  // Create a new block for the return node, but don't insert it yet.
  // TheExitBlock = new BasicBlock("exit");
}

void
LLVMEmitter::FinishFunction()
{
  // The entry block was created to hold the automatic variables. We now
  // need to terminate the block by branching it to the first active block
  // in the function.
  new BranchInst(TheFunction->front().getNext(),&TheFunction->front());
  hlvmAssert(blocks.empty());
  hlvmAssert(breaks.empty());
  hlvmAssert(continues.empty());
}

BasicBlock*
LLVMEmitter::pushBlock(const std::string& name)
{
  TheBlock = new BasicBlock(name,TheFunction);
  blocks.push_back(TheBlock);
  return TheBlock;
}

BasicBlock* 
LLVMEmitter::popBlock()
{
  BasicBlock* result = blocks.back();
  blocks.pop_back();
  if (blocks.empty())
    TheBlock = 0;
  else
    TheBlock = blocks.back();
  return result;
}

BasicBlock*
LLVMEmitter::newBlock(const std::string& name)
{
  blocks.pop_back();
  TheBlock = new BasicBlock(name,TheFunction);
  blocks.push_back(TheBlock);
  return TheBlock;
}

Value* 
LLVMEmitter::ConvertToBoolean(Value* V) const
{
  const Type* Ty = V->getType();
  if (Ty == Type::Int1Ty)
    return V;

  if (Ty->isInteger() || Ty->isFloatingPoint()) {
    Constant* CI = Constant::getNullValue(V->getType());
    return new ICmpInst(ICmpInst::ICMP_NE, V, CI, "i2b", TheBlock);
  } else if (isa<GlobalValue>(V)) {
    // GlobalValues always have non-zero constant address values, so always true
    return ConstantInt::getTrue();
  }
  hlvmAssert(!"Don't know how to convert V into bool");
  return ConstantInt::getTrue();
}

Value* 
LLVMEmitter::Pointer2Value(Value* V) const
{
  if (!isa<PointerType>(V->getType()))
    return V;

  // GetElementPtrInst* GEP = new GetElementPtrIns(V,
  //    ConstantInt::get(Type::Int32Ty,0),
  //    ConstantInt::get(Type::Int32Ty,0),
  //    "ptr2Value", TheBlock);
  return new LoadInst(V,"ptr2Value", TheBlock);
}

bool
LLVMEmitter::IsNoopCast(Value* V, const Type* Ty)
{
  // check signed to unsigned
  const Type *VTy = V->getType();
  if (VTy->canLosslesslyBitCastTo(Ty)) 
    return true;
  
  // Constant int to anything, to work around stuff like: "xor short X, int 1".
  if (isa<ConstantInt>(V)) 
    return true;
  
  return false;
}

/// CastToType - Cast the specified value to the specified type if it is
/// not already that type.
Value *
LLVMEmitter::CastToType(Value *V, bool srcIsSigned, const Type *Ty, 
                        bool destIsSigned, const std::string& newName) 
{
  // If they are the same type, no cast needed
  if (V->getType() == Ty) 
    return V;

  // Get the opcode necessary for the cast.
  Instruction::CastOps Opcode = 
    CastInst::getCastOpcode(V, srcIsSigned, Ty, destIsSigned);

  // If its a constant then we want a constant cast
  if (Constant *C = dyn_cast<Constant>(V))
    return ConstantExpr::getCast(Opcode, C, Ty);
  
  // If its a cast instruction and we're casting back to the original type, 
  // which is bool, then just get the operand of the cast instead of emitting 
  // duplicate cast instructions. This is just an optimization of a frequently
  // occurring case.
  if (CastInst *CI = dyn_cast<CastInst>(V))
    if (Ty == Type::Int1Ty && CI->getOperand(0)->getType() == Type::Int1Ty)
      return CI->getOperand(0);

  // Otherwise, just issue the cast
  return CastInst::create(Opcode, V, Ty, 
                          (newName.empty() ? V->getName() : newName), TheBlock);
}

void 
LLVMEmitter::ResolveBreaks(BasicBlock* exit)
{
  for (BranchList::iterator I = breaks.begin(), E = breaks.end(); I != E; ++I) {
    (*I)->setOperand(0,exit);
  }
  breaks.clear();
}

void 
LLVMEmitter::ResolveContinues(BasicBlock* entry)
{
  for (BranchList::iterator I = continues.begin(), E = continues.end(); 
       I != E; ++I) {
    (*I)->setOperand(0,entry);
  }
  continues.clear();
}

/// Return the number of elements in the specified type that will need to be 
/// loaded/stored if we copy this one element at a time.
unsigned 
LLVMEmitter::getNumElements(const Type *Ty) 
{
  if (Ty->isFirstClassType()) return 1;

  if (const StructType *STy = dyn_cast<StructType>(Ty)) {
    unsigned NumElts = 0;
    for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i)
      NumElts += getNumElements(STy->getElementType(i));
    return NumElts;
  } else if (const ArrayType *ATy = dyn_cast<ArrayType>(Ty)) {
    return ATy->getNumElements() * getNumElements(ATy->getElementType());
  } else if (const PackedType* PTy = dyn_cast<PackedType>(Ty)) {
    return PTy->getNumElements() * getNumElements(PTy->getElementType());
  } else
    hlvmAssert(!"Don't know how to count elements of this type");
  return 0;
}

Constant*
LLVMEmitter::getFirstElement(GlobalVariable* GV)
{
  ArgList indices;
  TwoZeroIndices(indices);
  return ConstantExpr::getGetElementPtr(GV,indices);
}

FunctionType*
LLVMEmitter::getFunctionType(
  const std::string& name, const Type* resultTy, 
  const TypeList& args, bool varargs)
{
  // The args might contain non-first-class typed arguments. We build a
  // new argument list the contains the argument types after conversion.
  TypeList Params;

  // We can't have results that are not first class so we use the
  // first argument to point to where the result should be stored and
  // switch the result type to VoidTy.
  if (!resultTy->isFirstClassType()) {
    Params.push_back(getFirstClassType(resultTy));
    resultTy = Type::VoidTy;
  }

  for (TypeList::const_iterator I = args.begin(), E = args.end(); I != E; ++I ) 
  {
    if ((*I)->isFirstClassType())
      Params.push_back(*I);
    else 
      Params.push_back(getFirstClassType(*I));
  }

  FunctionType* result = FunctionType::get(resultTy, Params, varargs);
  if (!name.empty())
    TheModule->addTypeName(name,result);
  return result;
}

Type*
LLVMEmitter::getTextType()
{
  return static_cast<LLVMEmitterImpl*>(this)->get_hlvm_text();
}

Type*
LLVMEmitter::getStreamType()
{
  return static_cast<LLVMEmitterImpl*>(this)->get_hlvm_stream();
}

Type*
LLVMEmitter::getBufferType()
{
  return static_cast<LLVMEmitterImpl*>(this)->get_hlvm_buffer();
}

FunctionType* 
LLVMEmitter::getProgramType()
{
  return static_cast<LLVMEmitterImpl*>(this)->get_hlvm_program_signature();
}

llvm::CmpInst* LLVMEmitter::emitNE(llvm::Value* V1, llvm::Value* V2){
  if (V1->getType()->isFloatingPoint())
    return new llvm::FCmpInst(llvm::FCmpInst::FCMP_ONE, V1, V2, "ne",TheBlock);
  else
    return new llvm::ICmpInst(llvm::ICmpInst::ICMP_NE,  V1, V2, "ne",TheBlock);
}

llvm::CmpInst* LLVMEmitter::emitEQ(llvm::Value* V1, llvm::Value* V2){
  if (V1->getType()->isFloatingPoint())
    return new llvm::FCmpInst(llvm::FCmpInst::FCMP_OEQ, V1, V2, "eq",TheBlock);
  else
    return new llvm::ICmpInst(llvm::ICmpInst::ICMP_EQ,  V1, V2, "eq",TheBlock);
}

llvm::CmpInst* LLVMEmitter::emitLT(llvm::Value* V1, llvm::Value* V2, bool sign){
  if (V1->getType()->isFloatingPoint())
    return new llvm::FCmpInst(llvm::FCmpInst::FCMP_OLT, V1, V2,"olt",TheBlock);
  else if (sign)
    return new llvm::ICmpInst(llvm::ICmpInst::ICMP_ULT, V1, V2,"ult",TheBlock);
  else
    return new llvm::ICmpInst(llvm::ICmpInst::ICMP_SLT, V1, V2,"slt",TheBlock);
}

llvm::CmpInst* LLVMEmitter::emitGT(llvm::Value* V1, llvm::Value* V2, bool sign){
  if (V1->getType()->isFloatingPoint())
    return new llvm::FCmpInst(llvm::FCmpInst::FCMP_OGT, V1, V2,"ogt",TheBlock);
  else if (sign)
    return new llvm::ICmpInst(llvm::ICmpInst::ICMP_SGT, V1, V2,"sgt",TheBlock);
  else
    return new llvm::ICmpInst(llvm::ICmpInst::ICMP_UGT, V1, V2,"ugt",TheBlock);
}

llvm::CmpInst* LLVMEmitter::emitLE(llvm::Value* V1, llvm::Value* V2, bool sign){
  if (V1->getType()->isFloatingPoint())
    return new llvm::FCmpInst(llvm::FCmpInst::FCMP_OLE, V1, V2,"ole",TheBlock);
  else if (sign)
    return new llvm::ICmpInst(llvm::ICmpInst::ICMP_ULE, V1, V2,"ule",TheBlock);
  else
    return new llvm::ICmpInst(llvm::ICmpInst::ICMP_SLE, V1, V2,"sle",TheBlock);
}

llvm::CmpInst* LLVMEmitter::emitGE(llvm::Value* V1, llvm::Value* V2, bool sign){
  if (V1->getType()->isFloatingPoint())
    return new llvm::FCmpInst(llvm::FCmpInst::FCMP_OGE, V1, V2,"oge",TheBlock);
  else if (sign)
    return new llvm::ICmpInst(llvm::ICmpInst::ICMP_SGE, V1, V2,"sge",TheBlock);
  else
    return new llvm::ICmpInst(llvm::ICmpInst::ICMP_UGE, V1, V2,"uge",TheBlock);
}

void
LLVMEmitter::emitAssign(Value* dest, Value* src)
{
  // If the destination is a load instruction then its the result of a previous
  // nested block so just get the first operand as the real destination.
  if (isa<LoadInst>(dest))
    dest = cast<LoadInst>(dest)->getOperand(0);

  // The destination must be a pointer type
  hlvmAssert(isa<PointerType>(dest->getType()));

  // Get the type of the destination and source
  const Type* destTy = cast<PointerType>(dest->getType())->getElementType();
  const Type* srcTy = src->getType();

  if (destTy->isFirstClassType()) {
    // Can't store an aggregate to a first class type
    hlvmAssert(srcTy->isFirstClassType());
    if (destTy == srcTy) {
      // simple case, the types match and they are both first class types, 
      // just emit a store instruction
      emitStore(src,dest);
    } else if (const PointerType* srcPT = dyn_cast<PointerType>(srcTy)) {
      // The source is a pointer to the value to return so just get a
      // pointer to its first element and store it since its pointing to
      // a first class type. Assert that this is true.
      hlvmAssert( srcPT->getElementType() == destTy );
      ArgList idx;
      TwoZeroIndices(idx);
      GetElementPtrInst* GEP = new GetElementPtrInst(src, idx, "",TheBlock);
      emitStore(GEP,dest);
    } else {
      // they are both first class types and the source is not a pointer, so 
      // just cast them. FIXME: signedness
      Value* V = CastToType(src, false, destTy, false, src->getName());
      emitStore(V, dest);
    }
  } 
  else if (const PointerType* srcPT = dyn_cast<PointerType>(srcTy)) 
  {
    // We have an aggregate to copy
    emitAggregateCopy(dest,src);
  } 
  else if (Constant* srcC = dyn_cast<Constant>(src)) 
  {
    // We have a constant aggregate to move into an aggregate gvar. We must
    // create a temporary gvar based on the constant in order to copy it.
    GlobalVariable* GVar = NewGConst(srcTy, srcC, srcC->getName());
    // Now we can aggregate copy it
    emitAggregateCopy(dest, GVar);
  }
}

/// Recursively traverse the potientially aggregate src/dest ptrs, copying all 
/// of the elements from src to dest.
static void 
CopyAggregate(
  Value *DestPtr, 
  bool DestVolatile, 
  Value *SrcPtr, 
  bool SrcVolatile,
  BasicBlock *BB) 
{
  assert(DestPtr->getType() == SrcPtr->getType() &&
         "Cannot copy between two pointers of different type!");
  const Type *ElTy = cast<PointerType>(DestPtr->getType())->getElementType();
  if (ElTy->isFirstClassType()) {
    Value *V = new LoadInst(SrcPtr, "tmp", SrcVolatile, BB);
    new StoreInst(V, DestPtr, DestVolatile, BB);
  } else if (const StructType *STy = dyn_cast<StructType>(ElTy)) {
    Constant *Zero = ConstantInt::get(Type::Int32Ty, 0);
    for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i) {
      Constant *Idx = ConstantInt::get(Type::Int32Ty, i);
      Value *DElPtr = new GetElementPtrInst(DestPtr, Zero, Idx, "tmp", BB);
      Value *SElPtr = new GetElementPtrInst(SrcPtr, Zero, Idx, "tmp", BB);
      CopyAggregate(DElPtr, DestVolatile, SElPtr, SrcVolatile, BB);
    }
  } else {
    const ArrayType *ATy = cast<ArrayType>(ElTy);
    Constant *Zero = ConstantInt::get(Type::Int32Ty, 0);
    for (unsigned i = 0, e = ATy->getNumElements(); i != e; ++i) {
      Constant *Idx = ConstantInt::get(Type::Int32Ty, i);
      Value *DElPtr = new GetElementPtrInst(DestPtr, Zero, Idx, "tmp", BB);
      Value *SElPtr = new GetElementPtrInst(SrcPtr, Zero, Idx, "tmp", BB);
      CopyAggregate(DElPtr, DestVolatile, SElPtr, SrcVolatile, BB);
    }
  }
}

/// EmitAggregateCopy - Copy the elements from SrcPtr to DestPtr, using the
/// GCC type specified by GCCType to know which elements to copy.
void 
LLVMEmitter::emitAggregateCopy(
  Value *DestPtr,
  Value *SrcPtr )
{
  // Make sure we're not mixing apples and oranges
  hlvmAssert(DestPtr->getType() == SrcPtr->getType());

  // Degenerate case, same pointer
  if (DestPtr == SrcPtr)
    return;  // noop copy.

  // If the type has just a few elements, then copy the elements directly 
  // using load/store. Otherwise use the llvm.memcpy.i64 intrinsic. This just
  // saves a function call for really small structures and arrays. 
  const Type* Ty = SrcPtr->getType();
  if (getNumElements(Ty) <= 8) 
    CopyAggregate(DestPtr, false, SrcPtr, false, TheBlock);
  else 
    emitMemCpy(DestPtr, SrcPtr, ConstantExpr::getSizeOf(Ty));
}

ReturnInst*
LLVMEmitter::emitReturn(Value* retVal)
{
  // First deal with the degenerate case, a void return
  if (retVal == 0) {
    hlvmAssert(getReturnType() == Type::VoidTy);
    return new ReturnInst(0,TheBlock);
  }

  // Now, deal with first class result types. Becasue of the way function
  // types are generated, a void type at this point indicates an aggregate
  // result. If we don't have a void type, then it must be a first class result.
  const Type* resultTy = retVal->getType();
  if (getReturnType() != Type::VoidTy) {
    Value* result = 0;
    if (const PointerType* PTy = dyn_cast<PointerType>(resultTy)) {
      // must be an autovar or global var, just load the value
      hlvmAssert(PTy->getElementType() == getReturnType());
      result = emitLoad(retVal,getBlockName() + "_result");
    } else if (resultTy != getReturnType()) {
      hlvmAssert(resultTy->isFirstClassType());
      // FIXME: signedness
      result = CastToType(retVal, false, getReturnType(), false, 
                          getBlockName()+"_result");
    } else {
      hlvmAssert(resultTy->isFirstClassType());
      result = retVal;
    }
    hlvmAssert(result && "No result for function");
    return new ReturnInst(result,TheBlock);
  }

  // Now, deal with the aggregate result case. At this point the function return
  // type must be void and the first argument must be a pointer to the storage
  // area for the result.
  hlvmAssert(getReturnType() == Type::VoidTy);

  // Get the first argument.
  hlvmAssert(!TheFunction->arg_empty());
  Argument* result_arg = TheFunction->arg_begin();

  // Both the first argument and the result should have the same type which
  // both should be pointer to the aggregate
  hlvmAssert(result_arg->getType() == resultTy);

  // Copy the aggregate result
  emitAggregateCopy(result_arg, retVal);

  // Emit the void return
  return new ReturnInst(0, TheBlock);
}

CallInst* 
LLVMEmitter::emitCall(Function* F, const ArgList& args) 
{
  // Detect the aggregate result case
  if ((F->getReturnType() == Type::VoidTy) &&
      (args.size() == F->arg_size() - 1)) {
    const Type* arg1Ty = F->arg_begin()->getType();
    if (const PointerType* PTy = dyn_cast<PointerType>(arg1Ty))
    {
      // This is the case where the result is a temporary variable that
      // holds the aggregate and is passed as the first argument
      const Type* elemTy = PTy->getElementType();
      hlvmAssert(!elemTy->isFirstClassType());

      // Get a temporary for the result
      AllocaInst* result = NewAutoVar(elemTy, F->getName() + "_result");

      // Install the temporary result area into a new arg list
      ArgList newArgs;
      newArgs.push_back(result);

      // Copy the other arguments
      for (ArgList::const_iterator I = args.begin(), E = args.end(); 
           I != E; ++I)
        if (isa<Constant>(*I) && !isa<GlobalValue>(*I) && 
            !(*I)->getType()->isFirstClassType())
          newArgs.push_back(NewGConst((*I)->getType(), 
            cast<Constant>(*I), (*I)->getName()));
        else
          newArgs.push_back(*I);

      // Generate the call
      return new CallInst(F, newArgs, "", TheBlock);
    }
  }

  // The result must be a first class type at this point, ensure it
  hlvmAssert(F->getReturnType()->isFirstClassType());

  // Copy the other arguments
  ArgList newArgs;
  for (ArgList::const_iterator I = args.begin(), E = args.end(); 
       I != E; ++I)
    if (isa<Constant>(*I) && !isa<GlobalValue>(*I) && 
        !(*I)->getType()->isFirstClassType())
      newArgs.push_back(NewGConst((*I)->getType(), 
        cast<Constant>(*I), (*I)->getName()));
    else
      newArgs.push_back(*I);
  return new CallInst(F, newArgs, F->getName() + "_result", TheBlock);
}

void 
LLVMEmitter::emitMemCpy(
  Value *dest, 
  Value *src, 
  Value *size
)
{
  const Type *SBP = PointerType::get(Type::Int8Ty);
  ArgList args;
  args.push_back(CastToType(dest, false, SBP, false, ""));
  args.push_back(CastToType(src, false, SBP, false, ""));
  args.push_back(size);
  args.push_back(ConstantInt::get(Type::Int32Ty, 0u));
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  new CallInst(emimp->get_llvm_memcpy(), args, "", TheBlock);
}

/// Emit an llvm.memmove.i64 intrinsic
void 
LLVMEmitter::emitMemMove(
  Value *dest,
  Value *src, 
  Value *size
)
{
  const Type *SBP = PointerType::get(Type::Int8Ty);
  ArgList args;
  args.push_back(CastToType(dest, false, SBP, false, ""));
  args.push_back(CastToType(src, false, SBP, false, ""));
  args.push_back(size);
  args.push_back(ConstantInt::get(Type::Int32Ty, 0u));
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  new CallInst(emimp->get_llvm_memmove(), args, "", TheBlock);
}

/// Emit an llvm.memset.i64 intrinsic
void 
LLVMEmitter::emitMemSet(
  Value *dest, 
  Value *val, 
  Value *size 
)
{
  const Type *SBP = PointerType::get(Type::Int8Ty);
  ArgList args;
  args.push_back(CastToType(dest, false, SBP, false, ""));
  args.push_back(CastToType(val, false, Type::Int8Ty, false, ""));
  args.push_back(size);
  args.push_back(ConstantInt::get(Type::Int32Ty, 0u));
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  new CallInst(emimp->get_llvm_memset(), args, "", TheBlock);
}

CallInst* 
LLVMEmitter::emitOpen(llvm::Value* strm)
{
  std::vector<llvm::Value*> args;
  if (const llvm::PointerType* PT = 
      llvm::dyn_cast<llvm::PointerType>(strm->getType())) 
  {
    const llvm::Type* Ty = PT->getElementType();
    if (Ty == llvm::Type::Int8Ty) {
      args.push_back(strm);
    } else if (llvm::isa<ArrayType>(Ty) && 
             cast<ArrayType>(Ty)->getElementType() == Type::Int8Ty) {
      ArgList indices;
      this->TwoZeroIndices(indices);
      args.push_back(this->emitGEP(strm,indices));
    } else
      hlvmAssert(!"Array element type is not Int8Ty");
  } else
    hlvmAssert(!"OpenOp parameter is not a pointer");

  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  return emimp->call_hlvm_stream_open(args,"open");
}

CallInst* 
LLVMEmitter::emitClose(llvm::Value* strm)
{
  std::vector<llvm::Value*> args;
  args.push_back(strm);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  return emimp->call_hlvm_stream_close(args);
}

CallInst* 
LLVMEmitter::emitRead(llvm::Value* strm,llvm::Value* arg2, llvm::Value* arg3)
{
  std::vector<llvm::Value*> args;
  args.push_back(strm);
  args.push_back(arg2);
  args.push_back(arg3);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  return emimp->call_hlvm_stream_read(args,"read");
}

CallInst* 
LLVMEmitter::emitWrite(llvm::Value* strm,llvm::Value* arg2)
{
  std::vector<llvm::Value*> args;
  args.push_back(strm);
  args.push_back(arg2);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  CallInst* result = 0;
  if (llvm::isa<llvm::PointerType>(arg2->getType()))
    if (llvm::cast<llvm::PointerType>(arg2->getType())->getElementType() ==
        llvm::Type::Int8Ty)
      result = emimp->call_hlvm_stream_write_string(args,"write");
  if (arg2->getType() == emimp->get_hlvm_text())
    result = emimp->call_hlvm_stream_write_text(args,"write");
  else if (arg2->getType() == emimp->get_hlvm_buffer())
    result = emimp->call_hlvm_stream_write_buffer(args,"write");
  return result;
}

CallInst*
LLVMEmitter::emitIsPInf(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_ispinf(args);
  return emimp->call_hlvm_f64_ispinf(args);
}

CallInst* 
LLVMEmitter::emitIsNInf(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_isninf(args);
  return emimp->call_hlvm_f64_isninf(args);
}

CallInst* 
LLVMEmitter::emitIsNan(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_isnan(args);
  return emimp->call_hlvm_f64_isnan(args);
}

CallInst* 
LLVMEmitter::emitTrunc(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_trunc(args);
  return emimp->call_hlvm_f64_trunc(args);
}

CallInst* 
LLVMEmitter::emitRound(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_round(args);
  return emimp->call_hlvm_f64_round(args);
}

CallInst* 
LLVMEmitter::emitFloor(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_floor(args);
  return emimp->call_hlvm_f64_floor(args);
}

CallInst* 
LLVMEmitter::emitCeiling(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_ceiling(args);
  return emimp->call_hlvm_f64_ceiling(args);
}

CallInst* 
LLVMEmitter::emitLogE(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_loge(args);
  return emimp->call_hlvm_f64_loge(args);
}

CallInst* 
LLVMEmitter::emitLog2(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_log2(args);
  return emimp->call_hlvm_f64_log2(args);
}

CallInst* 
LLVMEmitter::emitLog10(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_log10(args);
  return emimp->call_hlvm_f64_log10(args);
}

CallInst* 
LLVMEmitter::emitSquareRoot(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_squareroot(args);
  return emimp->call_hlvm_f64_squareroot(args);
}

CallInst* 
LLVMEmitter::emitCubeRoot(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_cuberoot(args);
  return emimp->call_hlvm_f64_cuberoot(args);
}

CallInst* 
LLVMEmitter::emitFactorial(Value* V)
{
  std::vector<llvm::Value*> args;
  args.push_back(V);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V->getType()->isFloatingPoint());
  if (Type::FloatTy == V->getType())
    return emimp->call_hlvm_f32_factorial(args);
  return emimp->call_hlvm_f64_factorial(args);
}

CallInst* 
LLVMEmitter::emitPower(Value* V1,Value*V2)
{
  std::vector<llvm::Value*> args;
  args.push_back(V1);
  args.push_back(V2);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V1->getType()->isFloatingPoint());
  hlvmAssert(V2->getType()->isFloatingPoint());
  if (Type::FloatTy == V1->getType())
    return emimp->call_hlvm_f32_power(args);
  return emimp->call_hlvm_f64_power(args);
}

CallInst* 
LLVMEmitter::emitRoot(Value* V1,Value*V2)
{
  std::vector<llvm::Value*> args;
  args.push_back(V1);
  args.push_back(V2);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V1->getType()->isFloatingPoint());
  hlvmAssert(V2->getType()->isFloatingPoint());
  if (Type::FloatTy == V1->getType())
    return emimp->call_hlvm_f32_root(args);
  return emimp->call_hlvm_f64_root(args);
}

CallInst* 
LLVMEmitter::emitGCD(Value* V1,Value*V2)
{
  std::vector<llvm::Value*> args;
  args.push_back(V1);
  args.push_back(V2);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V1->getType()->isFloatingPoint());
  hlvmAssert(V2->getType()->isFloatingPoint());
  if (Type::FloatTy == V1->getType())
    return emimp->call_hlvm_f32_gcd(args);
  return emimp->call_hlvm_f64_gcd(args);
}

CallInst* 
LLVMEmitter::emitLCM(Value* V1,Value*V2)
{
  std::vector<llvm::Value*> args;
  args.push_back(V1);
  args.push_back(V2);
  LLVMEmitterImpl* emimp = static_cast<LLVMEmitterImpl*>(this);
  hlvmAssert(V1->getType()->isFloatingPoint());
  hlvmAssert(V2->getType()->isFloatingPoint());
  if (Type::FloatTy == V1->getType())
    return emimp->call_hlvm_f32_lcm(args);
  return emimp->call_hlvm_f64_lcm(args);
}

}
