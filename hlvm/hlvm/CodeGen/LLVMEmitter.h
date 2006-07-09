//===-- LLVM Code Emitter Interface -----------------------------*- C++ -*-===//
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
/// @file hlvm/CodeGen/LLVMEmitter.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/07/09
/// @since 0.2.0
/// @brief Declares the interface for emitting LLVM code
//===----------------------------------------------------------------------===//

#ifndef HLVM_CODEGEN_LLVMEMITTER_H
#define HLVM_CODEGEN_LLVMEMITTER_H

// Include the LLVM classes that we use
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/BasicBlock.h>
#include <llvm/Function.h>
#include <llvm/GlobalVariable.h>
#include <llvm/Instructions.h>
#include <llvm/DerivedTypes.h>
#include <llvm/TypeSymbolTable.h>
#include <llvm/Constants.h>
#include <llvm/CallingConv.h>
#include "llvm/Support/DataTypes.h"
#include <vector>

namespace hlvm {

/// A simple list of LLVM Modules
typedef std::vector<llvm::Module*> ModuleList;

/// A List of LLVM Branch Instructions. These are used to record unconditional
/// branches that need to be fixed up later with LLVMEmitter::ResolveBranches.
typedef std::vector<llvm::BranchInst*> BranchList;

/// A list of LLVM Blocks, presumably used in a push/pop stack fashion.
typedef std::vector<llvm::BasicBlock*> BlockStack;

/// A list of LLVM Values, used for argument lists
typedef std::vector<llvm::Value*> ArgList;

/// This class provides utility functions for emitting LLVM code. It has no
/// concept of how to translate HLVM into LLVM, that logic is in LLVMGenerator
/// class. This class just keeps track of things in the LLVM world and provides
/// a group of methods to take care of the details of emitting LLVM code. The
/// main purpose for this class is simply to unclutter LLVMGenerator.
class LLVMEmitter 
{
  /// @name Constructors
  /// @{
  public:
    LLVMEmitter();

  /// @}
  /// @name Function Emission
  /// @{
  public:

    // Start a new modules
    llvm::Module* StartModule(const std::string& ID);

    // Finish the started module
    llvm::Module* FinishModule();

    /// Start a new function. This sets up the context of the emitter to start
    /// generating code for function \p F.
    void StartFunction(llvm::Function* F);

    /// Finish the function previously started. This must be called after a call
    /// to StartFunction. Mostly this just checks that the generated function is
    /// sane.
    void FinishFunction();

    /// Add a type to the module
    void AddType(const llvm::Type* Ty, const std::string& name)
    {
      TheModule->addTypeName(name, Ty);
    }

    /// Add a Function to the module
    llvm::Function* NewFunction(
      const llvm::FunctionType* Ty, 
      llvm::GlobalValue::LinkageTypes LT,
      const std::string& name) 
    { 
      return new llvm::Function(Ty, LT, name, TheModule); 
    }

    /// Add a global variable to the module
    llvm::GlobalVariable* NewGVar(
      const llvm::Type* Ty, 
      llvm::GlobalValue::LinkageTypes LT,
      llvm::Constant* init, 
      const std::string& name)
    {
      return new llvm::GlobalVariable(Ty, false, LT, init, name, TheModule);
    }
    /// Add a global constant to the module
    llvm::GlobalVariable* NewGConst(
      const llvm::Type* Ty, 
      llvm::Constant* init, 
      const std::string& name)
    {
      return new llvm::GlobalVariable(Ty, true,
         llvm::GlobalValue::InternalLinkage, init, name, TheModule);
    }

    /// Create a new AllocaInst in the entry block with the given name and type.
    /// The Type must have a fixed/constant size.
    llvm::AllocaInst* NewAutoVar(const llvm::Type* Ty, const std::string& name)
    {
      return new llvm::AllocaInst(Ty, 0, name, EntryInsertionPoint);
    }

    /// Add the specified basic block to the end of the function.  If
    /// the previous block falls through into it, add an explicit branch.  Also,
    /// manage fixups for EH info.
    void EmitBlock(llvm::BasicBlock *BB);
    
    /// Create a new LLVM block with the given \p name and push it onto the
    /// block stack.
    llvm::BasicBlock* pushBlock(const std::string& name);

    /// Pop a block off the block stack and return it.
    llvm::BasicBlock* popBlock();

