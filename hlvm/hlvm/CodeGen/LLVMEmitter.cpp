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

namespace hlvm {

LLVMEmitter::LLVMEmitter()
  : TheModule(0), TheFunction(0), TheEntryBlock(0), TheExitBlock(0), 
    EntryInsertionPoint(0), TheBlock(0), // TheTarget(0),
    hlvm_text(0), hlvm_text_create(0), hlvm_text_delete(0),
    hlvm_text_to_buffer(0), 
    hlvm_buffer(0), hlvm_buffer_create(0), hlvm_buffer_delete(0),
    hlvm_stream(0), hlvm_stream_open(0), hlvm_stream_read(0),
    hlvm_stream_write_buffer(0), hlvm_stream_write_text(0), 
    hlvm_stream_write_string(0), hlvm_stream_close(0), 
    hlvm_program_signature(0)
{ 
}

llvm::Module*
LLVMEmitter::StartModule(const std::string& ID)
{
  hlvmAssert(TheModule == 0);
  return TheModule = new llvm::Module(ID);
}

llvm::Module*
LLVMEmitter::FinishModule()
{
  hlvmAssert(TheModule != 0);
  llvm::Module* result = TheModule;
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
    new CastInst(Constant::getNullValue(Type::IntTy),Type::IntTy,
        "entry_point", TheEntryBlock);

  // Create a new block for the return node, but don't insert it yet.
  TheExitBlock = new BasicBlock("exit");
}

void
LLVMEmitter::FinishFunction()
{
  hlvmAssert(blocks.empty());
  hlvmAssert(breaks.empty());
  hlvmAssert(continues.empty());
}

llvm::BasicBlock*
LLVMEmitter::pushBlock(const std::string& name)
{
  TheBlock = new llvm::BasicBlock(name,TheFunction);
  blocks.push_back(TheBlock);
  return TheBlock;
}

llvm::BasicBlock* 
LLVMEmitter::popBlock()
{
  llvm::BasicBlock* result = blocks.back();
  blocks.pop_back();
  if (blocks.empty())
    TheBlock = 0;
  else
    TheBlock = blocks.back();
  return result;
}

llvm::BasicBlock*
LLVMEmitter::newBlock(const std::string& name)
{
  blocks.pop_back();
  TheBlock = new llvm::BasicBlock(name,TheFunction);
  blocks.push_back(TheBlock);
  return TheBlock;
}

llvm::Value* 
LLVMEmitter::ConvertToBoolean(llvm::Value* V) const
{
  const llvm::Type* Ty = V->getType();
  if (Ty == llvm::Type::BoolTy)
    return V;

  if (Ty->isInteger() || Ty->isFloatingPoint()) {
    llvm::Constant* CI = llvm::Constant::getNullValue(V->getType());
    return new llvm::SetCondInst(llvm::Instruction::SetNE, V, CI, "i2b", 
        TheBlock);
  } else if (llvm::isa<llvm::GlobalValue>(V)) {
    // GlobalValues always have non-zero constant address values, so always true
    return llvm::ConstantBool::get(true);
  }
  hlvmAssert(!"Don't know how to convert V into bool");
  return llvm::ConstantBool::get(true);
}

Value* 
LLVMEmitter::Pointer2Value(llvm::Value* V) const
{
  if (!llvm::isa<llvm::PointerType>(V->getType()))
    return V;

 // llvm::GetElementPtrInst* GEP = new llvm::GetElementPtrIns(V,
  //    llvm::ConstantInt::get(llvm::Type::UIntTy,0),
   //   llvm::ConstantInt::get(llvm::Type::UIntTy,0),
    //  "ptr2Value", TheBlock);
  return new llvm::LoadInst(V,"ptr2Value", TheBlock);
}

bool
LLVMEmitter::IsNoopCast(Value* V, const Type* Ty)
{
  // check signed to unsigned
  const Type *VTy = V->getType();
  if (VTy->isLosslesslyConvertibleTo(Ty)) return true;
  
  // Constant int to anything, to work around stuff like: "xor short X, int 1".
  if (isa<ConstantInt>(V)) return true;
  
  return false;
}

/// CastToType - Cast the specified value to the specified type if it is
/// not already that type.
Value *
LLVMEmitter::CastToType(Value *V, const Type *Ty) 
{
  // If they are the same type, no cast needed
  if (V->getType() == Ty) 
    return V;

  // If its a constant then we want a constant cast
  if (Constant *C = dyn_cast<Constant>(V))
    return ConstantExpr::getCast(C, Ty);
  
  // If its a cast instruction and we're casting back to the original type, 
  // which is bool, then just get the operand of the cast instead of emitting 
  // duplicate cast instructions. This is just an optimization of a frequently
  // occurring case.
  if (CastInst *CI = dyn_cast<CastInst>(V))
    if (Ty == Type::BoolTy && CI->getOperand(0)->getType() == Type::BoolTy)
      return CI->getOperand(0);

  // Otherwise, just issue the cast
  return new CastInst(V, Ty, V->getName(), TheBlock);
}

Value *
LLVMEmitter::NoopCastToType(Value *V, const Type *Ty)
{
  hlvmAssert(IsNoopCast(V, Ty) && "Invalid Noop Cast!");
  return CastToType(V, Ty);
}

void 
LLVMEmitter::ResolveBreaks(llvm::BasicBlock* exit)
{
  for (BranchList::iterator I = breaks.begin(), E = breaks.end(); I != E; ++I) {
    (*I)->setOperand(0,exit);
  }
  breaks.clear();
}

void 
LLVMEmitter::ResolveContinues(llvm::BasicBlock* entry)
{
  for (BranchList::iterator I = continues.begin(), E = continues.end(); 
       I != E; ++I) {
    (*I)->setOperand(0,entry);
  }
  continues.clear();
}

llvm::Type*
LLVMEmitter::get_hlvm_size()
{
  return llvm::Type::ULongTy;
}

llvm::PointerType*
LLVMEmitter::get_hlvm_text()
{
  if (! hlvm_text) {
    llvm::OpaqueType* opq = llvm::OpaqueType::get();
    TheModule->addTypeName("hlvm_text_obj", opq);
    hlvm_text = llvm::PointerType::get(opq);
    TheModule->addTypeName("hlvm_text", hlvm_text);
  }
  return hlvm_text;
}

llvm::Function*
LLVMEmitter::get_hlvm_text_create()
{
  if (! hlvm_text_create) {
    llvm::Type* result = get_hlvm_text();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(llvm::PointerType::get(llvm::Type::SByteTy));
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    TheModule->addTypeName("hlvm_text_create",FT);
    hlvm_text_create = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
      "hlvm_text_create", TheModule);
  }
  return hlvm_text_create;
}

