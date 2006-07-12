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
    EntryInsertionPoint(0), TheBlock(0),
    hlvm_text(0), hlvm_text_create(0), hlvm_text_delete(0),
    hlvm_text_to_buffer(0), 
    hlvm_buffer(0), hlvm_buffer_create(0), hlvm_buffer_delete(0),
    hlvm_stream(0), hlvm_stream_open(0), hlvm_stream_read(0),
    hlvm_stream_write_buffer(0), hlvm_stream_write_text(0), 
    hlvm_stream_write_string(0), hlvm_stream_close(0), 
    hlvm_program_signature(0),
    llvm_memcpy(0), llvm_memmove(0), llvm_memset(0)
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
  // TheExitBlock = new BasicBlock("exit");
}

void
LLVMEmitter::FinishFunction()
{
  // The entry block was created to hold the automatic variables. We now
  // need to terminate the block by branching it to the first active block
  // in the function.
  new llvm::BranchInst(TheFunction->front().getNext(),&TheFunction->front());
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
      // just cast them
      CastInst* CI = emitCast(src,destTy,src->getName());
      emitStore(CI,dest);
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
    Constant *Zero = ConstantUInt::get(Type::UIntTy, 0);
    for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i) {
      Constant *Idx = ConstantUInt::get(Type::UIntTy, i);
      Value *DElPtr = new GetElementPtrInst(DestPtr, Zero, Idx, "tmp", BB);
      Value *SElPtr = new GetElementPtrInst(SrcPtr, Zero, Idx, "tmp", BB);
      CopyAggregate(DElPtr, DestVolatile, SElPtr, SrcVolatile, BB);
    }
  } else {
    const ArrayType *ATy = cast<ArrayType>(ElTy);
    Constant *Zero = ConstantUInt::get(Type::UIntTy, 0);
    for (unsigned i = 0, e = ATy->getNumElements(); i != e; ++i) {
      Constant *Idx = ConstantUInt::get(Type::UIntTy, i);
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
    emitMemCpy(DestPtr, SrcPtr, llvm::ConstantExpr::getSizeOf(Ty));
}

ReturnInst*
LLVMEmitter::emitReturn(Value* retVal)
{
  // First deal with the degenerate case, a void return
  if (retVal == 0) {
    hlvmAssert(getReturnType() == Type::VoidTy);
    return new llvm::ReturnInst(0,TheBlock);
  }

  // Now, deal with first class result types. Becasue of the way function
  // types are generated, a void type at this point indicates an aggregate
  // result. If we don't have a void type, then it must be a first class result.
  const Type* resultTy = retVal->getType();
  if (getReturnType() != llvm::Type::VoidTy) {
    Value* result = 0;
    if (const PointerType* PTy = dyn_cast<PointerType>(resultTy)) {
      // must be an autovar or global var, just load the value
      hlvmAssert(PTy->getElementType() == getReturnType());
      result = emitLoad(retVal,getBlockName() + "_result");
    } else if (resultTy != getReturnType()) {
      hlvmAssert(resultTy->isFirstClassType());
      result = emitCast(retVal, getReturnType(), getBlockName()+"_result");
    } else {
      hlvmAssert(resultTy->isFirstClassType());
      result = retVal;
    }
    hlvmAssert(result && "No result for function");
    return new llvm::ReturnInst(result,TheBlock);
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
  return new llvm::ReturnInst(0, TheBlock);
}

llvm::CallInst* 
LLVMEmitter::emitCall(llvm::Function* F, const ArgList& args) 
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
      return new llvm::CallInst(F, newArgs, "", TheBlock);
    }
  }

  // The result must be a first class type at this point, ensure it
  hlvmAssert(F->getReturnType()->isFirstClassType());

  return new llvm::CallInst(F, args, F->getName() + "_result", TheBlock);
}

void 
LLVMEmitter::emitMemCpy(
  llvm::Value *dest, 
  llvm::Value *src, 
  llvm::Value *size
)
{
  const Type *SBP = PointerType::get(Type::SByteTy);
  ArgList args;
  args.push_back(CastToType(dest, SBP));
  args.push_back(CastToType(src, SBP));
  args.push_back(size);
  args.push_back(ConstantUInt::get(Type::UIntTy, 0));
  new CallInst(get_llvm_memcpy(), args, "", TheBlock);
}

/// Emit an llvm.memmove.i64 intrinsic
void 
LLVMEmitter::emitMemMove(
  llvm::Value *dest,
  llvm::Value *src, 
  llvm::Value *size
)
{
  const Type *SBP = PointerType::get(Type::SByteTy);
  ArgList args;
  args.push_back(CastToType(dest, SBP));
  args.push_back(CastToType(src, SBP));
  args.push_back(size);
  args.push_back(ConstantUInt::get(Type::UIntTy, 0));
  new CallInst(get_llvm_memmove(), args, "", TheBlock);
}

/// Emit an llvm.memset.i64 intrinsic
void 
LLVMEmitter::emitMemSet(
  llvm::Value *dest, 
  llvm::Value *val, 
  llvm::Value *size 
)
{
  const Type *SBP = PointerType::get(Type::SByteTy);
  ArgList args;
  args.push_back(CastToType(dest, SBP));
  args.push_back(CastToType(val, Type::UByteTy));
  args.push_back(size);
  args.push_back(ConstantUInt::get(Type::UIntTy, 0));
  new CallInst(get_llvm_memset(), args, "", TheBlock);
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
    // An hlvm_text is a variable length array of signed bytes preceded by
    // an
    // Arglist args;
    // args.push_back(Type::UIntTy);
    // args.push_back(ArrayType::Get(Type::SByteTy,0));
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

llvm::Function* 
LLVMEmitter::get_llvm_memcpy()
{
  if (!llvm_memcpy) {
    const Type *SBP = PointerType::get(Type::SByteTy);
    llvm_memcpy = TheModule->getOrInsertFunction(
      "llvm.memcpy.i64", Type::VoidTy, SBP, SBP, Type::ULongTy, Type::UIntTy, 
      NULL);
  }
  return llvm_memcpy;
}

llvm::Function* 
LLVMEmitter::get_llvm_memmove()
{
  if (!llvm_memmove) {
    const Type *SBP = PointerType::get(Type::SByteTy);
    llvm_memmove = TheModule->getOrInsertFunction(
      "llvm.memmove.i64", Type::VoidTy, SBP, SBP, Type::ULongTy, Type::UIntTy, 
      NULL);
  }

  return llvm_memmove;
}

llvm::Function* 
LLVMEmitter::get_llvm_memset()
{
  if (!llvm_memset) {
    const Type *SBP = PointerType::get(Type::SByteTy);
    llvm_memset = TheModule->getOrInsertFunction(
      "llvm.memset.i64", Type::VoidTy, SBP, Type::UByteTy, Type::ULongTy, 
      Type::UIntTy, NULL);
  }
  return llvm_memset;
}

}