    /// Replace the top of the block stack with a new block with the given
    /// \p name
    llvm::BasicBlock* newBlock(const std::string& name);

  /// @}
  /// @name Simple Helper Functions
  /// @{
  public: 
    /// Get the current module we're building
    llvm::Module* getModule() const { return TheModule; }

    /// Get the current function we're building
    llvm::Function* getFunction() const { return TheFunction; }

    /// Get the current block we are filling
    llvm::BasicBlock* getBlock() const { return TheBlock; }

    /// Get the name of the current block we're inserting into
    const std::string& getBlockName() const { 
      return TheBlock->getName(); 
    }
    // Get the terminating instruction of the current block
    llvm::Instruction* getBlockTerminator() const { 
      return TheBlock->getTerminator();
    }
    // Get the return type of the current function
    const llvm::Type* getReturnType() const {
      return TheFunction->getReturnType();
    }

    /// If V is a PointerType then load its value unless the referent type is
    /// not first class type
    llvm::Value* Pointer2Value(llvm::Value* V) const;

    /// Convert the value V into a Boolean value.
    llvm::Value* ConvertToBoolean(llvm::Value* V) const;

    /// Run through the \p list and set the first operand of each branch
    /// instruction found there to the \p exit block. This presumes that the
    /// branch instruction is unconditional.
    void ResolveBreaks(llvm::BasicBlock* exit);
    void ResolveContinues(llvm::BasicBlock* entry);

    /// Return true if a cast from V to Ty does not change any bits.
    static bool IsNoopCast(llvm::Value *V, const llvm::Type *Ty);

    /// Cast the \p V  to \p Ty if it is not already that type.
    llvm::Value *CastToType(llvm::Value *V, const llvm::Type *Ty);

    /// Insert a cast of \p V to \p Ty if needed.  This checks that
    /// the cast doesn't change any of the values bits.
    llvm::Value *NoopCastToType(llvm::Value *V, const llvm::Type *Ty); 

    static void TwoZeroIndices(ArgList& indices) {
      indices.clear();
      indices.push_back(llvm::Constant::getNullValue(llvm::Type::UIntTy));
      indices.push_back(llvm::Constant::getNullValue(llvm::Type::UIntTy));
    }

  /// @}
  /// @name Accessors for interface to HLVM Runtime Library
  /// @{
  public:
    /// Buffer manipulation methods
    llvm::PointerType*  get_hlvm_buffer();
    llvm::Function*     get_hlvm_buffer_create();
    llvm::CallInst*     call_hlvm_buffer_create(
        const ArgList& args, const char* name = 0);
    llvm::Function*     get_hlvm_buffer_delete();
    llvm::CallInst*     call_hlvm_buffer_delete(const ArgList& args);

    /// Get the signature for an HLVM program
    llvm::FunctionType* get_hlvm_program_signature();

    /// Get the LLVM type for an HLVM "size" type
    llvm::Type*         get_hlvm_size();

    /// Stream manipulation methods
    llvm::PointerType*  get_hlvm_stream();
    llvm::Function*     get_hlvm_stream_open();
    llvm::CallInst*     call_hlvm_stream_open(
        const ArgList& args, const char* name = 0);
    llvm::Function*     get_hlvm_stream_read();
    llvm::CallInst*     call_hlvm_stream_read(
        const ArgList& args, const char* name = 0);
    llvm::Function*     get_hlvm_stream_write_buffer();
    llvm::CallInst*     call_hlvm_stream_write_buffer(
        const ArgList& args, const char* name = 0);
    llvm::Function*     get_hlvm_stream_write_text();
    llvm::CallInst*     call_hlvm_stream_write_text(
        const ArgList& args, const char* name = 0);
    llvm::Function*     get_hlvm_stream_write_string();
    llvm::CallInst*     call_hlvm_stream_write_string(
        const ArgList& args, const char* name = 0);
    llvm::Function*     get_hlvm_stream_close();
    llvm::CallInst*     call_hlvm_stream_close(const ArgList& args);

    /// Text manipulation methods.
    llvm::PointerType*  get_hlvm_text();
    llvm::Function*     get_hlvm_text_create();
    llvm::CallInst*     call_hlvm_text_create(
        const ArgList& args, const char* name = 0);
    llvm::Function*     get_hlvm_text_delete();
    llvm::CallInst*     call_hlvm_text_delete(const ArgList& args);
    llvm::Function*     get_hlvm_text_to_buffer();
    llvm::CallInst*     call_hlvm_text_to_buffer(
        const ArgList& args, const char* name = 0);