CallInst*
LLVMEmitter::call_hlvm_text_create(const ArgList& args, const char* nm)
{
  Function* F = get_hlvm_text_create();
  return new llvm::CallInst(F, args, (nm ? nm : "buffer"), TheBlock);
}

llvm::Function*
LLVMEmitter::get_hlvm_text_delete()
{
  if (! hlvm_text_delete) {
    llvm::Type* result = get_hlvm_text();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_text());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    TheModule->addTypeName("hlvm_text_delete",FT);
    hlvm_text_delete = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
      "hlvm_text_delete", TheModule);
  }
  return hlvm_text_delete;
}

CallInst*
LLVMEmitter::call_hlvm_text_delete(const ArgList& args)
{
  Function* F = get_hlvm_text_delete();
  return new llvm::CallInst(F, args, "hlvm_text_delete", TheBlock);
}

llvm::Function*
LLVMEmitter::get_hlvm_text_to_buffer()
{
  if (! hlvm_text_to_buffer) {
    llvm::Type* result = get_hlvm_buffer();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_text());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    TheModule->addTypeName("hlvm_text_to_buffer_signature",FT);
    hlvm_text_to_buffer = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
      "hlvm_text_to_buffer", TheModule);
  }
  return hlvm_text_to_buffer;
}

CallInst*
LLVMEmitter::call_hlvm_text_to_buffer(const ArgList& args, const char* nm)
{
  Function* F = get_hlvm_text_to_buffer();
  return new llvm::CallInst(F, args, (nm ? nm : "buffer"), TheBlock);
}

llvm::PointerType*
LLVMEmitter::get_hlvm_buffer()
{
  if (! hlvm_buffer) {
    llvm::OpaqueType* opq = llvm::OpaqueType::get();
    TheModule->addTypeName("hlvm_buffer_obj", opq);
    hlvm_buffer = llvm::PointerType::get(opq);
    TheModule->addTypeName("hlvm_buffer", hlvm_buffer);
  }
  return hlvm_buffer;
}

llvm::Function*
LLVMEmitter::get_hlvm_buffer_create()
{
  if (! hlvm_buffer_create) {
    llvm::Type* result = get_hlvm_buffer();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_size());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    TheModule->addTypeName("hlvm_buffer_create",FT);
    hlvm_buffer_create = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
      "hlvm_buffer_create", TheModule);
  }
  return hlvm_buffer_create;
}

CallInst*
LLVMEmitter::call_hlvm_buffer_create(const ArgList& args, const char* nm)
{
  Function* F = get_hlvm_buffer_create();
  return new llvm::CallInst(F, args, (nm ? nm : "buffer"), TheBlock);
}

llvm::Function*
LLVMEmitter::get_hlvm_buffer_delete()
{
  if (! hlvm_buffer_delete) {
    llvm::Type* result = get_hlvm_buffer();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_buffer());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    TheModule->addTypeName("hlvm_buffer_delete",FT);
    hlvm_buffer_delete = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
      "hlvm_buffer_delete", TheModule);
  }
  return hlvm_buffer_delete;
}

CallInst*
LLVMEmitter::call_hlvm_buffer_delete(const ArgList& args)
{
  Function* F = get_hlvm_buffer_delete();
  return new llvm::CallInst(F, args, "", TheBlock);
}