  /// @}
  /// @name Simple Value getters
  /// @{
  public:
    llvm::Constant* getTrue() const { return llvm::ConstantBool::get(true); }
    llvm::Constant* getFalse() const { return llvm::ConstantBool::get(false); }
    llvm::Constant* getSZero() const { 
      return llvm::Constant::getNullValue(llvm::Type::IntTy);
    }
    llvm::Constant* getUZero() const { 
      return llvm::Constant::getNullValue(llvm::Type::UIntTy);
    }
    llvm::Constant* getFOne() const { 
      return llvm::ConstantFP::get(llvm::Type::FloatTy,1.0);
    }
    llvm::Constant* getDOne() const { 
      return llvm::ConstantFP::get(llvm::Type::DoubleTy,1.0);
    }
    llvm::Constant* getFPOne(const llvm::Type* Ty) const {
      return llvm::ConstantFP::get(Ty,1.0);
    }
    llvm::Constant* getSOne() const {
      return llvm::ConstantInt::get(llvm::Type::IntTy,1);
    }
    llvm::Constant* getUOne() const {
      return llvm::ConstantUInt::get(llvm::Type::UIntTy,1);
    }
    llvm::Constant* getIOne(const llvm::Type* Ty) const {
      return llvm::ConstantInt::get(Ty,1);
    }
    llvm::Constant* getNullValue(const llvm::Type* Ty) const { 
      return llvm::Constant::getNullValue(Ty);
    }
    llvm::Constant* getAllOnes(const llvm::Type* Ty) const {
      return llvm::ConstantIntegral::getAllOnesValue(Ty);
    }
  /// @}
  /// @name Simple emitters
  /// @{
  public:
    llvm::SetCondInst* emitNE(llvm::Value* V1, llvm::Value* V2) {
      return new llvm::SetCondInst(llvm::Instruction::SetNE, 
        V1, V2, "ne", TheBlock);
    }
    llvm::SetCondInst* emitEQ(llvm::Value* V1, llvm::Value* V2) {
      return new llvm::SetCondInst(llvm::Instruction::SetEQ, 
        V1,V2,"eq",TheBlock);
    }
    llvm::SetCondInst* emitLT(llvm::Value* V1, llvm::Value* V2) {
      return new llvm::SetCondInst(llvm::Instruction::SetLT, 
        V1,V2,"lt",TheBlock);
    }
    llvm::SetCondInst* emitGT(llvm::Value* V1, llvm::Value* V2) {
      return new llvm::SetCondInst(llvm::Instruction::SetGT, 
        V1,V2,"gt",TheBlock);
    }
    llvm::SetCondInst* emitLE(llvm::Value* V1, llvm::Value* V2) {
      return new llvm::SetCondInst(llvm::Instruction::SetLE, 
        V1,V2,"le",TheBlock);
    }
    llvm::SetCondInst* emitGE(llvm::Value* V1, llvm::Value* V2) {
      return new llvm::SetCondInst(llvm::Instruction::SetGE, 
        V1,V2,"ge",TheBlock);
    }
    llvm::CastInst* emitCast(llvm::Value* V1, const llvm::Type* Ty, 
                             const std::string& name) {
      return new llvm::CastInst(V1,Ty,name,TheBlock);
    }
    llvm::LoadInst* emitLoad(llvm::Value* V, const std::string& name) const {
      return  new llvm::LoadInst(V,name,TheBlock);
    }
    llvm::StoreInst* emitStore(llvm::Value* from, llvm::Value* to) const {
      return new llvm::StoreInst(from,to,TheBlock);
    }
    llvm::BinaryOperator* emitNeg(llvm::Value* V) const {
      return llvm::BinaryOperator::createNeg(V,"neg",TheBlock);
    }
    llvm::BinaryOperator* emitCmpl(llvm::Value* V) const {
      return llvm::BinaryOperator::create(llvm::Instruction::Xor,
        V, getAllOnes(V->getType()), "cmpl", TheBlock);
    }
    llvm::BinaryOperator* emitAdd(llvm::Value* V1, llvm::Value* V2) {
      return llvm::BinaryOperator::create(llvm::Instruction::Add, 
        V1, V2, "add", TheBlock);
    }
    llvm::BinaryOperator* emitSub(llvm::Value* V1, llvm::Value* V2) {
      return llvm::BinaryOperator::create(llvm::Instruction::Sub, 
        V1, V2, "sub", TheBlock);
    }
    llvm::BinaryOperator* emitMul(llvm::Value* V1, llvm::Value* V2) {
      return llvm::BinaryOperator::create(llvm::Instruction::Mul, 
        V1, V2, "mul", TheBlock);
    }
    llvm::BinaryOperator* emitDiv(llvm::Value* V1, llvm::Value* V2) const {
      return llvm::BinaryOperator::create(llvm::Instruction::Div, 
        V1, V2, "div", TheBlock);
    }
    llvm::BinaryOperator* emitMod(llvm::Value* V1, llvm::Value* V2) const {
      return llvm::BinaryOperator::create(llvm::Instruction::Rem, 
        V1, V2, "mod", TheBlock);
    }
    llvm::BinaryOperator* emitBAnd(llvm::Value*V1, llvm::Value* V2) const {
      return llvm::BinaryOperator::create(llvm::Instruction::And, 
        V1, V2, "band", TheBlock);
    }
    llvm::BinaryOperator* emitBOr(llvm::Value*V1, llvm::Value* V2) const {
      return llvm::BinaryOperator::create(llvm::Instruction::Or, 
        V1, V2, "bor", TheBlock);
    }
    llvm::BinaryOperator* emitBXor(llvm::Value*V1, llvm::Value* V2) const {
      return llvm::BinaryOperator::create(llvm::Instruction::Xor, 
        V1, V2, "bxor", TheBlock);
    }
    llvm::BinaryOperator* emitBNor(llvm::Value* V1, llvm::Value* V2) const {
      llvm::BinaryOperator* bor = llvm::BinaryOperator::create(
        llvm::Instruction::Or, V1, V2, "bnor", TheBlock);
      return llvm::BinaryOperator::createNot(bor,"bnor",TheBlock);
    }
    llvm::BinaryOperator* emitNot(llvm::Value* V1) const {
      return llvm::BinaryOperator::createNot(
          ConvertToBoolean(V1),"not",TheBlock);
    }
    llvm::BinaryOperator* emitAnd(llvm::Value* V1, llvm::Value* V2) const {
      return llvm::BinaryOperator::create(llvm::Instruction::And, 
        ConvertToBoolean(V1), ConvertToBoolean(V2), "and", TheBlock);
    }
    llvm::BinaryOperator* emitOr(llvm::Value* V1, llvm::Value* V2) const {
      return llvm::BinaryOperator::create(llvm::Instruction::Or, 
        ConvertToBoolean(V1), ConvertToBoolean(V2), "or", TheBlock);
    }
    llvm::BinaryOperator* emitXor(llvm::Value*V1, llvm::Value* V2) const {
      return llvm::BinaryOperator::create(llvm::Instruction::Xor, 
        ConvertToBoolean(V1), ConvertToBoolean(V2), "xor", TheBlock);
    }
    llvm::BinaryOperator* emitNor(llvm::Value* V1, llvm::Value* V2) const {
      llvm::BinaryOperator* op = llvm::BinaryOperator::create(
        llvm::Instruction::Or, ConvertToBoolean(V1), ConvertToBoolean(V2), 
        "nor", TheBlock);
      return llvm::BinaryOperator::createNot(op,"nor",TheBlock);
    }
    llvm::SelectInst* emitSelect(llvm::Value* V1, llvm::Value* V2, 
      llvm::Value* V3, const std::string& name) {
      return new llvm::SelectInst(V1,V2,V3,name,TheBlock);
    }
    llvm::BranchInst* emitBranch(llvm::BasicBlock* blk) {
      return new llvm::BranchInst(blk,TheBlock);
    }
    llvm::BranchInst* emitBreak() {
      llvm::BranchInst* brnch = new llvm::BranchInst(TheBlock,TheBlock);
      breaks.push_back(brnch);
      return brnch;
    }
    llvm::BranchInst* emitContinue() {
      llvm::BranchInst* brnch = new llvm::BranchInst(TheBlock,TheBlock);
      continues.push_back(brnch);
      return brnch;
    }
    llvm::ReturnInst* emitReturn(llvm::Value* V) {
      return new llvm::ReturnInst(V,TheBlock);
    }
    llvm::CallInst* emitCall(llvm::Function* F, const ArgList& args) {
      return new llvm::CallInst(F,args,"call_" + F->getName(),TheBlock);
    }
    llvm::GetElementPtrInst* emitGEP(llvm::Value* V, const ArgList& indices) {
      return new llvm::GetElementPtrInst(V,indices,"",TheBlock);
    }
  /// @}
  /// @name Other miscellaneous functions
  /// @{
  public:

    /// Return the unique ID of the specified basic
    /// block for uses that take the address of it.
    llvm::Constant *getIndirectGotoBlockNumber(llvm::BasicBlock *BB);
    
    /// Get (and potentially lazily create) the indirect
    /// goto block.
    llvm::BasicBlock *getIndirectGotoBlock();
    
    /// Copy the elements from SrcPtr to DestPtr, using the
    /// GCC type specified by GCCType to know which elements to copy.
    void EmitAggregateCopy(
      llvm::Value *DestPtr, llvm::Value *SrcPtr, bool isVolatile);

    /// Zero the elements of DestPtr.
    void EmitAggregateZero(llvm::Value *DestPtr);
                           
    /// Emit an llvm.memcpy.i32 or llvm.memcpy.i64 intrinsic
    void EmitMemCpy(
      llvm::Value *DestPtr, 
      llvm::Value *SrcPtr, 
      llvm::Value *Size, 
      unsigned Align
    );

    /// Emit an llvm.memmove.i32 or llvm.memmove.i64 intrinsic
    void EmitMemMove(
      llvm::Value *DestPtr,
      llvm::Value *SrcPtr, 
      llvm::Value *Size, 
      unsigned Align
    );

    /// Emit an llvm.memset.i32 or llvm.memset.i64 intrinsic
    void EmitMemSet(
      llvm::Value *DestPtr, 
      llvm::Value *SrcVal, 
      llvm::Value *Size, 
      unsigned Align
    );