llvm::PointerType* 
LLVMEmitter::get_hlvm_stream()
{
  if (! hlvm_stream) {
    llvm::OpaqueType* opq = llvm::OpaqueType::get();
    TheModule->addTypeName("hlvm_stream_obj", opq);
    hlvm_stream= llvm::PointerType::get(opq);
    TheModule->addTypeName("hlvm_stream", hlvm_stream);
  }
  return hlvm_stream;
}

llvm::Function*
LLVMEmitter::get_hlvm_stream_open()
{
  if (!hlvm_stream_open) {
    llvm::Type* result = get_hlvm_stream();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(llvm::PointerType::get(llvm::Type::SByteTy));
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    TheModule->addTypeName("hlvm_stream_open_signature",FT);
    hlvm_stream_open = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
        "hlvm_stream_open", TheModule);
  }
  return hlvm_stream_open;
}

CallInst*
LLVMEmitter::call_hlvm_stream_open(const ArgList& args, const char* nm)
{
  Function* F = get_hlvm_stream_open();
  return new llvm::CallInst(F, args, (nm ? nm : "stream"), TheBlock);
}

llvm::Function*
LLVMEmitter::get_hlvm_stream_read()
{
  if (!hlvm_stream_read) {
    llvm::Type* result = get_hlvm_size();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_stream());
    arg_types.push_back(get_hlvm_buffer());
    arg_types.push_back(get_hlvm_size());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    TheModule->addTypeName("hlvm_stream_read_signature",FT);
    hlvm_stream_read = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage,
      "hlvm_stream_read", TheModule);
  }
  return hlvm_stream_read;
}

CallInst*
LLVMEmitter::call_hlvm_stream_read(const ArgList& args, const char* nm)
{
  Function* F = get_hlvm_stream_read();
  return new llvm::CallInst(F, args, (nm ? nm : "readlen"), TheBlock);
}

llvm::Function*
LLVMEmitter::get_hlvm_stream_write_buffer()
{
  if (!hlvm_stream_write_buffer) {
    llvm::Type* result = get_hlvm_size();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_stream());
    arg_types.push_back(get_hlvm_buffer());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    TheModule->addTypeName("hlvm_stream_write_buffer_signature",FT);
    hlvm_stream_write_buffer = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage,
      "hlvm_stream_write_buffer", TheModule);
  }
  return hlvm_stream_write_buffer;
}

CallInst*
LLVMEmitter::call_hlvm_stream_write_buffer(const ArgList& args, const char* nm)
{
  Function* F = get_hlvm_stream_write_buffer();
  return new llvm::CallInst(F, args, (nm ? nm : "writelen"), TheBlock);
}

llvm::Function*
LLVMEmitter::get_hlvm_stream_write_string()
{
  if (!hlvm_stream_write_string) {
    llvm::Type* result = get_hlvm_size();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_stream());
    arg_types.push_back(llvm::PointerType::get(llvm::Type::SByteTy));
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    TheModule->addTypeName("hlvm_stream_write_string_signature",FT);
    hlvm_stream_write_string = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage,
      "hlvm_stream_write_string", TheModule);
  }
  return hlvm_stream_write_string;
}

CallInst*
LLVMEmitter::call_hlvm_stream_write_string(const ArgList& args, const char* nm)
{
  Function* F = get_hlvm_stream_write_string();
  return new llvm::CallInst(F, args, (nm ? nm : "writelen"), TheBlock);
}

llvm::Function*
LLVMEmitter::get_hlvm_stream_write_text()
{
  if (!hlvm_stream_write_text) {
    llvm::Type* result = get_hlvm_size();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_stream());
    arg_types.push_back(get_hlvm_text());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    TheModule->addTypeName("hlvm_stream_write_text_signature",FT);
    hlvm_stream_write_text = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage,
      "hlvm_stream_write_text", TheModule);
  }
  return hlvm_stream_write_text;
}

CallInst*
LLVMEmitter::call_hlvm_stream_write_text(const ArgList& args, const char* nm)
{
  Function* F = get_hlvm_stream_write_text();
  return new llvm::CallInst(F, args, (nm ? nm : "writelen"), TheBlock);
}

llvm::Function*
LLVMEmitter::get_hlvm_stream_close()
{
  if (!hlvm_stream_close) {
    llvm::Type* result = llvm::Type::VoidTy;
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_stream());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    TheModule->addTypeName("hlvm_stream_close_signature",FT);
    hlvm_stream_close = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage,
      "hlvm_stream_close", TheModule);
  }
  return hlvm_stream_close;
}

CallInst*
LLVMEmitter::call_hlvm_stream_close(const ArgList& args)
{
  Function* F = get_hlvm_stream_close();
  return new llvm::CallInst(F, args, "", TheBlock);
}

llvm::FunctionType*
LLVMEmitter::get_hlvm_program_signature()
{
  if (!hlvm_program_signature) {
    // Get the type of function that all entry points must have
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(llvm::Type::IntTy);
    arg_types.push_back(
      llvm::PointerType::get(llvm::PointerType::get(llvm::Type::SByteTy)));
    hlvm_program_signature = 
      llvm::FunctionType::get(llvm::Type::IntTy,arg_types,false);
    TheModule->addTypeName("hlvm_program_signature",hlvm_program_signature);
  }
  return hlvm_program_signature;
}

}