    /// Emit an unconditional branch to the specified basic block, running 
    /// cleanups if the branch exits scopes.  The argument specify
    /// how to handle these cleanups.
    void EmitBranchInternal(llvm::BasicBlock *Dest, bool IsExceptionEdge);

    /// Add the specified unconditional branch to the fixup list for the 
    /// outermost exception scope, merging it if there is already a fixup that 
    /// works.
    void AddBranchFixup(llvm::BranchInst *BI, bool isExceptionEdge);

  /// @}
  /// @name Data Members
  /// @{
  private:
    BlockStack blocks;              ///< The stack of nested blocks 
    BranchList breaks;              ///< The list of breaks to fix up later
    BranchList continues;           ///< The list of continues to fix up later
    llvm::Module *TheModule;        ///< The module that we are compiling into.
    llvm::Function * TheFunction;   ///< The function we're constructing
    llvm::BasicBlock* TheEntryBlock;///< The function's entry block
    llvm::BasicBlock* TheExitBlock; ///< The function's exit (return) block
    llvm::Instruction* EntryInsertionPoint; ///< Insertion point for entry stuff
    llvm::BasicBlock* TheBlock;     ///< The current block we're building
    //llvm::TargetMachine *TheTarget; /// The current target being compiled for.

    // Caches of things to interface with the HLVM Runtime Library
    llvm::PointerType*  hlvm_text;          ///< Opaque type for text objects
    llvm::Function*     hlvm_text_create;   ///< Create a new text object
    llvm::Function*     hlvm_text_delete;   ///< Delete a text object
    llvm::Function*     hlvm_text_to_buffer;///< Convert text to a buffer
    llvm::PointerType*  hlvm_buffer;        ///< Pointer To octet
    llvm::Function*     hlvm_buffer_create; ///< Create a new buffer object
    llvm::Function*     hlvm_buffer_delete; ///< Delete a buffer
    llvm::PointerType*  hlvm_stream;        ///< Pointer to stream type
    llvm::Function*     hlvm_stream_open;   ///< Function for stream_open
    llvm::Function*     hlvm_stream_read;   ///< Function for stream_read
    llvm::Function*     hlvm_stream_write_buffer; ///< Write buffer to stream
    llvm::Function*     hlvm_stream_write_text;   ///< Write text to stream
    llvm::Function*     hlvm_stream_write_string; ///< Write string to stream
    llvm::Function*     hlvm_stream_close;  ///< Function for stream_close
    llvm::FunctionType* hlvm_program_signature; ///< The llvm type for programs
  /// @}
};

} // end hlvm namespace

#endif
