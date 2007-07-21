/* APPLE LOCAL begin LLVM (ENTIRE FILE!)  */
/* High-level LLVM backend interface 
Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
Contributed by Chris Lattner (sabre@nondot.org)

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

//===----------------------------------------------------------------------===//
// This is the code that converts GCC AST nodes into LLVM code.
//===----------------------------------------------------------------------===//

#include "llvm/ValueSymbolTable.h"
#include "llvm-abi.h"
#include "llvm-internal.h"
#include "llvm-debug.h"
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/InlineAsm.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/Target/TargetAsmInfo.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/DenseMap.h"
#include <iostream>

extern "C" {
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "c-tree.h"  // FIXME: eliminate.
#include "tree-iterator.h"
#include "output.h"
#include "diagnostic.h"
#include "real.h"
#include "langhooks.h"
#include "function.h"
#include "toplev.h"
#include "flags.h"
#include "target.h"
#include "hard-reg-set.h"
#include "except.h"
extern bool tree_could_throw_p(tree);  // tree-flow.h uses non-C++ C constructs.
extern int get_pointer_alignment (tree exp, unsigned int max_align);
}

//#define ITANIUM_STYLE_EXCEPTIONS

//===----------------------------------------------------------------------===//
//                   Matching LLVM Values with GCC DECL trees
//===----------------------------------------------------------------------===//
//
// LLVMValues is a vector of LLVM Values. GCC tree nodes keep track of LLVM 
// Values using this vector's index. It is easier to save and restore the index 
// than the LLVM Value pointer while usig PCH. 

// Collection of LLVM Values
static std::vector<Value *> LLVMValues;
typedef DenseMap<Value *, unsigned> LLVMValuesMapTy;
static LLVMValuesMapTy LLVMValuesMap;

// Remember the LLVM value for GCC tree node.
void llvm_set_decl(tree Tr, Value *V) {

  // If there is not any value then do not add new LLVMValues entry.
  // However clear Tr index if it is non zero.
  if (!V) {
    if (GET_DECL_LLVM_INDEX(Tr))
      SET_DECL_LLVM_INDEX(Tr, 0);
    return;
  }

  unsigned &ValueSlot = LLVMValuesMap[V];
  if (ValueSlot) {
    // Already in map
    SET_DECL_LLVM_INDEX(Tr, ValueSlot);
    return;
  }

  unsigned Index = LLVMValues.size() + 1;
  LLVMValues.push_back(V);
  SET_DECL_LLVM_INDEX(Tr, Index);
  LLVMValuesMap[V] = Index;
}

// Return TRUE if there is a LLVM Value associate with GCC tree node.
bool llvm_set_decl_p(tree Tr) {
  unsigned Index = GET_DECL_LLVM_INDEX(Tr);
  if (Index == 0)
    return false;

  if (LLVMValues[Index - 1])
    return true;

  return false;
}

// Get LLVM Value for the GCC tree node based on LLVMValues vector index.
// If there is not any value associated then use make_decl_llvm() to 
// make LLVM value. When GCC tree node is initialized, it has 0 as the 
// index value. This is why all recorded indices are offset by 1.
Value *llvm_get_decl(tree Tr) {

  unsigned Index = GET_DECL_LLVM_INDEX(Tr);
  if (Index == 0) {
    make_decl_llvm(Tr);
    Index = GET_DECL_LLVM_INDEX(Tr);
    
    // If there was an error, we may have disabled creating LLVM values.
    if (Index == 0) return 0;
  }
  assert ((Index - 1) < LLVMValues.size() && "Invalid LLVM Value index");

  return LLVMValues[Index - 1];
}

/// changeLLVMValue - If Old exists in the LLVMValues map, rewrite it to New.
/// At this point we know that New is not in the map.
void changeLLVMValue(Value *Old, Value *New) {
  assert(!LLVMValuesMap.count(New) && "New cannot be in the map!");
  
  // Find Old in the table.
  LLVMValuesMapTy::iterator I = LLVMValuesMap.find(Old);
  if (I == LLVMValuesMap.end()) return;
  
  unsigned Idx = I->second-1;
  assert(Idx < LLVMValues.size() && "Out of range index!");
  assert(LLVMValues[Idx] == Old && "Inconsistent LLVMValues mapping!");
  
  LLVMValues[Idx] = New;
  
  // Remove the old value from the value map.
  LLVMValuesMap.erase(I);
  
  // Insert the new value into the value map.  We know that it can't already
  // exist in the mapping.
  LLVMValuesMap[New] = Idx+1;
}

// Read LLVM Types string table
void readLLVMValuesStringTable() {

  GlobalValue *V = TheModule->getNamedGlobal("llvm.pch.values");
  if (!V)
    return;

  GlobalVariable *GV = cast<GlobalVariable>(V);
  ConstantStruct *LValueNames = cast<ConstantStruct>(GV->getOperand(0));

  for (unsigned i = 0; i < LValueNames->getNumOperands(); ++i) {
    Value *Va = LValueNames->getOperand(i);

    if (!Va) {
      // If V is empty then nsert NULL to represent empty entries.
      LLVMValues.push_back(Va);
      continue;
    }
    if (ConstantArray *CA = dyn_cast<ConstantArray>(Va)) {
      std::string Str = CA->getAsString();
      Va = TheModule->getValueSymbolTable().lookup(Str);
    } 
    assert (Va != NULL && "Invalid Value in LLVMValues string table");
    LLVMValues.push_back(Va);
  }

  // Now, llvm.pch.values is not required so remove it from the symbol table.
  GV->eraseFromParent();
}

// GCC tree's uses LLVMValues vector's index to reach LLVM Values.
// Create a string table to hold these LLVM Values' names. This string
// table will be used to recreate LTypes vector after loading PCH.
void writeLLVMValuesStringTable() {
  
  if (LLVMValues.empty()) 
    return;

  std::vector<Constant *> LLVMValuesNames;

  for (std::vector<Value *>::iterator I = LLVMValues.begin(),
         E = LLVMValues.end(); I != E; ++I)  {
    Value *V = *I;

    if (!V) {
      LLVMValuesNames.push_back(ConstantArray::get("", false));      
      continue;
    }

    // Give names to nameless values.
    if (!V->hasName())
      V->setName("llvm.fe.val");

    LLVMValuesNames.push_back(ConstantArray::get(V->getName(), false));
  }

  // Create string table.
  Constant *LLVMValuesNameTable = ConstantStruct::get(LLVMValuesNames, false);

  // Create variable to hold this string table.
  new GlobalVariable(LLVMValuesNameTable->getType(), true,
                     GlobalValue::ExternalLinkage, 
                     LLVMValuesNameTable,
                     "llvm.pch.values", TheModule);
}

/// isGCC_SSA_Temporary - Return true if this is an SSA temporary that we can
/// directly compile into an LLVM temporary.  This saves us from creating an
/// alloca and creating loads/stores of that alloca (a compile-time win).  We
/// can only do this if the value is a first class llvm value and if it's a 
/// "gimple_formal_tmp_var".
static bool isGCC_SSA_Temporary(tree decl) {
  return TREE_CODE(decl) == VAR_DECL &&
         DECL_GIMPLE_FORMAL_TEMP_P(decl) && !TREE_ADDRESSABLE(decl) &&
         !isAggregateTreeType(TREE_TYPE(decl));
}

/// isStructWithVarSizeArrayAtEnd - Return true if this StructType contains a
/// zero sized array as its last element.  This typically happens due to C
/// constructs like:  struct X { int A; char B[]; };
static bool isStructWithVarSizeArrayAtEnd(const Type *Ty) {
  const StructType *STy = dyn_cast<StructType>(Ty);
  if (STy == 0) return false;
  assert(STy->getNumElements() && "empty struct?");
  const Type *LastElTy = STy->getElementType(STy->getNumElements()-1);
  return isa<ArrayType>(LastElTy) && 
         cast<ArrayType>(LastElTy)->getNumElements() == 0;
}

/// getINTEGER_CSTVal - Return the specified INTEGER_CST value as a uint64_t.
///
static uint64_t getINTEGER_CSTVal(tree exp) {
  unsigned HOST_WIDE_INT HI = (unsigned HOST_WIDE_INT)TREE_INT_CST_HIGH(exp);
  unsigned HOST_WIDE_INT LO = (unsigned HOST_WIDE_INT)TREE_INT_CST_LOW(exp);
  if (HOST_BITS_PER_WIDE_INT == 64) {
    return (uint64_t)LO;
  } else {
    assert(HOST_BITS_PER_WIDE_INT == 32 &&
           "Only 32- and 64-bit hosts supported!");
    return ((uint64_t)HI << 32) | (uint64_t)LO;
  }
}

//===----------------------------------------------------------------------===//
//                         ... High-Level Methods ...
//===----------------------------------------------------------------------===//

/// TheTreeToLLVM - Keep track of the current function being compiled.  This is
/// only to support the address of labels extension.
static TreeToLLVM *TheTreeToLLVM = 0;

const TargetData &getTargetData() {
  return *TheTarget->getTargetData();
}

TreeToLLVM::TreeToLLVM(tree fndecl) : TD(getTargetData()) {
  FnDecl = fndecl;
  Fn = 0;
  ReturnBB = UnwindBB = 0;
  CurBB = 0;
  
  if (TheDebugInfo) {
    expanded_location Location = expand_location(DECL_SOURCE_LOCATION (fndecl));
     
    if (Location.file) {
      TheDebugInfo->setLocationFile(Location.file);
      TheDebugInfo->setLocationLine(Location.line);
    } else {
      TheDebugInfo->setLocationFile("<unknown file>");
      TheDebugInfo->setLocationLine(0);
    }
  }

  AllocaInsertionPoint = 0;
  
  ExceptionValue = 0;
  ExceptionSelectorValue = 0;
  FuncEHException = 0;
  FuncEHSelector = 0;
  FuncEHFilter = 0;
  FuncEHGetTypeID = 0;
  FuncCPPPersonality = 0;
  FuncUnwindResume = 0; 
  
  NumAddressTakenBlocks = 0;
  IndirectGotoBlock = 0;
  CurrentEHScopes.reserve(16);
  
  assert(TheTreeToLLVM == 0 && "Reentering function creation?");
  TheTreeToLLVM = this;
}

TreeToLLVM::~TreeToLLVM() {
  TheTreeToLLVM = 0;
}

namespace {
  /// FunctionPrologArgumentConversion - This helper class is driven by the ABI
  /// definition for this target to figure out how to retrieve arguments from
  /// the stack/regs coming into a function and store them into an appropriate
  /// alloca for the argument.
  struct FunctionPrologArgumentConversion : public DefaultABIClient {
    tree FunctionDecl;
    Function::arg_iterator &AI;
    BasicBlock *CurBB;
    std::vector<Value*> LocStack;
    std::vector<std::string> NameStack;
    FunctionPrologArgumentConversion(tree FnDecl,
                                     Function::arg_iterator &ai,
                                     BasicBlock *curbb)
      : FunctionDecl(FnDecl), AI(ai), CurBB(curbb) {}
    
    void setName(const std::string &Name) {
      NameStack.push_back(Name);
    }
    void setLocation(Value *Loc) {
      LocStack.push_back(Loc);
    }
    void clear() {
      assert(NameStack.size() == 1 && LocStack.size() == 1 && "Imbalance!");
      NameStack.clear(); 
      LocStack.clear();
    }
    
    void HandleAggregateShadowArgument(const PointerType *PtrArgTy, 
                                       bool RetPtr) {
      // If the function returns a structure by value, we transform the function
      // to take a pointer to the result as the first argument of the function
      // instead.
      assert(AI != CurBB->getParent()->arg_end() &&"No explicit return value?");
      AI->setName("agg.result");
        
      tree ResultDecl = DECL_RESULT(FunctionDecl);
      tree RetTy = TREE_TYPE(TREE_TYPE(FunctionDecl));
      if (TREE_CODE(RetTy) == TREE_CODE(TREE_TYPE(ResultDecl))) {
        SET_DECL_LLVM(ResultDecl, AI);
        ++AI;
        return;
      }
      
      // Otherwise, this must be something returned with NRVO.
      assert(TREE_CODE(TREE_TYPE(ResultDecl)) == REFERENCE_TYPE &&
             "Not type match and not passing by reference?");
      // Create an alloca for the ResultDecl.
      Value *Tmp = TheTreeToLLVM->CreateTemporary(AI->getType());
      new StoreInst(AI, Tmp, CurBB);
      SET_DECL_LLVM(ResultDecl, Tmp);
      if (TheDebugInfo) {
        TheDebugInfo->EmitDeclare(ResultDecl,
                                  llvm::dwarf::DW_TAG_return_variable,
                                  "agg.result", RetTy, Tmp, CurBB);
      }
      ++AI;
    }
    
    void HandleScalarArgument(const llvm::Type *LLVMTy, tree type) {
      Value *ArgVal = AI;
      if (ArgVal->getType() != LLVMTy) {
        if (isa<PointerType>(ArgVal->getType()) && isa<PointerType>(LLVMTy)) {
          // If this is GCC being sloppy about pointer types, insert a bitcast.
          // See PR1083 for an example.
          ArgVal = new BitCastInst(ArgVal, LLVMTy, "tmp", CurBB);
        } else if (ArgVal->getType() == Type::DoubleTy) {
          // If this is a K&R float parameter, it got promoted to double. Insert
          // the truncation to float now.
          ArgVal = new FPTruncInst(ArgVal, LLVMTy, NameStack.back(), CurBB);
        } else {
          // If this is just a mismatch between integer types, this is due
          // to K&R prototypes, where the forward proto defines the arg as int
          // and the actual impls is a short or char.
          assert(ArgVal->getType() == Type::Int32Ty && LLVMTy->isInteger() &&
                 "Lowerings don't match?");
          ArgVal = new TruncInst(ArgVal, LLVMTy, NameStack.back(), CurBB);
        }
      }
      assert(!LocStack.empty());
      Value *Loc = LocStack.back();
      if (cast<PointerType>(Loc->getType())->getElementType() != LLVMTy)
        // This cast only involves pointers, therefore BitCast
        Loc = CastInst::create(Instruction::BitCast, Loc, 
                               PointerType::get(LLVMTy), "tmp", CurBB);
      
      new StoreInst(ArgVal, Loc, CurBB);
      AI->setName(NameStack.back());
      ++AI;
    }
        
    void EnterField(unsigned FieldNo, const llvm::Type *StructTy) {
      NameStack.push_back(NameStack.back()+"."+utostr(FieldNo));
      
      Constant *Zero = Constant::getNullValue(Type::Int32Ty);
      Constant *FIdx = ConstantInt::get(Type::Int32Ty, FieldNo);
      Value *Loc = LocStack.back();
      if (cast<PointerType>(Loc->getType())->getElementType() != StructTy)
        // This cast only involves pointers, therefore BitCast
        Loc = CastInst::create(Instruction::BitCast, Loc, 
                               PointerType::get(StructTy), 
                                           "tmp", CurBB);
      Loc = new GetElementPtrInst(Loc, Zero, FIdx, "tmp", CurBB);
      LocStack.push_back(Loc);    
    }
    void ExitField() {
      NameStack.pop_back();
      LocStack.pop_back();
    }
  };
}

void TreeToLLVM::StartFunctionBody() {
  const char *Name = "";
  // Get the name of the function.
  if (tree ID = DECL_ASSEMBLER_NAME(FnDecl))
    Name = IDENTIFIER_POINTER(ID);
  
  // Determine the FunctionType and calling convention for this function.
  tree static_chain = cfun->static_chain_decl;
  const FunctionType *FTy;
  unsigned CallingConv;
  
  // If the function has no arguments and is varargs (...), turn it into a
  // non-varargs function by scanning the param list for the function.  This
  // allows C functions declared as "T foo() {}" to be treated like 
  // "T foo(void) {}" and allows us to handle functions with K&R-style
  // definitions correctly.
  if (TYPE_ARG_TYPES(TREE_TYPE(FnDecl)) == 0) {
    FTy = TheTypeConverter->ConvertArgListToFnType(TREE_TYPE(TREE_TYPE(FnDecl)),
                                                   DECL_ARGUMENTS(FnDecl),
                                                   static_chain,
                                                   CallingConv);
#ifdef TARGET_ADJUST_LLVM_CC
    TARGET_ADJUST_LLVM_CC(CallingConv, TREE_TYPE(FnDecl));
#endif
  } else {
    // Otherwise, just get the type from the function itself.
    FTy = TheTypeConverter->ConvertFunctionType(TREE_TYPE(FnDecl),
						static_chain,
						CallingConv);
  }
  
  // If we've already seen this function and created a prototype, and if the
  // proto has the right LLVM type, just use it.
  if (DECL_LLVM_SET_P(FnDecl) &&
      cast<PointerType>(DECL_LLVM(FnDecl)->getType())->getElementType() == FTy){
    Fn = cast<Function>(DECL_LLVM(FnDecl));
    assert(Fn->getCallingConv() == CallingConv &&
           "Calling convention disagreement between prototype and impl!");
    // The visibility can be changed from the last time we've seen this
    // function. Set to current.    
    if (TREE_PUBLIC(FnDecl) && DECL_VISIBILITY(FnDecl) == VISIBILITY_HIDDEN)
      Fn->setVisibility(Function::HiddenVisibility);
    else if (DECL_VISIBILITY(FnDecl) == VISIBILITY_DEFAULT)
      Fn->setVisibility(Function::DefaultVisibility);
  } else {
    Function *FnEntry = TheModule->getFunction(Name);
    if (FnEntry) {
      assert(FnEntry->getName() == Name && "Same entry, different name?");
      assert(FnEntry->isDeclaration() &&
             "Multiple fns with same name and neither are external!");
      FnEntry->setName("");  // Clear name to avoid conflicts.
      assert(FnEntry->getCallingConv() == CallingConv &&
             "Calling convention disagreement between prototype and impl!");
    }
    
    // Otherwise, either it exists with the wrong type or it doesn't exist.  In
    // either case create a new function.
    Fn = new Function(FTy, Function::ExternalLinkage, Name, TheModule);
    assert(Fn->getName() == Name && "Preexisting fn with the same name!");
    Fn->setCallingConv(CallingConv);
    
    // If a previous proto existed with the wrong type, replace any uses of it
    // with the actual function and delete the proto.
    if (FnEntry) {
      FnEntry->replaceAllUsesWith(ConstantExpr::getBitCast(Fn,
                                                           FnEntry->getType()));
      FnEntry->eraseFromParent();
    }
    SET_DECL_LLVM(FnDecl, Fn);
  }

  // The function should not already have a body.
  assert(Fn->empty() && "Function expanded multiple times!");
  
  // Compute the linkage that the function should get.
  if (!TREE_PUBLIC(FnDecl) /*|| lang_hooks.llvm_is_in_anon(subr)*/) {
    Fn->setLinkage(Function::InternalLinkage);
  } else if (DECL_DECLARED_INLINE_P(FnDecl)) {
    if (DECL_EXTERNAL(FnDecl)) {
      Fn->setLinkage(Function::LinkOnceLinkage);
    } else {
      Fn->setLinkage(Function::WeakLinkage);
    }
  } else if (DECL_WEAK(FnDecl) || DECL_ONE_ONLY(FnDecl)) {
    Fn->setLinkage(Function::WeakLinkage);
  } else if (DECL_COMDAT(FnDecl)) {
    Fn->setLinkage(Function::LinkOnceLinkage);
  }

#ifdef TARGET_ADJUST_LLVM_LINKAGE
  TARGET_ADJUST_LLVM_LINKAGE(Fn,FnDecl);
#endif /* TARGET_ADJUST_LLVM_LINKAGE */

  // Handle visibility style
  if (TREE_PUBLIC(FnDecl) && DECL_VISIBILITY(FnDecl) == VISIBILITY_HIDDEN)
    Fn->setVisibility(Function::HiddenVisibility);
  
  // Handle functions in specified sections.
  if (DECL_SECTION_NAME(FnDecl))
    Fn->setSection(TREE_STRING_POINTER(DECL_SECTION_NAME(FnDecl)));

  // Handle used Functions
  if (DECL_PRESERVE_P (FnDecl))
    AttributeUsedGlobals.push_back(Fn);
  
  // Create a new basic block for the function.
  CurBB = new BasicBlock("entry", Fn);
  
  if (TheDebugInfo) TheDebugInfo->EmitFunctionStart(FnDecl, Fn, CurBB);
  
  // Loop over all of the arguments to the function, setting Argument names and
  // creating argument alloca's for the PARM_DECLs in case their address is
  // exposed.
  Function::arg_iterator AI = Fn->arg_begin();

  // Rename and alloca'ify real arguments.
  FunctionPrologArgumentConversion Client(FnDecl, AI, CurBB);
  TheLLVMABI<FunctionPrologArgumentConversion> ABIConverter(Client);

  // Handle the DECL_RESULT.
  ABIConverter.HandleReturnType(TREE_TYPE(TREE_TYPE(FnDecl)));

  // Prepend the static chain (if any) to the list of arguments.
  tree Args = static_chain ? static_chain : DECL_ARGUMENTS(FnDecl);

  while (Args) {
    const char *Name = "unnamed_arg";
    if (DECL_NAME(Args)) Name = IDENTIFIER_POINTER(DECL_NAME(Args));

    if (isPassedByInvisibleReference(TREE_TYPE(Args))) {
      // If the value is passed by 'invisible reference', the l-value for the
      // argument IS the argument itself.
      SET_DECL_LLVM(Args, AI);
      AI->setName(Name);
      ++AI;
    } else {
      // Otherwise, we create an alloca to hold the argument value and provide
      // an l-value.  On entry to the function, we copy formal argument values
      // into the alloca.
      const Type *ArgTy = ConvertType(TREE_TYPE(Args));
      Value *Tmp = CreateTemporary(ArgTy);
      Tmp->setName(std::string(Name)+"_addr");
      SET_DECL_LLVM(Args, Tmp);
      if (TheDebugInfo) {
        TheDebugInfo->EmitDeclare(Args, llvm::dwarf::DW_TAG_arg_variable,
                                  Name, TREE_TYPE(Args), Tmp, CurBB);
      }

      Client.setName(Name);
      Client.setLocation(Tmp);
      ABIConverter.HandleArgument(TREE_TYPE(Args));
      Client.clear();
    }

    Args = Args == static_chain ? DECL_ARGUMENTS(FnDecl) : TREE_CHAIN(Args);
  }

  // If this is not a void-returning function, initialize the RESULT_DECL.
  if (DECL_RESULT(FnDecl) && !VOID_TYPE_P(TREE_TYPE(DECL_RESULT(FnDecl))) &&
      !DECL_LLVM_SET_P(DECL_RESULT(FnDecl)))
    EmitAutomaticVariableDecl(DECL_RESULT(FnDecl));

  // If this function has nested functions, we should handle a potential
  // nonlocal_goto_save_area.
  if (cfun->nonlocal_goto_save_area) {
    // Not supported yet.
  }
  
  // As it turns out, not all temporaries are associated with blocks.  For those
  // that aren't, emit them now.
  for (tree t = cfun->unexpanded_var_list; t; t = TREE_CHAIN(t)) {
    assert(!DECL_LLVM_SET_P(TREE_VALUE(t)) && "var already emitted?");
    EmitAutomaticVariableDecl(TREE_VALUE(t));
  }
  
  // Create a new block for the return node, but don't insert it yet.
  ReturnBB = new BasicBlock("return");
}

Function *TreeToLLVM::FinishFunctionBody() {
  // Insert the return block at the end of the function.
  EmitBlock(ReturnBB);
  
  Value *RetVal = 0;
  // If the function returns a value, get it into a register and return it now.
  if (Fn->getReturnType() != Type::VoidTy) {
    if (!isAggregateTreeType(TREE_TYPE(DECL_RESULT(FnDecl)))) {
      // If the DECL_RESULT is a scalar type, just load out the return value
      // and return it.
      tree TreeRetVal = DECL_RESULT(FnDecl);
      RetVal = new LoadInst(DECL_LLVM(TreeRetVal), "retval", CurBB);
      bool RetValSigned = !TYPE_UNSIGNED(TREE_TYPE(TreeRetVal));
      Instruction::CastOps opcode = CastInst::getCastOpcode(
          RetVal, RetValSigned, Fn->getReturnType(), RetValSigned);
      RetVal = CastToType(opcode, RetVal, Fn->getReturnType());
    } else {
      // Otherwise, this aggregate result must be something that is returned in
      // a scalar register for this target.  We must bit convert the aggregate
      // to the specified scalar type, which we do by casting the pointer and
      // loading.
      RetVal = BitCastToType(DECL_LLVM(DECL_RESULT(FnDecl)),
                             PointerType::get(Fn->getReturnType()));
      RetVal = new LoadInst(RetVal, "retval", CurBB);
    }
  }
  if (TheDebugInfo) TheDebugInfo->EmitRegionEnd(Fn, CurBB);
  new ReturnInst(RetVal, CurBB);
  
  // If this function has exceptions, emit the lazily created unwind block.
  if (UnwindBB) {
    EmitBlock(UnwindBB);
#ifdef ITANIUM_STYLE_EXCEPTIONS
    if (ExceptionValue) {
      // Fetch and store exception handler.
      std::vector<Value*> Args;
      Args.push_back(new LoadInst(ExceptionValue, "eh_ptr", CurBB));
      new CallInst(FuncUnwindResume,
                   &Args[0], Args.size(), "",
                   CurBB);
      new UnreachableInst(CurBB);
    }
#endif
    new UnwindInst(UnwindBB);
  }
  
  // If this function takes the address of a label, emit the indirect goto
  // block.
  if (IndirectGotoBlock) {
    EmitBlock(IndirectGotoBlock);
    
    // Change the default destination to go to one of the other destinations, if
    // there is any other dest.
    SwitchInst *SI = cast<SwitchInst>(IndirectGotoBlock->getTerminator());
    if (SI->getNumSuccessors() > 1)
      SI->setSuccessor(0, SI->getSuccessor(1));
  }
  
  return Fn;
}


Value *TreeToLLVM::Emit(tree exp, Value *DestLoc) {
  assert((isAggregateTreeType(TREE_TYPE(exp)) == (DestLoc != 0) ||
          TREE_CODE(exp) == MODIFY_EXPR) &&
         "Didn't pass DestLoc to an aggregate expr, or passed it to scalar!");
  
  Value *Result = 0;
  
  if (TheDebugInfo) {
    if (EXPR_HAS_LOCATION(exp)) {
      // Set new location on the way up the tree.
      TheDebugInfo->setLocationFile(EXPR_FILENAME(exp));
      TheDebugInfo->setLocationLine(EXPR_LINENO(exp));
    }
  
    // These node create an artificial jump to end of block.  
    if (TREE_CODE(exp) != BIND_EXPR && TREE_CODE(exp) != STATEMENT_LIST) {
      TheDebugInfo->EmitStopPoint(Fn, CurBB);
    }
  }
  
  switch (TREE_CODE(exp)) {
  default:
    std::cerr << "Unhandled expression!\n";
    debug_tree(exp);
    abort();
    
  // Basic lists and binding scopes
  case BIND_EXPR:      Result = EmitBIND_EXPR(exp, DestLoc); break;
  case STATEMENT_LIST: Result = EmitSTATEMENT_LIST(exp, DestLoc); break;

  // Control flow
  case LABEL_EXPR:     Result = EmitLABEL_EXPR(exp); break;
  case GOTO_EXPR:      Result = EmitGOTO_EXPR(exp); break;
  case RETURN_EXPR:    Result = EmitRETURN_EXPR(exp, DestLoc); break;
  case COND_EXPR:      Result = EmitCOND_EXPR(exp); break;
  case SWITCH_EXPR:    Result = EmitSWITCH_EXPR(exp); break;
  case TRY_FINALLY_EXPR:
  case TRY_CATCH_EXPR: Result = EmitTRY_EXPR(exp); break;
  case EXC_PTR_EXPR:   Result = EmitEXC_PTR_EXPR(exp); break;
  case CATCH_EXPR:     Result = EmitCATCH_EXPR(exp); break;
  case EH_FILTER_EXPR: Result = EmitEH_FILTER_EXPR(exp); break;
    
  // Expressions
  case VAR_DECL:
  case PARM_DECL:
  case RESULT_DECL:
  case INDIRECT_REF:
  case ARRAY_REF:
  case ARRAY_RANGE_REF:
  case COMPONENT_REF:
  case BIT_FIELD_REF:
  case STRING_CST:
  case REALPART_EXPR:
  case IMAGPART_EXPR:
    Result = EmitLoadOfLValue(exp, DestLoc);
    break;
  case OBJ_TYPE_REF:   Result = EmitOBJ_TYPE_REF(exp); break;
  case ADDR_EXPR:      Result = EmitADDR_EXPR(exp); break;
  case CALL_EXPR:      Result = EmitCALL_EXPR(exp, DestLoc); break;
  case MODIFY_EXPR:    Result = EmitMODIFY_EXPR(exp, DestLoc); break;
  case ASM_EXPR:       Result = EmitASM_EXPR(exp); break;
  case NON_LVALUE_EXPR: Result = Emit(TREE_OPERAND(exp, 0), DestLoc); break;

    // Unary Operators
  case NOP_EXPR:       Result = EmitNOP_EXPR(exp, DestLoc); break;
  case FIX_TRUNC_EXPR:
  case FLOAT_EXPR:
  case CONVERT_EXPR:   Result = EmitCONVERT_EXPR(exp, DestLoc); break;
  case VIEW_CONVERT_EXPR: Result = EmitVIEW_CONVERT_EXPR(exp, DestLoc); break;
  case NEGATE_EXPR:    Result = EmitNEGATE_EXPR(exp, DestLoc); break;
  case CONJ_EXPR:      Result = EmitCONJ_EXPR(exp, DestLoc); break;
  case ABS_EXPR:       Result = EmitABS_EXPR(exp); break;
  case BIT_NOT_EXPR:   Result = EmitBIT_NOT_EXPR(exp); break;
  case TRUTH_NOT_EXPR: Result = EmitTRUTH_NOT_EXPR(exp); break;

  // Binary Operators
  case LT_EXPR: 
    Result = EmitCompare(exp, ICmpInst::ICMP_ULT, ICmpInst::ICMP_SLT, 
                         FCmpInst::FCMP_OLT);
    break;
  case LE_EXPR:
    Result = EmitCompare(exp, ICmpInst::ICMP_ULE, ICmpInst::ICMP_SLE,
                         FCmpInst::FCMP_OLE);
    break;
  case GT_EXPR:
    Result = EmitCompare(exp, ICmpInst::ICMP_UGT, ICmpInst::ICMP_SGT,
                         FCmpInst::FCMP_OGT);
    break;
  case GE_EXPR:
    Result = EmitCompare(exp, ICmpInst::ICMP_UGE, ICmpInst::ICMP_SGE, 
                         FCmpInst::FCMP_OGE);
    break;
  case EQ_EXPR:
    Result = EmitCompare(exp, ICmpInst::ICMP_EQ, ICmpInst::ICMP_EQ, 
                         FCmpInst::FCMP_OEQ);
    break;
  case NE_EXPR:
    Result = EmitCompare(exp, ICmpInst::ICMP_NE, ICmpInst::ICMP_NE, 
                         FCmpInst::FCMP_UNE);
    break;
  case UNORDERED_EXPR: 
    Result = EmitCompare(exp, 0, 0, FCmpInst::FCMP_UNO);
    break;
  case ORDERED_EXPR: 
    Result = EmitCompare(exp, 0, 0, FCmpInst::FCMP_ORD);
    break;
  case UNLT_EXPR: Result = EmitCompare(exp, 0, 0, FCmpInst::FCMP_ULT); break;
  case UNLE_EXPR: Result = EmitCompare(exp, 0, 0, FCmpInst::FCMP_ULE); break;
  case UNGT_EXPR: Result = EmitCompare(exp, 0, 0, FCmpInst::FCMP_UGT); break;
  case UNGE_EXPR: Result = EmitCompare(exp, 0, 0, FCmpInst::FCMP_UGE); break;
  case UNEQ_EXPR: Result = EmitCompare(exp, 0, 0, FCmpInst::FCMP_UEQ); break;
  case LTGT_EXPR: Result = EmitCompare(exp, 0, 0, FCmpInst::FCMP_ONE); break;
  case PLUS_EXPR: Result = EmitBinOp(exp, DestLoc, Instruction::Add);break;
  case MINUS_EXPR:Result = EmitBinOp(exp, DestLoc, Instruction::Sub);break;
  case MULT_EXPR: Result = EmitBinOp(exp, DestLoc, Instruction::Mul);break;
  case TRUNC_DIV_EXPR: 
  case EXACT_DIV_EXPR:   // TODO: Optimize EXACT_DIV_EXPR.
    if (TYPE_UNSIGNED(TREE_TYPE(exp)))
      Result = EmitBinOp(exp, DestLoc, Instruction::UDiv);
    else 
      Result = EmitBinOp(exp, DestLoc, Instruction::SDiv);
    break;
  case RDIV_EXPR: Result = EmitBinOp(exp, DestLoc, Instruction::FDiv); break;
  case ROUND_DIV_EXPR: Result = EmitROUND_DIV_EXPR(exp); break;
  case TRUNC_MOD_EXPR: 
    if (TYPE_UNSIGNED(TREE_TYPE(exp)))
      Result = EmitBinOp(exp, DestLoc, Instruction::URem);
    else
      Result = EmitBinOp(exp, DestLoc, Instruction::SRem);
    break;
  case FLOOR_MOD_EXPR: Result = EmitFLOOR_MOD_EXPR(exp, DestLoc); break;
  case BIT_AND_EXPR:   Result = EmitBinOp(exp, DestLoc, Instruction::And);break;
  case BIT_IOR_EXPR:   Result = EmitBinOp(exp, DestLoc, Instruction::Or );break;
  case BIT_XOR_EXPR:   Result = EmitBinOp(exp, DestLoc, Instruction::Xor);break;
  case TRUTH_AND_EXPR: Result = EmitTruthOp(exp, Instruction::And); break;
  case TRUTH_OR_EXPR:  Result = EmitTruthOp(exp, Instruction::Or); break;
  case TRUTH_XOR_EXPR: Result = EmitTruthOp(exp, Instruction::Xor); break;
  case RSHIFT_EXPR:
    Result = EmitShiftOp(exp,DestLoc,
       TYPE_UNSIGNED(TREE_TYPE(exp)) ? Instruction::LShr : Instruction::AShr);
    break;
  case LSHIFT_EXPR:    Result = EmitShiftOp(exp,DestLoc,Instruction::Shl);break;
  case RROTATE_EXPR: 
    Result = EmitRotateOp(exp, Instruction::LShr, Instruction::Shl);
    break;
  case LROTATE_EXPR:
    Result = EmitRotateOp(exp, Instruction::Shl, Instruction::LShr);
    break;
  case MIN_EXPR: 
    Result = EmitMinMaxExpr(exp, ICmpInst::ICMP_ULE, ICmpInst::ICMP_SLE, 
                            FCmpInst::FCMP_OLE);
    break;
  case MAX_EXPR:
    Result = EmitMinMaxExpr(exp, ICmpInst::ICMP_UGE, ICmpInst::ICMP_SGE,
                            FCmpInst::FCMP_OGE);
    break;
  case CONSTRUCTOR:    Result = EmitCONSTRUCTOR(exp, DestLoc); break;
    
  // Complex Math Expressions.
  case COMPLEX_CST:    EmitCOMPLEX_CST (exp, DestLoc); break;
  case COMPLEX_EXPR:   EmitCOMPLEX_EXPR(exp, DestLoc); break;
    
  // Constant Expressions
  case INTEGER_CST:
    if (!DestLoc)
      Result = TreeConstantToLLVM::ConvertINTEGER_CST(exp);
    else
      EmitINTEGER_CST_Aggregate(exp, DestLoc);
    break;
  case REAL_CST:
    Result = TreeConstantToLLVM::ConvertREAL_CST(exp);
    break;
  case VECTOR_CST:
    Result = TreeConstantToLLVM::ConvertVECTOR_CST(exp);
    break;
  }
  
  if (TheDebugInfo && EXPR_HAS_LOCATION(exp)) {
    // Restore location back down the tree.
    TheDebugInfo->setLocationFile(EXPR_FILENAME(exp));
    TheDebugInfo->setLocationLine(EXPR_LINENO(exp));
  }

  assert(((DestLoc && Result == 0) || DestLoc == 0) &&
         "Expected a scalar or aggregate but got the wrong thing!"); 
  return Result;
}

/// EmitLV - Convert the specified l-value tree node to LLVM code, returning
/// the address of the result.
LValue TreeToLLVM::EmitLV(tree exp) {
  switch (TREE_CODE(exp)) {
  default:
    std::cerr << "Unhandled lvalue expression!\n";
    debug_tree(exp);
    abort();
  
  case PARM_DECL:
  case VAR_DECL:
  case FUNCTION_DECL:
  case CONST_DECL:
  case RESULT_DECL:   return EmitLV_DECL(exp);
  case ARRAY_RANGE_REF:
  case ARRAY_REF:     return EmitLV_ARRAY_REF(exp);
  case COMPONENT_REF: return EmitLV_COMPONENT_REF(exp);
  case BIT_FIELD_REF: return EmitLV_BIT_FIELD_REF(exp);
  case REALPART_EXPR: return EmitLV_XXXXPART_EXPR(exp, 0);
  case IMAGPART_EXPR: return EmitLV_XXXXPART_EXPR(exp, 1);

  // Constants.
  case LABEL_DECL:    return TreeConstantToLLVM::EmitLV_LABEL_DECL(exp);
  case STRING_CST:    return LValue(TreeConstantToLLVM::EmitLV_STRING_CST(exp));

  // Type Conversion.
  case VIEW_CONVERT_EXPR: return EmitLV_VIEW_CONVERT_EXPR(exp);

  // Trivial Cases.
  case WITH_SIZE_EXPR:
    // The address is the address of the operand.
    return EmitLV(TREE_OPERAND(exp, 0));
  case INDIRECT_REF:
    // The lvalue is just the address.
    return Emit(TREE_OPERAND(exp, 0), 0);
  }
}

//===----------------------------------------------------------------------===//
//                         ... Utility Functions ...
//===----------------------------------------------------------------------===//

void TreeToLLVM::TODO(tree exp) {
  std::cerr << "Unhandled tree node\n";
  if (exp) debug_tree(exp);
  abort();  
}

/// CastToType - Cast the specified value to the specified type if it is
/// not already that type.
Value *TreeToLLVM::CastToType(unsigned opcode, Value *V, const Type* Ty) {
  // Eliminate useless casts of a type to itself.
  if (V->getType() == Ty) 
    return V;

  if (Constant *C = dyn_cast<Constant>(V))
    return ConstantExpr::getCast(Instruction::CastOps(opcode), C, Ty);
  
  // Handle cast (cast bool X to T2) to bool as X, because this occurs all over
  // the place.
  if (CastInst *CI = dyn_cast<CastInst>(V))
    if (Ty == Type::Int1Ty && CI->getOperand(0)->getType() == Type::Int1Ty)
      return CI->getOperand(0);
  return CastInst::create(Instruction::CastOps(opcode), V, Ty, V->getName(), 
                          CurBB);
}

/// CastToAnyType - Cast the specified value to the specified type making no
/// assumptions about the types of the arguments. This creates an inferred cast.
Value *TreeToLLVM::CastToAnyType(Value *V, bool VisSigned, 
                                 const Type* Ty, bool TyIsSigned) {
  // Eliminate useless casts of a type to itself.
  if (V->getType() == Ty)
    return V;

  // The types are different so we must cast. Use getCastOpcode to create an
  // inferred cast opcode.
  Instruction::CastOps opc = 
    CastInst::getCastOpcode(V, VisSigned, Ty, TyIsSigned);

  // Generate the cast and return it.
  return CastToType(opc, V, Ty);
}

/// CastToUIntType - Cast the specified value to the specified type assuming
/// that the value and type are unsigned integer types.
Value *TreeToLLVM::CastToUIntType(Value *V, const Type* Ty) {
  // Eliminate useless casts of a type to itself.
  if (V->getType() == Ty)
    return V;

  unsigned SrcBits = V->getType()->getPrimitiveSizeInBits();
  unsigned DstBits = Ty->getPrimitiveSizeInBits();
  assert(SrcBits != DstBits && "Types are different but have same #bits?");

  Instruction::CastOps opcode = 
    (SrcBits > DstBits ? Instruction::Trunc : Instruction::ZExt);
  return CastToType(opcode, V, Ty);
}

/// CastToSIntType - Cast the specified value to the specified type assuming
/// that the value and type are signed integer types.
Value *TreeToLLVM::CastToSIntType(Value *V, const Type* Ty) {
  // Eliminate useless casts of a type to itself.
  if (V->getType() == Ty)
    return V;

  unsigned SrcBits = V->getType()->getPrimitiveSizeInBits();
  unsigned DstBits = Ty->getPrimitiveSizeInBits();
  assert(SrcBits != DstBits && "Types are different but have same #bits?");

  Instruction::CastOps opcode = 
    (SrcBits > DstBits ? Instruction::Trunc : Instruction::SExt);
  return CastToType(opcode, V, Ty);
}

/// CastToFPType - Cast the specified value to the specified type assuming
/// that the value and type or floating point
Value *TreeToLLVM::CastToFPType(Value *V, const Type* Ty) {
  unsigned SrcBits = V->getType()->getPrimitiveSizeInBits();
  unsigned DstBits = Ty->getPrimitiveSizeInBits();
  if (SrcBits == DstBits)
    return V;
  Instruction::CastOps opcode = (SrcBits > DstBits ? 
      Instruction::FPTrunc : Instruction::FPExt);
  return CastToType(opcode, V, Ty);
}

/// BitCastToType - Insert a BitCast from V to Ty if needed. This is just a 
/// shorthand convenience function for CastToType(Instruction::BitCast,V,Ty).
Value *TreeToLLVM::BitCastToType(Value *V, const Type *Ty) {
  return CastToType(Instruction::BitCast, V, Ty);
}

/// CreateTemporary - Create a new alloca instruction of the specified type,
/// inserting it into the entry block and returning it.  The resulting
/// instruction's type is a pointer to the specified type.
AllocaInst *TreeToLLVM::CreateTemporary(const Type *Ty) {
  if (AllocaInsertionPoint == 0) {
    // Create a dummy instruction in the entry block as a marker to insert new
    // alloc instructions before.  It doesn't matter what this instruction is,
    // it is dead.  This allows us to insert allocas in order without having to
    // scan for an insertion point. Use BitCast for int -> int
    AllocaInsertionPoint = CastInst::create(Instruction::BitCast,
      Constant::getNullValue(Type::Int32Ty), Type::Int32Ty, "alloca point");
    // Insert it as the first instruction in the entry block.
    Fn->begin()->getInstList().insert(Fn->begin()->begin(),
                                      AllocaInsertionPoint);
  }
  return new AllocaInst(Ty, 0, "memtmp", AllocaInsertionPoint);
}

/// EmitBlock - Add the specified basic block to the end of the function.  If
/// the previous block falls through into it, add an explicit branch.  Also,
/// manage fixups for EH info.
void TreeToLLVM::EmitBlock(BasicBlock *BB) {
  // If the previous block falls through to BB, add an explicit branch.
  if (CurBB->getTerminator() == 0) {
    // If the previous block has no label and is empty, remove it: it is a
    // post-terminator block.
    if (CurBB->getName().empty() && CurBB->begin() == CurBB->end()) {
      CurBB->eraseFromParent();
      
      if (!CurrentEHScopes.empty()) {
        assert(CurrentEHScopes.back().Blocks.back() == CurBB);
        CurrentEHScopes.back().Blocks.pop_back();
        BlockEHScope.erase(CurBB);
      }
    } else {
      // Otherwise, fall through to this block.
      new BranchInst(BB, CurBB);
    }
  }
  
  // Add this block.
  Fn->getBasicBlockList().push_back(BB);
  CurBB = BB;  // It is now the current block.

  // If there are no exception scopes that contain this block, exit.  This is
  // common for C++ code and almost uniformly true for C code.
  if (CurrentEHScopes.empty()) return;
  
  // Otherwise, we now know that this block is in this exception scope.  Update
  // records.
  CurrentEHScopes.back().Blocks.push_back(BB);
  BlockEHScope[BB] = CurrentEHScopes.size()-1;
}

/// CopyAggregate - Recursively traverse the potientially aggregate src/dest
/// ptrs, copying all of the elements.
static void CopyAggregate(Value *DestPtr, Value *SrcPtr, 
                          bool isDstVolatile, bool isSrcVolatile,
                          BasicBlock *CurBB) {
  assert(DestPtr->getType() == SrcPtr->getType() &&
         "Cannot copy between two pointers of different type!");
  const Type *ElTy = cast<PointerType>(DestPtr->getType())->getElementType();
  if (ElTy->isFirstClassType()) {
    Value *V = new LoadInst(SrcPtr, "tmp", isSrcVolatile, CurBB);
    new StoreInst(V, DestPtr, isDstVolatile, CurBB);
  } else if (const StructType *STy = dyn_cast<StructType>(ElTy)) {
    Constant *Zero = ConstantInt::get(Type::Int32Ty, 0);
    for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i) {
      Constant *Idx = ConstantInt::get(Type::Int32Ty, i);
      Value *DElPtr = new GetElementPtrInst(DestPtr, Zero, Idx, "tmp", CurBB);
      Value *SElPtr = new GetElementPtrInst(SrcPtr, Zero, Idx, "tmp", CurBB);
      CopyAggregate(DElPtr, SElPtr, isDstVolatile, isSrcVolatile, CurBB);
    }
  } else {
    const ArrayType *ATy = cast<ArrayType>(ElTy);
    Constant *Zero = ConstantInt::get(Type::Int32Ty, 0);
    for (unsigned i = 0, e = ATy->getNumElements(); i != e; ++i) {
      Constant *Idx = ConstantInt::get(Type::Int32Ty, i);
      Value *DElPtr = new GetElementPtrInst(DestPtr, Zero, Idx, "tmp", CurBB);
      Value *SElPtr = new GetElementPtrInst(SrcPtr, Zero, Idx, "tmp", CurBB);
      CopyAggregate(DElPtr, SElPtr, isDstVolatile, isSrcVolatile, CurBB);
    }
  }
}

/// CountAggregateElements - Return the number of elements in the specified type
/// that will need to be loaded/stored if we copy this by explicit accesses.
static unsigned CountAggregateElements(const Type *Ty) {
  if (Ty->isFirstClassType()) return 1;

  if (const StructType *STy = dyn_cast<StructType>(Ty)) {
    unsigned NumElts = 0;
    for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i)
      NumElts += CountAggregateElements(STy->getElementType(i));
    return NumElts;
  } else {
    const ArrayType *ATy = cast<ArrayType>(Ty);
    return ATy->getNumElements()*CountAggregateElements(ATy->getElementType());
  }
}

/// EmitAggregateCopy - Copy the elements from SrcPtr to DestPtr, using the
/// GCC type specified by GCCType to know which elements to copy.
void TreeToLLVM::EmitAggregateCopy(Value *DestPtr, Value *SrcPtr, tree type,
                                   bool isDstVolatile, bool isSrcVolatile) {
  if (DestPtr == SrcPtr && !isDstVolatile && !isSrcVolatile)
    return;  // noop copy.

  // If the type is small, copy the elements instead of using a block copy.
  if (TREE_CODE(TYPE_SIZE(type)) == INTEGER_CST &&
      TREE_INT_CST_LOW(TYPE_SIZE_UNIT(type)) < 64) {
    const Type *LLVMTy = ConvertType(type);
    if (CountAggregateElements(LLVMTy) <= 8) {
      DestPtr = CastToType(Instruction::BitCast, DestPtr, 
                           PointerType::get(LLVMTy));
      SrcPtr = CastToType(Instruction::BitCast, SrcPtr, 
                          PointerType::get(LLVMTy));
      
      // FIXME: Is this always safe?  The LLVM type might theoretically have
      // holes or might be suboptimal to copy this way.  It may be better to
      // copy the structure by the GCCType's fields.
      CopyAggregate(DestPtr, SrcPtr, isDstVolatile, isSrcVolatile, CurBB);
      return;
    }
  }
  
  unsigned Alignment = TYPE_ALIGN_OK(type) ? (TYPE_ALIGN_UNIT(type) & ~0U) : 0;
  Value *TypeSize = Emit(TYPE_SIZE_UNIT(type), 0);
  EmitMemCpy(DestPtr, SrcPtr, TypeSize, Alignment);
}

/// ZeroAggregate - Recursively traverse the potientially aggregate dest
/// ptr, zero'ing all of the elements.
static void ZeroAggregate(Value *DestPtr, BasicBlock *CurBB) {
  const Type *ElTy = cast<PointerType>(DestPtr->getType())->getElementType();
  if (ElTy->isFirstClassType()) {
    new StoreInst(Constant::getNullValue(ElTy), DestPtr, CurBB);
  } else if (const StructType *STy = dyn_cast<StructType>(ElTy)) {
    Constant *Zero = ConstantInt::get(Type::Int32Ty, 0);
    for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i) {
      Constant *Idx = ConstantInt::get(Type::Int32Ty, i);
      Value *DElPtr = new GetElementPtrInst(DestPtr, Zero, Idx, "tmp", CurBB);
      ZeroAggregate(DElPtr, CurBB);
    }
  } else {
    const ArrayType *ATy = cast<ArrayType>(ElTy);
    Constant *Zero = ConstantInt::get(Type::Int32Ty, 0);
    for (unsigned i = 0, e = ATy->getNumElements(); i != e; ++i) {
      Constant *Idx = ConstantInt::get(Type::Int32Ty, i);
      Value *DElPtr = new GetElementPtrInst(DestPtr, Zero, Idx, "tmp", CurBB);
      ZeroAggregate(DElPtr, CurBB);
    }
  }
}

/// EmitAggregateZero - Zero the elements of DestPtr.
///
void TreeToLLVM::EmitAggregateZero(Value *DestPtr, tree type) {
  // If the type is small, copy the elements instead of using a block copy.
  if (TREE_CODE(TYPE_SIZE(type)) == INTEGER_CST &&
      TREE_INT_CST_LOW(TYPE_SIZE_UNIT(type)) < 128) {
    const Type *LLVMTy = ConvertType(type);
    DestPtr = CastToType(Instruction::BitCast, DestPtr, 
                         PointerType::get(LLVMTy));
    
    // FIXME: Is this always safe?  The LLVM type might theoretically have holes
    // or might be suboptimal to copy this way.  It may be better to copy the
    // structure by the GCCType's fields.
    ZeroAggregate(DestPtr, CurBB);
    return;
  }

  unsigned Alignment = TYPE_ALIGN_OK(type) ? (TYPE_ALIGN_UNIT(type) & ~0U) : 0;
  EmitMemSet(DestPtr, ConstantInt::get(Type::Int8Ty, 0),
             Emit(TYPE_SIZE_UNIT(type), 0), Alignment);
}

void TreeToLLVM::EmitMemCpy(Value *DestPtr, Value *SrcPtr, Value *Size, 
                            unsigned Align) {
  const Type *SBP = PointerType::get(Type::Int8Ty);
  const Type *IntPtr = TD.getIntPtrType();
  Value *Ops[4] = {
    CastToType(Instruction::BitCast, DestPtr, SBP),
    CastToType(Instruction::BitCast, SrcPtr, SBP),
    CastToSIntType(Size, IntPtr),
    ConstantInt::get(Type::Int32Ty, Align)
  };

  new CallInst(Intrinsic::getDeclaration(TheModule, 
                                         (IntPtr == Type::Int32Ty) ?
                                         Intrinsic::memcpy_i32 :
                                         Intrinsic::memcpy_i64),
               Ops, 4, "", CurBB);
}

void TreeToLLVM::EmitMemMove(Value *DestPtr, Value *SrcPtr, Value *Size, 
                             unsigned Align) {
  const Type *SBP = PointerType::get(Type::Int8Ty);
  const Type *IntPtr = TD.getIntPtrType();
  Value *Ops[4] = {
    CastToType(Instruction::BitCast, DestPtr, SBP),
    CastToType(Instruction::BitCast, SrcPtr, SBP),
    CastToSIntType(Size, IntPtr),
    ConstantInt::get(Type::Int32Ty, Align)
  };

  new CallInst(Intrinsic::getDeclaration(TheModule,
                                         (IntPtr == Type::Int32Ty) ?
                                         Intrinsic::memmove_i32 :
                                         Intrinsic::memmove_i64),
               Ops, 4, "", CurBB);
}

void TreeToLLVM::EmitMemSet(Value *DestPtr, Value *SrcVal, Value *Size, 
                            unsigned Align) {
  const Type *SBP = PointerType::get(Type::Int8Ty);
  const Type *IntPtr = TD.getIntPtrType();
  Value *Ops[4] = {
    CastToType(Instruction::BitCast, DestPtr, SBP),
    CastToSIntType(SrcVal, Type::Int8Ty),
    CastToSIntType(Size, IntPtr),
    ConstantInt::get(Type::Int32Ty, Align)
  };

  new CallInst(Intrinsic::getDeclaration(TheModule,
                                         (IntPtr == Type::Int32Ty) ?
                                         Intrinsic::memset_i32 :
                                         Intrinsic::memset_i64),
               Ops, 4, "", CurBB);
}


/// EmitBranchInternal - Emit an unconditional branch to the specified basic
/// block, running cleanups if the branch exits scopes.  The arguments specify
/// how to handle these cleanups.
///
/// This function is used for a variety of control flow purposes. In particular,
/// it is responsible for determining which cleanups must be executed as a
/// result of leaving blocks with destructors.  For branches that require
/// cleanups, it schedules cleanup insertion with a goto_fixup record.  When the
/// block containing the cleanup is exited, the end-of-block code inserts the
/// cleanups as indicated by goto_fixups.
///
/// Note that some cleanups only apply to exception edges.  If this is an
/// exception edge (as indicated by IsExceptionEdge) these are expanded,
/// otherwise not.
///
/// Note that it is illegal for inserted cleanups to throw any exceptions,
/// as indicated by isExceptionEdge.  This is an important case for
/// exception handling: when unwinding the stack due to an active exception, any
/// destructors which propagate exceptions should cause terminate to be called.
/// Thus, if a cleanup does throw an exception, its exception destination must
/// go to a designated terminate block.
///
/// Note that all calling code should emit a new basic block after this, so that
/// future code does not fall after the terminator.
///
void TreeToLLVM::EmitBranchInternal(BasicBlock *Dest, bool IsExceptionEdge) {
  // Insert the branch.
  BranchInst *BI = new BranchInst(Dest, CurBB);
  
  // If there are no current exception scopes, this edge *couldn't* need 
  // cleanups.  It is not possible to jump into a scope that requires a cleanup.
  // This keeps the C case fast.
  if (CurrentEHScopes.empty()) return;

  // If the destination block has already been emitted, this is a backwards
  // branch, and we can resolve it now.
  if (Dest->getParent()) {
    // This is a forward reference to a block.  Since we know that we can't jump
    // INTO a region that has cleanups, we can only be branching out.
    //
    std::map<BasicBlock*, unsigned>::iterator I = BlockEHScope.find(Dest);
    if (I != BlockEHScope.end() && I->second == CurrentEHScopes.size())
      return;  // Branch within the same EH scope.
    
    assert((I == BlockEHScope.end() || I->second < CurrentEHScopes.size()) &&
           "Invalid branch into EH region");
  }

  AddBranchFixup(BI, IsExceptionEdge);
}

/// AddBranchFixup - Add the specified unconditional branch to the fixup list
/// for the outermost exception scope, merging it if there is already a fixup
/// that works.
void TreeToLLVM::AddBranchFixup(BranchInst *BI, bool isExceptionEdge) {
  BasicBlock *Dest = BI->getSuccessor(0);
  
  // Check to see if we already have a fixup for this destination.
  std::vector<BranchFixup> &BranchFixups = CurrentEHScopes.back().BranchFixups;
  for (unsigned i = 0, e = BranchFixups.size(); i != e; ++i)
    if (BranchFixups[i].SrcBranch->getSuccessor(0) == Dest &&
        BranchFixups[i].isExceptionEdge) {
      BranchFixup &Fixup = BranchFixups[i];
      // We found a fixup for this destination already.  Recycle it.
      if (&Fixup.SrcBranch->getParent()->front() == Fixup.SrcBranch) {
        // If the fixup's branch is the only instruction in its block, change
        // the branch we just emitted to branch to that block instead.
        BI->setSuccessor(0, Fixup.SrcBranch->getParent());
        return;
      }
      
      if (&CurBB->front() == BI) {
        // Otherwise, if this block is empty except for the branch, change the
        // fixup to jump here and change the fixup to fix this branch.
        Fixup.SrcBranch->setSuccessor(0, CurBB);
        Fixup.SrcBranch = BI;
        return;
      }
      
      // Finally, if neither block is empty, create a new (empty) one and
      // revector BOTH branches to the new block.
      EmitBlock(new BasicBlock("cleanup"));
      BranchInst *NewBI = new BranchInst(Dest, CurBB);
      BI->setSuccessor(0, CurBB);
      Fixup.SrcBranch->setSuccessor(0, CurBB);
      Fixup.SrcBranch = NewBI;
      return;
    }
  
  // Otherwise, create a new fixup for this branch so we know that it needs
  // cleanups when we finish up the scopes that it is in.
  BranchFixups.push_back(BranchFixup(BI, isExceptionEdge));
}

//===----------------------------------------------------------------------===//
//                  ... Basic Lists and Binding Scopes ...
//===----------------------------------------------------------------------===//

/// EmitAutomaticVariableDecl - Emit the function-local decl to the current
/// function and set DECL_LLVM for the decl to the right pointer.
void TreeToLLVM::EmitAutomaticVariableDecl(tree decl) {
  tree type = TREE_TYPE(decl);
  
  // An LLVM value pointer for this decl may already be set, for example, if the
  // named return value optimization is being applied to this function, and
  // this variable is the one being returned.
  assert(!DECL_LLVM_SET_P(decl) && "Shouldn't call this on an emitted var!");
  
  // For a CONST_DECL, set mode, alignment, and sizes from those of the
  // type in case this node is used in a reference.
  if (TREE_CODE(decl) == CONST_DECL) {
    DECL_MODE(decl)      = TYPE_MODE(type);
    DECL_ALIGN(decl)     = TYPE_ALIGN(type);
    DECL_SIZE(decl)      = TYPE_SIZE(type);
    DECL_SIZE_UNIT(decl) = TYPE_SIZE_UNIT(type);
    return;
  }
  
  // Otherwise, only automatic (and result) variables need any expansion done.
  // Static and external variables, and external functions, will be handled by
  // `assemble_variable' (called from finish_decl).  TYPE_DECL requires nothing.
  // PARM_DECLs are handled in `llvm_expand_function_start'.
  if ((TREE_CODE(decl) != VAR_DECL && TREE_CODE(decl) != RESULT_DECL) ||
      TREE_STATIC(decl) || DECL_EXTERNAL(decl) || type == error_mark_node)
    return;

  // SSA temporaries are handled specially: their DECL_LLVM is set when the
  // definition is encountered.
  if (isGCC_SSA_Temporary(decl))
    return;
  
  // If this is just the rotten husk of a variable that the gimplifier
  // eliminated all uses of, but is preserving for debug info, ignore it.
  if (TREE_CODE(decl) == VAR_DECL && DECL_VALUE_EXPR(decl))
    return;
  
  const Type *Ty;         // Type to allocate
  Value *Size = 0;        // Amount to alloca (null for 1)
  unsigned Alignment = 0; // Alignment in bytes.
  
  if (DECL_SIZE(decl) == 0) {    // Variable with incomplete type.
    if (DECL_INITIAL(decl) == 0)
      return; // Error message was already done; now avoid a crash.
    else {
      // "An initializer is going to decide the size of this array."??
      TODO(decl);
      abort();
    }
  } else if (TREE_CODE(DECL_SIZE_UNIT(decl)) == INTEGER_CST) {
    // Variable of fixed size that goes on the stack.
    Ty = ConvertType(type);
    
    // Set alignment we actually gave this decl.
    if (DECL_MODE(decl) == BLKmode)
      DECL_ALIGN(decl) = BIGGEST_ALIGNMENT;
    else
      DECL_ALIGN(decl) = GET_MODE_BITSIZE(DECL_MODE(decl));
    DECL_USER_ALIGN(decl) = 0;
    Alignment = DECL_ALIGN(decl)/8;
  } else {
    tree length;

    // Dynamic-size object: must push space on the stack.
    if (TREE_CODE(type) == ARRAY_TYPE && (length = arrayLength(type))) {
      Ty = ConvertType(TREE_TYPE(type));  // Get array element type.
      // Compute the number of elements in the array.
      Size = Emit(length, 0);
      Size = CastToUIntType(Size, Size->getType());
    } else {
      // Compute the variable's size in bytes.
      Size = CastToUIntType(Emit(DECL_SIZE_UNIT(decl), 0), Type::Int32Ty);
      Ty = Type::Int8Ty;
    }
  }
  
  const char *Name;      // Name of variable
  if (DECL_NAME(decl))
    Name = IDENTIFIER_POINTER(DECL_NAME(decl));
  else if (TREE_CODE(decl) == RESULT_DECL)
    Name = "retval";
  else
    Name = "tmp";

  // Insert an alloca for this variable.
  AllocaInst *AI;
  if (!Size) {                           // Fixed size alloca -> entry block.
    AI = CreateTemporary(Ty);
    AI->setName(Name);
  } else {
    AI = new AllocaInst(Ty, Size, Name, CurBB);
  }
  
  AI->setAlignment(Alignment);
  
  SET_DECL_LLVM(decl, AI);
  
  if (TheDebugInfo) {
    if (DECL_NAME(decl)) {
      TheDebugInfo->EmitDeclare(decl, llvm::dwarf::DW_TAG_auto_variable,
                                Name, TREE_TYPE(decl), AI, CurBB);
    } else if (TREE_CODE(decl) == RESULT_DECL) {
      TheDebugInfo->EmitDeclare(decl, llvm::dwarf::DW_TAG_return_variable,
                                Name, TREE_TYPE(decl), AI, CurBB);
    }
  }
}

Value *TreeToLLVM::EmitBIND_EXPR(tree exp, Value *DestLoc) {
  // Start region only if not top level.
  if (TheDebugInfo && DECL_SAVED_TREE(FnDecl) != exp) 
    TheDebugInfo->EmitRegionStart(Fn, CurBB);
  
  // Mark the corresponding BLOCK for output in its proper place.
  if (BIND_EXPR_BLOCK(exp) != 0 && !TREE_USED(BIND_EXPR_BLOCK(exp)))
    TREE_USED(BIND_EXPR_BLOCK(exp)) = 1;
  //lang_hooks.decls.insert_block(BIND_EXPR_BLOCK(exp));

  // If VARS have not yet been expanded, expand them now.
  tree Var = BIND_EXPR_VARS(exp);
  for (; Var; Var = TREE_CHAIN(Var)) {
    if (TREE_STATIC(Var)) {
      // If this is an inlined copy of a static local variable, look up the
      // original.
      tree RealVar = DECL_ORIGIN(Var);
      
      // If we haven't already emitted the var, do so now.
      if (!TREE_ASM_WRITTEN(RealVar) && !lang_hooks.expand_decl(RealVar) &&
          TREE_CODE (Var) == VAR_DECL)
        rest_of_decl_compilation(RealVar, 0, 0);
      continue;
    }

    // Otherwise, if this is an automatic variable that hasn't been emitted, do
    // so now.
    if (!DECL_LLVM_SET_P(Var))
      EmitAutomaticVariableDecl(Var);
  }

  // Finally, emit the body of the bind expression.
  Value *Result = Emit(BIND_EXPR_BODY(exp), DestLoc);

  TREE_USED(exp) = 1;

  // End region only if not top level.
  if (TheDebugInfo && DECL_SAVED_TREE(FnDecl) != exp) 
    TheDebugInfo->EmitRegionEnd(Fn, CurBB);

  return Result;
}

Value *TreeToLLVM::EmitSTATEMENT_LIST(tree exp, Value *DestLoc) {
  assert(DestLoc == 0 && "Does not return a value!");

  // Convert each statement.
  for (tree_stmt_iterator I = tsi_start(exp); !tsi_end_p(I); tsi_next(&I)) {
    tree stmt = tsi_stmt(I);
    Value *DestLoc = 0;
    
    // If this stmt returns an aggregate value (e.g. a call whose result is
    // ignored), create a temporary to receive the value.  Note that we don't
    // do this for MODIFY_EXPRs as an efficiency hack.
    if (isAggregateTreeType(TREE_TYPE(stmt)) && TREE_CODE(stmt) != MODIFY_EXPR)
      DestLoc = CreateTemporary(ConvertType(TREE_TYPE(stmt)));

    Emit(stmt, DestLoc);
  }
  return 0;
}

//===----------------------------------------------------------------------===//
//                ... Address Of Labels Extension Support ...
//===----------------------------------------------------------------------===//

/// getIndirectGotoBlockNumber - Return the unique ID of the specified basic
/// block for uses that take the address of it.
Constant *TreeToLLVM::getIndirectGotoBlockNumber(BasicBlock *BB) {
  ConstantInt *&Val = AddressTakenBBNumbers[BB];
  if (Val) return Val;
  
  // Assign the new ID, update AddressTakenBBNumbers to remember it.
  uint64_t BlockNo = ++NumAddressTakenBlocks;
  BlockNo &= ~0ULL >> (64-TD.getPointerSizeInBits());
  Val = ConstantInt::get(TD.getIntPtrType(), BlockNo);

  // Add it to the switch statement in the indirect goto block.
  cast<SwitchInst>(getIndirectGotoBlock()->getTerminator())->addCase(Val, BB);
  return Val;
}

/// getIndirectGotoBlock - Get (and potentially lazily create) the indirect
/// goto block.
BasicBlock *TreeToLLVM::getIndirectGotoBlock() {
  if (IndirectGotoBlock) return IndirectGotoBlock;

  // Create a temporary for the value to be switched on.
  IndirectGotoValue = CreateTemporary(TD.getIntPtrType());
  
  // Create the block, emit a load, and emit the switch in the block.
  IndirectGotoBlock = new BasicBlock("indirectgoto");
  Value *Ld = new LoadInst(IndirectGotoValue, "gotodest", IndirectGotoBlock);
  new SwitchInst(Ld, IndirectGotoBlock, 0, IndirectGotoBlock);
  
  // Finally, return it.
  return IndirectGotoBlock;
}


//===----------------------------------------------------------------------===//
//                           ... Control Flow ...
//===----------------------------------------------------------------------===//

/// getLabelDeclBlock - Lazily get and create a basic block for the specified
/// label.
static BasicBlock *getLabelDeclBlock(tree LabelDecl) {
  assert(TREE_CODE(LabelDecl) == LABEL_DECL && "Isn't a label!?");
  if (DECL_LLVM_SET_P(LabelDecl))
    return cast<BasicBlock>(DECL_LLVM(LabelDecl));

  const char *Name = "bb";
  if (DECL_NAME(LabelDecl))
    Name = IDENTIFIER_POINTER(DECL_NAME(LabelDecl));
  
  BasicBlock *NewBB = new BasicBlock(Name);
  SET_DECL_LLVM(LabelDecl, NewBB);
  return NewBB;
}

/// EmitLABEL_EXPR - Emit the basic block corresponding to the specified label.
///
Value *TreeToLLVM::EmitLABEL_EXPR(tree exp) {
  EmitBlock(getLabelDeclBlock(TREE_OPERAND(exp, 0)));
  return 0;
}

Value *TreeToLLVM::EmitGOTO_EXPR(tree exp) {
  if (TREE_CODE(TREE_OPERAND(exp, 0)) == LABEL_DECL) {
    // Direct branch.
    EmitBranchInternal(getLabelDeclBlock(TREE_OPERAND(exp, 0)), false);
  } else {

    // Otherwise we have an indirect goto.
    BasicBlock *DestBB = getIndirectGotoBlock();

    // Store the destination block to the GotoValue alloca.
    Value *V = Emit(TREE_OPERAND(exp, 0), 0);
    V = CastToType(Instruction::PtrToInt, V, TD.getIntPtrType());
    new StoreInst(V, IndirectGotoValue, CurBB);
    
    // NOTE: This is HORRIBLY INCORRECT in the presence of exception handlers.
    // There should be one collector block per cleanup level!  Note that
    // standard GCC gets this wrong as well.
    //
    EmitBranchInternal(DestBB, false);
  }
  EmitBlock(new BasicBlock(""));
  return 0;
}


Value *TreeToLLVM::EmitRETURN_EXPR(tree exp, Value *DestLoc) {
  assert(DestLoc == 0 && "Does not return a value!");
  if (TREE_OPERAND(exp, 0)) {
    // Emit the expression, including the assignment to RESULT_DECL.  If the
    // operand is an aggregate value, create a temporary to evaluate it into.
    Value *DestLoc = 0;
    const Type *DestTy = ConvertType(TREE_TYPE(TREE_OPERAND(exp, 0)));
    if (!DestTy->isFirstClassType() && TREE_CODE(exp) != MODIFY_EXPR)
      DestLoc = CreateTemporary(DestTy);
    Emit(TREE_OPERAND(exp, 0), DestLoc);
  }

  // Emit a branch to the exit label.
  EmitBranchInternal(ReturnBB, false);
  EmitBlock(new BasicBlock(""));
  return 0;
}

Value *TreeToLLVM::EmitCOND_EXPR(tree exp) {
  // Emit the conditional expression and trunc/bitcast to Int1Ty
  Value *Cond = Emit(COND_EXPR_COND(exp), 0);
  // If its not already a bool, insert a comparison against zero to make it so.
  if (Cond->getType() != Type::Int1Ty)
    Cond = new ICmpInst(ICmpInst::ICMP_NE, Cond, 
                        Constant::getNullValue(Cond->getType()), "toBool", 
                           CurBB);
  tree Then = COND_EXPR_THEN(exp);
  tree Else = COND_EXPR_ELSE(exp);

  // One extremely common pattern produced by the loop lowering code are 
  // COND_EXPRS that look like:
  //
  //    if (cond) { goto <D905>; } else { goto <D907>; }
  //
  // The generic code handles this below, but there is no reason to create a
  // cond branch to two blocks which just contain branches themselves.
  // Note that we only do this if we're not in the presence of C++ exceptions.
  // C++ exceptions could require information for the edge, which requires the
  // uncond branch to be available.
  if (CurrentEHScopes.empty() && TREE_CODE(Then) == STATEMENT_LIST &&
      TREE_CODE(Else) == STATEMENT_LIST) {
    tree_stmt_iterator ThenI = tsi_start(Then), ElseI = tsi_start(Else);
    if (!tsi_end_p(ThenI) && !tsi_end_p(ElseI)) { // {} isn't empty.
      tree ThenStmt = tsi_stmt(ThenI), ElseStmt = tsi_stmt(ElseI);
      tsi_next(&ThenI);
      tsi_next(&ElseI);
      
      if (TREE_CODE(ThenStmt) == GOTO_EXPR &&      // Found two uncond gotos.
          TREE_CODE(ElseStmt) == GOTO_EXPR &&
          tsi_end_p(ThenI) && tsi_end_p(ElseI) &&  // Nothing after them.
          TREE_CODE(TREE_OPERAND(ThenStmt, 0)) == LABEL_DECL &&// Not goto *p.
          TREE_CODE(TREE_OPERAND(ElseStmt, 0)) == LABEL_DECL) {
        BasicBlock *ThenDest = getLabelDeclBlock(TREE_OPERAND(ThenStmt, 0));
        BasicBlock *ElseDest = getLabelDeclBlock(TREE_OPERAND(ElseStmt, 0));
        
        // Okay, we have success. Output the conditional branch.
        new BranchInst(ThenDest, ElseDest, Cond, CurBB);
        // Emit a "fallthrough" block, which is almost certainly dead.
        EmitBlock(new BasicBlock(""));
        return 0;
      }
    }
  }
  
  BasicBlock *TrueBlock = new BasicBlock("cond_true");
  BasicBlock *FalseBlock;
  BasicBlock *ContBlock = new BasicBlock("cond_next");
  
  // Another extremely common case we want to handle are if/then blocks with
  // no else.  The gimplifier turns these into:
  //
  //  if (cond) { goto <D905>; } else { }
  //
  // Recognize when the else is an empty STATEMENT_LIST, and don't emit the
  // else if so.
  //
  bool HasEmptyElse =
    TREE_CODE(Else) == STATEMENT_LIST && tsi_end_p(tsi_start(Else));

  if (HasEmptyElse)
    FalseBlock = ContBlock;
  else
    FalseBlock = new BasicBlock("cond_false");

  // Emit the branch based on the condition.
  new BranchInst(TrueBlock, FalseBlock, Cond, CurBB);
  
  // Emit the true code.
  EmitBlock(TrueBlock);
  
  Emit(Then, 0);
  
  // If this is an if/then/else cond-expr, emit the else part, otherwise, just
  // fall through to the ContBlock.
  if (!HasEmptyElse) {
    if (CurBB->getTerminator() == 0 &&
        (!CurBB->getName().empty() || 
         !CurBB->use_empty()))
      new BranchInst(ContBlock, CurBB);  // Branch to continue block.

    EmitBlock(FalseBlock);
    
    Emit(Else, 0);
  }
  
  EmitBlock(ContBlock);
  return 0;
}

Value *TreeToLLVM::EmitSWITCH_EXPR(tree exp) {
  tree Cases = SWITCH_LABELS(exp);
  
  // Emit the condition.
  Value *SwitchExp = Emit(SWITCH_COND(exp), 0);
  bool ExpIsSigned = !TYPE_UNSIGNED(TREE_TYPE(SWITCH_COND(exp)));
  
  // Emit the switch instruction.
  SwitchInst *SI = new SwitchInst(SwitchExp, CurBB,
                                  TREE_VEC_LENGTH(Cases), CurBB);
  EmitBlock(new BasicBlock(""));
  SI->setSuccessor(0, CurBB);   // Default location starts out as fall-through

  assert(!SWITCH_BODY(exp) && "not a gimple switch?");

  BasicBlock *DefaultDest = NULL;
  for (unsigned i = 0, e = TREE_VEC_LENGTH(Cases); i != e; ++i) {
    BasicBlock *Dest = getLabelDeclBlock(CASE_LABEL(TREE_VEC_ELT(Cases, i)));

    tree low = CASE_LOW(TREE_VEC_ELT(Cases, i));
    if (!low) {
      DefaultDest = Dest;
      continue;
    }

    // Convert the integer to the right type.
    Value *Val = Emit(low, 0);
    Val = CastToAnyType(Val, !TYPE_UNSIGNED(TREE_TYPE(low)),
                        SwitchExp->getType(), ExpIsSigned);
    ConstantInt *LowC = cast<ConstantInt>(Val);

    tree high = CASE_HIGH(TREE_VEC_ELT(Cases, i));
    if (!high) {
      SI->addCase(LowC, Dest); // Single destination.
      continue;
    }

    // Otherwise, we have a range, like 'case 1 ... 17'.
    Val = Emit(high, 0);
    // Make sure the case value is the same type as the switch expression
    Val = CastToAnyType(Val, !TYPE_UNSIGNED(TREE_TYPE(high)),
                        SwitchExp->getType(), ExpIsSigned);
    ConstantInt *HighC = cast<ConstantInt>(Val);

    APInt Range = HighC->getValue() - LowC->getValue();
    if (Range.ult(APInt(Range.getBitWidth(), 64))) {
      // Add all of the necessary successors to the switch.
      APInt CurrentValue = LowC->getValue();
      while (1) {
        SI->addCase(LowC, Dest);
        if (LowC == HighC) break;  // Emitted the last one.
        CurrentValue++;
        LowC = ConstantInt::get(CurrentValue);
      }
    } else {
      // The range is too big to add to the switch - emit an "if".
      Value *Diff = BinaryOperator::create(Instruction::Sub, SwitchExp, LowC,
                                           "tmp", CurBB);
      Value *Cond = new ICmpInst(ICmpInst::ICMP_ULE, Diff,
                                 ConstantInt::get(Range), "tmp", CurBB);
      BasicBlock *False_Block = new BasicBlock("case_false");
      new BranchInst(Dest, False_Block, Cond, CurBB);
      EmitBlock(False_Block);
    }
  }

  if (DefaultDest)
    if (SI->getSuccessor(0) == CurBB)
      SI->setSuccessor(0, DefaultDest);
    else {
      new BranchInst(DefaultDest, CurBB);
      // Emit a "fallthrough" block, which is almost certainly dead.
      EmitBlock(new BasicBlock(""));
    }

  return 0;
}

#ifndef NDEBUG
void TreeToLLVM::dumpEHScopes() const {
  std::cerr << CurrentEHScopes.size() << " EH Scopes:\n";
  for (unsigned i = 0, e = CurrentEHScopes.size(); i != e; ++i) {
    std::cerr << "  " << i << ". tree=" << (void*)CurrentEHScopes[i].TryExpr
              << " #blocks=" << CurrentEHScopes[i].Blocks.size()
              << " #fixups=" << CurrentEHScopes[i].BranchFixups.size() << "\n";
    for (unsigned f = 0, e = CurrentEHScopes[i].BranchFixups.size(); f != e;++f)
      std::cerr << "    Fixup #" << f << ": isEH="
                << CurrentEHScopes[i].BranchFixups[f].isExceptionEdge
                << "  br = " << *CurrentEHScopes[i].BranchFixups[f].SrcBranch;
  }
}
#endif

/// StripLLVMTranslationFn - Recursive function called from walk_trees to
/// implement StripLLVMTranslation.
static tree StripLLVMTranslationFn(tree *nodep, int *walk_subtrees,
                                   void *data) {
  tree node = *nodep;
  if (TYPE_P(node)) {
    // Don't walk into types.
    *walk_subtrees = 0;
  } else if (TREE_CODE(node) == STATEMENT_LIST) {
    // Look for basic block labels, clearing them out.
    for (tree_stmt_iterator I = tsi_start(node); !tsi_end_p(I); tsi_next(&I))
      if (TREE_CODE(tsi_stmt(I)) == LABEL_EXPR)
        SET_DECL_LLVM(TREE_OPERAND(tsi_stmt(I), 0), 0);
  } else if (TREE_CODE(node) == BIND_EXPR) {
    // Reset the declarations for local vars.  
    tree Var = BIND_EXPR_VARS(node);
    for (; Var; Var = TREE_CHAIN(Var)) {
      if (TREE_CODE(Var) == VAR_DECL && !TREE_STATIC(Var))
        SET_DECL_LLVM(Var, 0);
    }
  }
  return NULL_TREE;
}


/// StripLLVMTranslation - Given a block of code, walk it stripping off LLVM
/// information from declarations.  This permits the code to be expanded into
/// multiple places in the code without (e.g.) emitting the same LABEL_DECL node
/// into multiple places.
static void StripLLVMTranslation(tree code) {
  // Strip off llvm code.
  walk_tree_without_duplicates(&code, StripLLVMTranslationFn, 0);
}


/// GatherTypeInfo - Walk through the expression gathering all the
/// typeinfos that are used.
void TreeToLLVM::GatherTypeInfo(tree exp,
                                std::vector<Value *> &TypeInfos) {
  if (TREE_CODE(exp) == CATCH_EXPR || TREE_CODE(exp) == EH_FILTER_EXPR) {
    tree Types = TREE_CODE(exp) == CATCH_EXPR ? CATCH_TYPES(exp)
                                              : EH_FILTER_TYPES(exp);

    if (!Types) {
      // Catch all.
      TypeInfos.push_back(Constant::getNullValue(Type::Int32Ty));
    } else if (TREE_CODE(Types) != TREE_LIST) {
      // Construct typeinfo object.  Each call will produce a new expression
      // even if duplicate.
      tree TypeInfoNopExpr = (*lang_eh_runtime_type)(Types);
      // Produce value.  Duplicate typeinfo get folded here.
      Value *TypeInfo = Emit(TypeInfoNopExpr, 0);
      // Capture typeinfo.
      TypeInfos.push_back(TypeInfo);
    } else {
      for (; Types; Types = TREE_CHAIN (Types)) {
        // Construct typeinfo object.  Each call will produce a new expression
        // even if duplicate.
        tree TypeInfoNopExpr = (*lang_eh_runtime_type)(TREE_VALUE(Types));
        // Produce value.  Duplicate typeinfo get folded here.
        Value *TypeInfo = Emit(TypeInfoNopExpr, 0);
        // Capture typeinfo.
        TypeInfos.push_back(TypeInfo);
      }
    }
  } else {
    assert(TREE_CODE(exp) == STATEMENT_LIST && "Need an exp with typeinfo");
    // Each statement in the statement list will be a catch.
    for (tree_stmt_iterator I = tsi_start(exp); !tsi_end_p(I); tsi_next(&I))
      GatherTypeInfo(tsi_stmt(I), TypeInfos);
  }
}


/// AddLandingPad - Insert code to fetch and save the exception and exception
/// selector.
void TreeToLLVM::AddLandingPad() {
  tree TryCatch = 0;
  for (std::vector<EHScope>::reverse_iterator I = CurrentEHScopes.rbegin(),
                                              E = CurrentEHScopes.rend();
       I != E; ++I) {
    if (TREE_CODE(I->TryExpr) == TRY_CATCH_EXPR) {
      TryCatch = I->TryExpr;
      break;
    }
  }
  
  if (!TryCatch) return;
  
  // Gather the typeinfo.
  std::vector<Value *> TypeInfos;
  tree Catches = TREE_OPERAND(TryCatch, 1);
  GatherTypeInfo(Catches, TypeInfos);
  
  CreateExceptionValues();
  
  // Choose type of landing pad type.
  Function *F = FuncEHSelector;
  
  if (TREE_CODE(Catches) == STATEMENT_LIST &&
      !tsi_end_p(tsi_start(Catches)) &&
      TREE_CODE(tsi_stmt(tsi_start(Catches))) == EH_FILTER_EXPR) {
    F = FuncEHFilter;
  }
  
  // Fetch and store the exception.
  Value *Ex = new CallInst(FuncEHException, "eh_ptr", CurBB);
  new StoreInst(Ex, ExceptionValue, CurBB);
        
  // Fetch and store exception handler.
  std::vector<Value*> Args;
  Args.push_back(new LoadInst(ExceptionValue, "eh_ptr", CurBB));
  Args.push_back(CastToType(Instruction::BitCast, FuncCPPPersonality,
                            PointerType::get(Type::Int8Ty)));
  for (unsigned i = 0, N = TypeInfos.size(); i < N; ++i)
    Args.push_back(TypeInfos[i]);
  Value *Select = new CallInst(F, &Args[0], Args.size(), "eh_select", CurBB);
  new StoreInst(Select, ExceptionSelectorValue, CurBB);
}


/// CreateExceptionValues - Create values used internally by exception handling.
///
void TreeToLLVM::CreateExceptionValues() {
  // Check to see if the exception values have been constructed.
  if (ExceptionValue) return;
  
  ExceptionValue = CreateTemporary(PointerType::get(Type::Int8Ty));
  ExceptionValue->setName("eh_exception");
  
  ExceptionSelectorValue = CreateTemporary(Type::Int32Ty);
  ExceptionSelectorValue->setName("eh_selector");
  
  FuncEHException = Intrinsic::getDeclaration(TheModule,
                                              Intrinsic::eh_exception);
  FuncEHSelector  = Intrinsic::getDeclaration(TheModule,
                                              Intrinsic::eh_selector);
  FuncEHFilter    = Intrinsic::getDeclaration(TheModule,
                                              Intrinsic::eh_filter);
  FuncEHGetTypeID = Intrinsic::getDeclaration(TheModule,
                                              Intrinsic::eh_typeid_for);
                                                      
  FuncCPPPersonality = cast<Function>(
    TheModule->getOrInsertFunction("__gxx_personality_v0",
                                   Type::getPrimitiveType(Type::VoidTyID),
                                   NULL));
  FuncCPPPersonality->setLinkage(Function::ExternalLinkage);
  FuncCPPPersonality->setCallingConv(CallingConv::C);

  FuncUnwindResume = cast<Function>(
    TheModule->getOrInsertFunction("_Unwind_Resume",
                                   Type::getPrimitiveType(Type::VoidTyID),
                                   PointerType::get(Type::Int8Ty),
                                   NULL));
  FuncUnwindResume->setLinkage(Function::ExternalLinkage);
  FuncUnwindResume->setCallingConv(CallingConv::C);

}


/// EmitTRY_EXPR - Handle TRY_FINALLY_EXPR and TRY_CATCH_EXPR.
Value *TreeToLLVM::EmitTRY_EXPR(tree exp) {
  // The C++ front-end produces a lot of TRY_FINALLY_EXPR nodes that have empty
  // try blocks.  When these are seen, just emit the finally block directly for
  // a small compile time speedup.
  if (TREE_CODE(TREE_OPERAND(exp, 0)) == STATEMENT_LIST &&
      tsi_end_p(tsi_start(TREE_OPERAND(exp, 0)))) {
    if (TREE_CODE(exp) == TRY_CATCH_EXPR)
      return 0;   // TRY_CATCH_EXPR with empty try block: nothing thrown.

    // TRY_FINALLY_EXPR - Just run the finally block.
    assert(TREE_CODE(exp) == TRY_FINALLY_EXPR);
    Emit(TREE_OPERAND(exp, 1), 0);
    return 0;
  }  

  // Remember that we are in this scope.
  CurrentEHScopes.push_back(exp);
  
  Emit(TREE_OPERAND(exp, 0), 0);
  
  assert(CurrentEHScopes.back().TryExpr == exp && "Scope imbalance!");

  // Emit a new block for the fall-through of the finally block.
  BasicBlock *FinallyBlock = new BasicBlock("finally");
  EmitBlock(FinallyBlock);

  // Get the basic blocks in the current scope.
  std::vector<BasicBlock*> BlocksInScope;
  std::swap(CurrentEHScopes.back().Blocks, BlocksInScope);
  
  // Get the fixups in the current scope.
  std::vector<BranchFixup> BranchFixups;
  std::swap(CurrentEHScopes.back().BranchFixups, BranchFixups);
  
  // Remove the current scope.  The state of the function is no longer in this
  // scope.
  CurrentEHScopes.pop_back();

  // The finally fall-through block actually goes into the parent EH scope.  We
  // must emit things in this order so that EmitBlock can choose to nuke the
  // previous block, and correctly nuke it from the nested scope.
  if (!CurrentEHScopes.empty()) {
    CurrentEHScopes.back().Blocks.push_back(FinallyBlock);
    BlockEHScope[FinallyBlock] = CurrentEHScopes.size()-1;
  } else {
    BlockEHScope.erase(FinallyBlock);
  }
  
  // The finally block is not in the inner scope, it's actually in the outer
  // one.
  assert(BlocksInScope.back() == FinallyBlock);
  BlocksInScope.pop_back();
  
  // Give the FinallyBlock back a temporary terminator instruction.
  new UnreachableInst(FinallyBlock);

  // If the try block falls through (i.e. it doesn't end with a return), make
  // sure to add a fixup on that edge if needed.  If it falls through, EmitBlock
  // would add a branch from the fall-through source to FinallyBlock, otherwise
  // there will be no uses of it.
  if (!FinallyBlock->use_empty()) {
    BranchInst *BI = cast<BranchInst>(FinallyBlock->use_back());
    assert(FinallyBlock->hasOneUse() && BI->isUnconditional() &&
           "Unexpected behavior for EmitBlock");
    // Add an extra branch fixup for the try fall-through.
    BranchFixups.push_back(BranchFixup(BI, false));
  }
  
  // Loop over all of the fixups.  If the fixup destination was in the current
  // scope, then there is nothing to do and the fixup is done.  Remove these.
  for (unsigned i = 0, e = BranchFixups.size(); i != e; ++i) {
    BasicBlock *DestBlock = BranchFixups[i].SrcBranch->getSuccessor(0);
    std::map<BasicBlock*, unsigned>::iterator I = BlockEHScope.find(DestBlock);
    if (I != BlockEHScope.end() && I->second == CurrentEHScopes.size()) {
      BranchFixups[i] = BranchFixups.back();
      BranchFixups.pop_back();
      --i; --e;
      continue;
    }
    
    // If this is a TRY_CATCH expression and the fixup isn't for an exception
    // edge, punt the fixup up to the parent scope.
    if (TREE_CODE(exp) == TRY_CATCH_EXPR && !BranchFixups[i].isExceptionEdge) {
      // Add the fixup to the parent cleanup scope if there is one.
      if (!CurrentEHScopes.empty())
        AddBranchFixup(BranchFixups[i].SrcBranch, false);
      // Remove the fixup from this scope.
      BranchFixups[i] = BranchFixups.back();
      BranchFixups.pop_back();
      --i; --e;
      continue;
    }
  }
  
  // Otherwise, the branch is to some block outside of the scope, which requires
  // us to emit a copy of the finally code into the codepath.
  while (!BranchFixups.empty()) {
    BranchInst *FixupBr = BranchFixups.back().SrcBranch;
    bool FixupIsExceptionEdge = BranchFixups.back().isExceptionEdge;
    BranchFixups.pop_back();
    
    // Okay, the destination is in a parent to this scope, which means that the
    // branch fixup corresponds to an exit from this region.  Expand the cleanup
    // code then patch it into the code sequence.
    tree CleanupCode = TREE_OPERAND(exp, 1);
    
    // Add a basic block to emit the code into.
    BasicBlock *CleanupBB = new BasicBlock("cleanup");
    EmitBlock(CleanupBB);
    
    // Provide exit point for cleanup code.
    FinallyStack.push_back(FinallyBlock);
    
    // Emit the code.
    Emit(CleanupCode, 0);

    // Clear exit point for cleanup code.
    FinallyStack.pop_back();

    // Because we can emit the same cleanup in more than one context, we must
    // strip off LLVM information from the decls in the code.  Otherwise, the
    // we will try to insert the same label into multiple places in the code.
    StripLLVMTranslation(CleanupCode);
     
    
    // Catches will supply own terminator.
    if (!CurBB->getTerminator()) {
      // Emit a branch to the new target.
      BranchInst *BI = new BranchInst(FixupBr->getSuccessor(0), CurBB);
    
      // The old branch now goes to the cleanup block.
      FixupBr->setSuccessor(0, CleanupBB);
    
      // Fixup this new branch now.
      FixupBr = BI;
      
      // Add the fixup to the next cleanup scope if there is one.
      if (!CurrentEHScopes.empty())
        AddBranchFixup(FixupBr, FixupIsExceptionEdge);
    } else {
      // The old branch now goes to the cleanup block.
      FixupBr->setSuccessor(0, CleanupBB);
    }
  }

  // Move the finally block to the end of the function so we can continue
  // emitting code into it.
  Fn->getBasicBlockList().splice(Fn->end(), Fn->getBasicBlockList(),
                                 FinallyBlock);
  CurBB = FinallyBlock;
  
  // Now that all of the cleanup blocks have been expanded, remove the temporary
  // terminator we put on the FinallyBlock.
  assert(isa<UnreachableInst>(FinallyBlock->getTerminator()));
  FinallyBlock->getInstList().pop_back();
  
  // Finally, remove the blocks in the scope from the BlockEHScope map.
  for (unsigned i = 0, e = BlocksInScope.size(); i != e; ++i) {
    bool Erased = BlockEHScope.erase(BlocksInScope[i]);
    assert(Erased && "Block wasn't in map!");
  }
  return 0;
}


/// EmitCATCH_EXPR - Handle CATCH_EXPR.
///
Value *TreeToLLVM::EmitCATCH_EXPR(tree exp) {
#ifndef ITANIUM_STYLE_EXCEPTIONS
  return 0;
#endif

  // Make sure we have all the exception values in place.
  CreateExceptionValues();

  // Break out parts of catch.
  tree Types = CATCH_TYPES(exp);
  tree Body = CATCH_BODY(exp);
  
  // Destinations of the catch entry conditions.
  BasicBlock *ThenBlock = 0;
  BasicBlock *ElseBlock = 0;
  
  // FiXME - Determine last case so we don't need to emit test.
  if (!Types) {
    // Catch all - no testing required.
  } else if (TREE_CODE(Types) != TREE_LIST) {
    // Construct typeinfo object.  Each call will produce a new expression
    // even if duplicate.
    tree TypeInfoNopExpr = (*lang_eh_runtime_type)(Types);
    // Produce value.  Duplicate typeinfo get folded here.
    Value *TypeInfo = Emit(TypeInfoNopExpr, 0);

    // Call get eh type id.
    std::vector<Value*> Args;
    Args.push_back(TypeInfo);
    Value *TypeID = new CallInst(cast<Value>(FuncEHGetTypeID),
                                 &Args[0], Args.size(), "eh_typeid", CurBB);
    Value *Select = new LoadInst(ExceptionSelectorValue, "tmp", CurBB);

    // Compare with the exception selector.
    Value *Compare =
           new ICmpInst(ICmpInst::ICMP_EQ, Select, TypeID, "tmp", CurBB);
    ThenBlock = new BasicBlock("eh_then");
    ElseBlock = new BasicBlock("eh_else");
    
    // Branch on the compare.
    new BranchInst(ThenBlock, ElseBlock, Compare, CurBB);
  } else {
    ThenBlock = new BasicBlock("eh_then");
  
    for (; Types; Types = TREE_CHAIN (Types)) {
      if (ElseBlock) EmitBlock(ElseBlock);
    
      // Construct typeinfo object.  Each call will produce a new expression
      // even if duplicate.
      tree TypeInfoNopExpr = (*lang_eh_runtime_type)(TREE_VALUE(Types));
      // Produce value.  Duplicate typeinfo get folded here.
      Value *TypeInfo = Emit(TypeInfoNopExpr, 0);

      // Call get eh type id.
      std::vector<Value*> Args;
      Args.push_back(TypeInfo);
      Value *TypeID = new CallInst(cast<Value>(FuncEHGetTypeID),
                                 &Args[0], Args.size(), "eh_typeid", CurBB);
      Value *Select = new LoadInst(ExceptionSelectorValue, "tmp", CurBB);

      // Compare with the exception selector.
      Value *Compare =
             new ICmpInst(ICmpInst::ICMP_EQ, Select, TypeID, "tmp", CurBB);
      ElseBlock = new BasicBlock("eh_else");
      
      // Branch on the compare.
      new BranchInst(ThenBlock, ElseBlock, Compare, CurBB);
    }
  }
  
  // Start the then block.
  if (ThenBlock) EmitBlock(ThenBlock);
  
  // Emit the body.
  Emit(Body, 0);

  // Branch to the try exit.
  assert(!FinallyStack.empty() && "Need an exit point");
  if (!CurBB->getTerminator())
    new BranchInst(FinallyStack.back(), CurBB);

  // Start the else block.
  if (ElseBlock) EmitBlock(ElseBlock);
  
  return 0;
}

/// EmitEXC_PTR_EXPR - Handle EXC_PTR_EXPR.
///
Value *TreeToLLVM::EmitEXC_PTR_EXPR(tree exp) {
#ifndef ITANIUM_STYLE_EXCEPTIONS
  return 0;
#endif

  // Load exception address.
  CreateExceptionValues();
  return new LoadInst(ExceptionValue, "eh_value", CurBB);
}

/// EmitEH_FILTER_EXPR - Handle EH_FILTER_EXPR.
///
Value *TreeToLLVM::EmitEH_FILTER_EXPR(tree exp) {
#ifndef ITANIUM_STYLE_EXCEPTIONS
  return 0;
#endif

  CreateExceptionValues();

  // The result of a filter landing pad will be a negative index if there is
  // a match.
  Value *Select = new LoadInst(ExceptionSelectorValue, "tmp", CurBB);

  // Compare with the filter action value.
  Value *Zero = ConstantInt::get(Type::Int32Ty, 0);
  Value *Compare =
         new ICmpInst(ICmpInst::ICMP_SLT, Select, Zero, "tmp", CurBB);
  
  // Branch on the compare.
  BasicBlock *FilterBB = new BasicBlock("filter");
  BasicBlock *NoFilterBB = new BasicBlock("nofilter");
  new BranchInst(FilterBB, NoFilterBB, Compare, CurBB);

  EmitBlock(FilterBB);
  Emit(EH_FILTER_FAILURE(exp), 0);
  
  EmitBlock(NoFilterBB);
  
  return 0;
}

//===----------------------------------------------------------------------===//
//                           ... Expressions ...
//===----------------------------------------------------------------------===//

/// EmitINTEGER_CST_Aggregate - The C++ front-end abuses INTEGER_CST nodes to
/// represent empty classes.  For now we check that this is the case we handle,
/// then zero out DestLoc.
///
/// FIXME: When merged with mainline, remove this code.  The C++ front-end has
/// been fixed.
///
void TreeToLLVM::EmitINTEGER_CST_Aggregate(tree exp, Value *DestLoc) {
  tree type = TREE_TYPE(exp);
#ifndef NDEBUG
  assert(TREE_CODE(type) == RECORD_TYPE && "Not an empty class!");
  for (tree F = TYPE_FIELDS(type); F; F = TREE_CHAIN(F))
    assert(TREE_CODE(F) != FIELD_DECL && "Not an empty struct/class!");
#endif
  EmitAggregateZero(DestLoc, type);
}

/// EmitLoadOfLValue - When an l-value expression is used in a context that
/// requires an r-value, this method emits the lvalue computation, then loads
/// the result.
Value *TreeToLLVM::EmitLoadOfLValue(tree exp, Value *DestLoc) {
  // If this is an SSA value, don't emit a load, just use the result.
  if (isGCC_SSA_Temporary(exp)) {
    assert(DECL_LLVM_SET_P(exp) && "Definition not found before use!");
    return DECL_LLVM(exp);
  } else if (TREE_CODE(exp) == VAR_DECL && DECL_REGISTER(exp) &&
             TREE_STATIC(exp)) {
    // If this is a register variable, EmitLV can't handle it (there is no
    // l-value of a register variable).  Emit an inline asm node that copies the
    // value out of the specified register.
    return EmitReadOfRegisterVariable(exp, DestLoc);
  }
  
  LValue LV = EmitLV(exp);
  bool isVolatile = TREE_THIS_VOLATILE(exp);
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  
  if (!LV.isBitfield()) {
  
    if (!DestLoc) {
      // Scalar value: emit a load.
      Value *Ptr = CastToType(Instruction::BitCast, LV.Ptr, 
                              PointerType::get(Ty));
      return new LoadInst(Ptr, "tmp", isVolatile, CurBB);
    } else {
      EmitAggregateCopy(DestLoc, LV.Ptr, TREE_TYPE(exp), false, isVolatile);
      return 0;
    }
  } else {
    // This is a bitfield reference.
    Value *Val = new LoadInst(LV.Ptr, "tmp", isVolatile, CurBB);
    unsigned ValSizeInBits = Val->getType()->getPrimitiveSizeInBits();
      
    assert(Val->getType()->isInteger() && "Invalid bitfield lvalue!");
    assert(ValSizeInBits >= LV.BitSize && "Bad bitfield lvalue!");
    assert(ValSizeInBits >= LV.BitSize+LV.BitStart && "Bad bitfield lvalue!");

    // Mask the bits out by shifting left first, then shifting right.  The
    // LLVM optimizer will turn this into an AND if this is an unsigned
    // expression.
    
    // If this target has bitfields laid out in big-endian order, invert the bit
    // in the word if needed.
    if (BITS_BIG_ENDIAN)
      LV.BitStart = ValSizeInBits-LV.BitStart-LV.BitSize;
    
    if (LV.BitStart+LV.BitSize != ValSizeInBits) {
      Value *ShAmt = ConstantInt::get(Val->getType(),
                                       ValSizeInBits-(LV.BitStart+LV.BitSize));
      Val = BinaryOperator::createShl(Val, ShAmt, "tmp", CurBB);
    }
    
    // Shift right required?
    if (ValSizeInBits-LV.BitSize) {
      Value *ShAmt = ConstantInt::get(Val->getType(), ValSizeInBits-LV.BitSize);
      Val = BinaryOperator::create(TYPE_UNSIGNED(TREE_TYPE(exp)) ? 
        Instruction::LShr : Instruction::AShr, Val, ShAmt, "tmp", CurBB);
    }

    if (TYPE_UNSIGNED(TREE_TYPE(exp)))
      return CastToUIntType(Val, Ty);
    else
      return CastToSIntType(Val, Ty);
  }
}

Value *TreeToLLVM::EmitADDR_EXPR(tree exp) {
  LValue LV = EmitLV(TREE_OPERAND(exp, 0));
  assert((!LV.isBitfield() || LV.BitStart == 0) &&
         "It is illegal to take the address of a bitfield");
  // Perform a cast here if necessary.  For example, GCC sometimes forms an
  // ADDR_EXPR where the operand is an array, and the ADDR_EXPR type is a
  // pointer to the first element.
  return CastToType(Instruction::BitCast, LV.Ptr, ConvertType(TREE_TYPE(exp))); 
}

Value *TreeToLLVM::EmitOBJ_TYPE_REF(tree exp) {
  return CastToType(Instruction::BitCast, Emit(OBJ_TYPE_REF_EXPR(exp), 0), 
                    ConvertType(TREE_TYPE(exp))); 
}

Value *TreeToLLVM::EmitCALL_EXPR(tree exp, Value *DestLoc) {
  // Check for a built-in function call.  If we can lower it directly, do so
  // now.
  tree fndecl = get_callee_fndecl(exp);
  if (fndecl && DECL_BUILT_IN(fndecl) &&
      DECL_BUILT_IN_CLASS(fndecl) != BUILT_IN_FRONTEND) {
    Value *Res = 0;
    if (EmitBuiltinCall(exp, fndecl, DestLoc, Res))
      return Res;
  }

  Value *Callee = Emit(TREE_OPERAND(exp, 0), 0);

  if (TREE_OPERAND(exp, 2)) {
    // This is a direct call to a function using a static chain.  We need to
    // change the function type to one with an extra parameter for the chain.
    assert(fndecl && "Indirect static chain call!");
    tree function_type = TYPE_MAIN_VARIANT(TREE_TYPE(fndecl));
    tree static_chain = TREE_OPERAND(exp, 2);

    unsigned CallingConv;
    const Type *Ty = TheTypeConverter->ConvertFunctionType(function_type,
                                                           static_chain,
                                                           CallingConv);
    Callee = CastToType(Instruction::BitCast, Callee, PointerType::get(Ty));
  }

  //EmitCall(exp, DestLoc);
  Value *Result = EmitCallOf(Callee, exp, DestLoc);

  // If the function has the volatile bit set, then it is a "noreturn" function.
  // Output an unreachable instruction right after the function to prevent LLVM
  // from thinking that control flow will fall into the subsequent block.
  //
  if (fndecl && TREE_THIS_VOLATILE(fndecl)) {
    new UnreachableInst(CurBB);
    EmitBlock(new BasicBlock(""));
  }
  return Result;
}

namespace {
  /// FunctionCallArgumentConversion - This helper class is driven by the ABI
  /// definition for this target to figure out how to pass arguments into the
  /// stack/regs for a function call.
  struct FunctionCallArgumentConversion : public DefaultABIClient {
    tree CallExpression;
    SmallVector<Value*, 16> &CallOperands;
    CallingConv::ID &CallingConvention;
    bool isStructRet;
    BasicBlock *CurBB;
    Value *DestLoc;
    std::vector<Value*> LocStack;

    FunctionCallArgumentConversion(tree exp, SmallVector<Value*, 16> &ops,
                                   CallingConv::ID &cc,
                                   BasicBlock *bb, Value *destloc)
      : CallExpression(exp), CallOperands(ops), CallingConvention(cc),
        CurBB(bb), DestLoc(destloc) {
      CallingConvention = CallingConv::C;
      isStructRet = false;
#ifdef TARGET_ADJUST_LLVM_CC
      tree ftype;
      if (tree fdecl = get_callee_fndecl(exp)) {
        ftype = TREE_TYPE(fdecl);
      } else {
        ftype = TREE_TYPE(TREE_OPERAND(exp,0));

        // If it's call to pointer, we look for the function type.
        if (TREE_CODE(ftype) == POINTER_TYPE)
          ftype = TREE_TYPE(ftype);
      }
      
      TARGET_ADJUST_LLVM_CC(CallingConvention, ftype);
#endif
    }
    
    void setLocation(Value *Loc) {
      LocStack.push_back(Loc);
    }
    void clear() {
      assert(LocStack.size() == 1 && "Imbalance!");
      LocStack.clear();
    }
    
    /// HandleScalarResult - This callback is invoked if the function returns a
    /// simple scalar result value.
    void HandleScalarResult(const Type *RetTy) {
      // There is nothing to do here if we return a scalar or void.
      assert(DestLoc == 0 &&
             "Call returns a scalar but caller expects aggregate!");
    }
    
    /// HandleAggregateResultAsScalar - This callback is invoked if the function
    /// returns an aggregate value by bit converting it to the specified scalar
    /// type and returning that.
    void HandleAggregateResultAsScalar(const Type *ScalarTy) {
      // There is nothing to do here.
    }
    
    /// HandleAggregateShadowArgument - This callback is invoked if the function
    /// returns an aggregate value by using a "shadow" first parameter.  If
    /// RetPtr is set to true, the pointer argument itself is returned from the
    /// function.
    void HandleAggregateShadowArgument(const PointerType *PtrArgTy,
                                       bool RetPtr) {
      // Make sure this call is marked as 'struct return'.
      isStructRet = true;
      
      // If the front-end has already made the argument explicit, don't do it
      // again.
      if (CALL_EXPR_HAS_RETURN_SLOT_ADDR(CallExpression))
        return;
      
      // Otherwise, we need to pass a buffer to return into.  If the caller uses
      // the result, DestLoc will be set.  If it ignores it, it could be unset,
      // in which case we need to create a dummy buffer.
      if (DestLoc == 0)
        DestLoc = TheTreeToLLVM->CreateTemporary(PtrArgTy->getElementType());
      else
        assert(PtrArgTy == DestLoc->getType());
      CallOperands.push_back(DestLoc);
    }    
    
    void HandleScalarArgument(const llvm::Type *LLVMTy, tree type) {
      assert(!LocStack.empty());
      Value *Loc = LocStack.back();
      if (cast<PointerType>(Loc->getType())->getElementType() != LLVMTy)
        // This always deals with pointer types so BitCast is appropriate
        Loc = CastInst::create(Instruction::BitCast, Loc, 
                               PointerType::get(LLVMTy), "tmp", CurBB);
      
      Value *V = new LoadInst(Loc, "tmp", CurBB);
      CallOperands.push_back(V);
    }
    
    void EnterField(unsigned FieldNo, const llvm::Type *StructTy) {
      Constant *Zero = Constant::getNullValue(Type::Int32Ty);
      Constant *FIdx = ConstantInt::get(Type::Int32Ty, FieldNo);
      Value *Loc = LocStack.back();
      if (cast<PointerType>(Loc->getType())->getElementType() != StructTy)
        // This always deals with pointer types so BitCast is appropriate
        Loc = CastInst::create(Instruction::BitCast, Loc, 
                               PointerType::get(StructTy), "tmp", CurBB);
      
      Loc = new GetElementPtrInst(Loc, Zero, FIdx, "tmp", CurBB);
      LocStack.push_back(Loc);    
    }
    void ExitField() {
      LocStack.pop_back();
    }
  };
}


/// EmitCallOf - Emit a call to the specified callee with the operands specified
/// in the CALL_EXP 'exp'.  If the result of the call is a scalar, return the
/// result, otherwise store it in DestLoc.
Value *TreeToLLVM::EmitCallOf(Value *Callee, tree exp, Value *DestLoc) {
  // Determine if we need to generate an invoke instruction (instead of a simple
  // call) and if so, what the exception destination will be.
  BasicBlock *UnwindBlock = 0;
  
  // Do not turn intrinsic calls or no-throw calls into invokes.
  if ((!isa<Function>(Callee) || !cast<Function>(Callee)->getIntrinsicID()) &&
      // Turn calls that throw that are inside of a cleanup scope into invokes.
      !CurrentEHScopes.empty() && tree_could_throw_p(exp)) {
    if (UnwindBB == 0)
      UnwindBB = new BasicBlock("Unwind");
    UnwindBlock = UnwindBB;
  }
  
  SmallVector<Value*, 16> CallOperands;
  CallingConv::ID CallingConvention;
  FunctionCallArgumentConversion Client(exp, CallOperands, CallingConvention,
                                        CurBB, DestLoc);
  TheLLVMABI<FunctionCallArgumentConversion> ABIConverter(Client);

  // Handle the result, including struct returns.
  ABIConverter.HandleReturnType(TREE_TYPE(exp));

  // Pass the static chain, if any, as the first parameter.
  if (TREE_OPERAND(exp, 2))
    CallOperands.push_back(Emit(TREE_OPERAND(exp, 2), 0));

  // Loop over the arguments, expanding them and adding them to the op list.
  const PointerType *PFTy = cast<PointerType>(Callee->getType());
  const FunctionType *FTy = cast<FunctionType>(PFTy->getElementType());
  for (tree arg = TREE_OPERAND(exp, 1); arg; arg = TREE_CHAIN(arg)) {
    const Type *ActualArgTy = ConvertType(TREE_TYPE(TREE_VALUE(arg)));
    const Type *ArgTy = ActualArgTy;
    if (CallOperands.size() < FTy->getNumParams())
      ArgTy = FTy->getParamType(CallOperands.size());
    
    // If we are implicitly passing the address of this argument instead of 
    // passing it by value, handle this first.
    if (isPassedByInvisibleReference(TREE_TYPE(TREE_VALUE(arg)))) {
      // Get the address of the parameter passed in.
      LValue ArgVal = EmitLV(TREE_VALUE(arg));
      assert(!ArgVal.isBitfield() && "Bitfields shouldn't be invisible refs!");
      Value *Ptr = ArgVal.Ptr;
      
      if (CallOperands.size() >= FTy->getNumParams())
        ArgTy = PointerType::get(ArgTy);
      CallOperands.push_back(CastToType(Instruction::BitCast, Ptr, ArgTy));
    } else if (ActualArgTy->isFirstClassType()) {
      Value *V = Emit(TREE_VALUE(arg), 0);
      bool isSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_VALUE(arg)));
      CallOperands.push_back(CastToAnyType(V, isSigned, ArgTy, false));
    } else {
      // If this is an aggregate value passed by-value, use the current ABI to
      // determine how the parameters are passed.
      LValue LV = EmitLV(TREE_VALUE(arg));
      assert(!LV.isBitfield() && "Bitfields are first-class types!");
      Client.setLocation(LV.Ptr);
      ABIConverter.HandleArgument(TREE_TYPE(TREE_VALUE(arg)));
    }
  }
  
  // Compile stuff like:
  //   %tmp = call float (...)* bitcast (float ()* @foo to float (...)*)( )
  // to:
  //   %tmp = call float @foo( )
  // This commonly occurs due to C "implicit ..." semantics.
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(Callee)) {
    if (CallOperands.empty() && CE->getOpcode() == Instruction::BitCast) {
      Constant *RealCallee = CE->getOperand(0);
      assert(isa<PointerType>(RealCallee->getType()) &&
             "Bitcast to ptr not from ptr?");
      const PointerType *RealPT = cast<PointerType>(RealCallee->getType());
      if (const FunctionType *RealFT =
          dyn_cast<FunctionType>(RealPT->getElementType())) {
        const PointerType *ActualPT = cast<PointerType>(Callee->getType());
        const FunctionType *ActualFT =
          cast<FunctionType>(ActualPT->getElementType());
        if (RealFT->getReturnType() == ActualFT->getReturnType() &&
            ActualFT->getNumParams() == 0)
          Callee = RealCallee;
      }
    }
  }
  
  Value *Call;
  if (!UnwindBlock) {
    Call = new CallInst(Callee, &CallOperands[0], CallOperands.size(),
                        "", CurBB);
    cast<CallInst>(Call)->setCallingConv(CallingConvention);
  } else {
    BasicBlock *NextBlock = new BasicBlock("invcont");
    Call = new InvokeInst(Callee, NextBlock, UnwindBlock,
                          &CallOperands[0], CallOperands.size(), "", CurBB);
    cast<InvokeInst>(Call)->setCallingConv(CallingConvention);

    // Lazily create an unwind block for this scope, which we can emit a fixup
    // branch in.
    if (CurrentEHScopes.back().UnwindBlock == 0) {
      EmitBlock(CurrentEHScopes.back().UnwindBlock = new BasicBlock("unwind"));
      
#ifdef ITANIUM_STYLE_EXCEPTIONS
      // Add landing pad entry code.
      AddLandingPad();
#endif
      // This branch to the unwind edge should have exception cleanups
      // inserted onto it.
      EmitBranchInternal(UnwindBlock, true);
    }

    cast<InvokeInst>(Call)->setUnwindDest(CurrentEHScopes.back().UnwindBlock);
    
    EmitBlock(NextBlock);
  }
  
  if (Call->getType() == Type::VoidTy)
    return 0;
  
  Call->setName("tmp");
    
  // If the caller expects an aggregate, we have a situation where the ABI for
  // the current target specifies that the aggregate be returned in scalar
  // registers even though it is an aggregate.  We must bitconvert the scalar
  // to the destination aggregate type.  We do this by casting the DestLoc
  // pointer and storing into it.
  if (!DestLoc)
    return Call;   // Normal scalar return.

  DestLoc = BitCastToType(DestLoc, PointerType::get(Call->getType()));
  new StoreInst(Call, DestLoc, CurBB);
  return 0;
}

/// HandleMultiplyDefinedGCCTemp - GCC temporaries are *mostly* single
/// definition, and always have all uses dominated by the definition.  In cases
/// where the temporary has multiple uses, we will first see the initial 
/// definition, some uses of that definition, then subsequently see another
/// definition with uses of this second definition.
///
/// Because LLVM temporaries *must* be single definition, when we see the second
/// definition, we actually change the temporary to mark it as not being a GCC
/// temporary anymore.  We then create an alloca for it, initialize it with the
/// first value seen, then treat it as a normal variable definition.
///
void TreeToLLVM::HandleMultiplyDefinedGCCTemp(tree Var) {
  Value *FirstVal = DECL_LLVM(Var);

  // Create a new temporary and set the VAR_DECL to use it as the llvm location.
  Value *NewTmp = CreateTemporary(FirstVal->getType());
  SET_DECL_LLVM(Var, NewTmp);

  // Store the already existing initial value into the alloca.  If the value
  // being stored is an instruction, emit the store right after the instruction,
  // otherwise, emit it into the entry block.
  StoreInst *SI = new StoreInst(FirstVal, NewTmp);
  
  BasicBlock::iterator InsertPt;
  if (Instruction *I = dyn_cast<Instruction>(FirstVal)) {
    InsertPt = I;                      // Insert after the init instruction.
    // If the instruction is an alloca in the entry block, the insert point
    // will be before the alloca.  Advance to the AllocaInsertionPoint if we are
    // before it.
    if (I->getParent() == &Fn->front()) {
      for (BasicBlock::iterator CI = InsertPt, E = Fn->begin()->end();
           CI != E; ++CI) {
        if (&*CI == AllocaInsertionPoint) {
          InsertPt = AllocaInsertionPoint;
          break;
        }
      }
    }
  } else {
    InsertPt = AllocaInsertionPoint;   // Insert after the allocas.
  }
  BasicBlock *BB = InsertPt->getParent();
  BB->getInstList().insert(++InsertPt, SI);
  
  // Finally, This is no longer a GCC temporary.
  DECL_GIMPLE_FORMAL_TEMP_P(Var) = 0;
}

/// EmitMODIFY_EXPR - Note that MODIFY_EXPRs are rvalues only!
///
Value *TreeToLLVM::EmitMODIFY_EXPR(tree exp, Value *DestLoc) {
  // If this is the definition of an SSA variable, set its DECL_LLVM to the
  // RHS.
  bool Op0Signed = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 0)));
  bool Op1Signed = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 1)));
  if (isGCC_SSA_Temporary(TREE_OPERAND(exp, 0))) {
    // If DECL_LLVM is already set, this is a multiply defined GCC temporary.
    if (DECL_LLVM_SET_P(TREE_OPERAND(exp, 0))) {
      HandleMultiplyDefinedGCCTemp(TREE_OPERAND(exp, 0));
      return EmitMODIFY_EXPR(exp, DestLoc);
    }
    
    Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);
    RHS = CastToAnyType(RHS, Op1Signed, 
                        ConvertType(TREE_TYPE(TREE_OPERAND(exp, 0))), 
                        Op0Signed);
    SET_DECL_LLVM(TREE_OPERAND(exp, 0), RHS);
    return RHS;
  } else if (TREE_CODE(TREE_OPERAND(exp, 0)) == VAR_DECL &&
             DECL_REGISTER(TREE_OPERAND(exp, 0)) &&
             TREE_STATIC(TREE_OPERAND(exp, 0))) {
    // If this is a store to a register variable, EmitLV can't handle the dest
    // (there is no l-value of a register variable).  Emit an inline asm node
    // that copies the value into the specified register.
    Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);
    RHS = CastToAnyType(RHS, Op1Signed,
                        ConvertType(TREE_TYPE(TREE_OPERAND(exp, 0))), 
                        Op0Signed);
    EmitModifyOfRegisterVariable(TREE_OPERAND(exp, 0), RHS);
    return RHS;
  }
  
  LValue LV = EmitLV(TREE_OPERAND(exp, 0));
  bool isVolatile = TREE_THIS_VOLATILE(TREE_OPERAND(exp, 0));

  if (!LV.isBitfield()) {
    const Type *ValTy = ConvertType(TREE_TYPE(TREE_OPERAND(exp, 1)));
    if (ValTy->isFirstClassType()) {
      // Non-bitfield, scalar value.  Just emit a store.
      Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);
      // Convert RHS to the right type if we can, otherwise convert the pointer.
      const PointerType *PT = cast<PointerType>(LV.Ptr->getType());
      if (PT->getElementType()->canLosslesslyBitCastTo(RHS->getType()))
        RHS = CastToAnyType(RHS, Op1Signed, PT->getElementType(), Op0Signed);
      else
        LV.Ptr = BitCastToType(LV.Ptr, PointerType::get(RHS->getType()));
      new StoreInst(RHS, LV.Ptr, isVolatile, CurBB);
      return RHS;
    }

    // Non-bitfield aggregate value.
    Emit(TREE_OPERAND(exp, 1), LV.Ptr);
    if (DestLoc)
      EmitAggregateCopy(DestLoc, LV.Ptr, TREE_TYPE(exp), isVolatile, false);
    return 0;
  }

  // Last case, this is a store to a bitfield, so we have to emit a 
  // read/modify/write sequence.
  Value *OldVal = new LoadInst(LV.Ptr, "tmp", isVolatile, CurBB);
  
  // If the target is big-endian, invert the bit in the word.
  unsigned ValSizeInBits = TD.getTypeSize(OldVal->getType())*8;
  if (BITS_BIG_ENDIAN)
    LV.BitStart = ValSizeInBits-LV.BitStart-LV.BitSize;

  // If not storing into the zero'th bit, shift the Src value to the left.
  Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);
  Value *RetVal = RHS;
  RHS = CastToAnyType(RHS, Op1Signed, OldVal->getType(), Op0Signed);
  if (LV.BitStart)
    RHS = BinaryOperator::createShl(RHS, ConstantInt::get(RHS->getType(), 
                                    LV.BitStart), "tmp", CurBB);

  // Next, if this doesn't touch the top bit, mask out any bits that shouldn't
  // be set in the result.
  uint64_t MaskVal = ((1ULL << LV.BitSize)-1) << LV.BitStart;
  Constant *Mask = ConstantInt::get(Type::Int64Ty, MaskVal);
  Mask = ConstantExpr::getTruncOrBitCast(Mask, RHS->getType());
  if (LV.BitStart+LV.BitSize != ValSizeInBits)
    RHS = BinaryOperator::createAnd(RHS, Mask, "tmp", CurBB);
  
  // Next, mask out the bits this bit-field should include from the old value.
  Mask = ConstantExpr::getNot(Mask);
  OldVal = BinaryOperator::createAnd(OldVal, Mask, "tmp", CurBB);
  
  // Finally, merge the two together and store it.
  Value *Val = BinaryOperator::createOr(OldVal, RHS, "tmp", CurBB);
  new StoreInst(Val, LV.Ptr, isVolatile, CurBB);
  return RetVal;
}

Value *TreeToLLVM::EmitNOP_EXPR(tree exp, Value *DestLoc) {
  if (TREE_CODE(TREE_TYPE(exp)) == VOID_TYPE &&    // deleted statement.
      TREE_CODE(TREE_OPERAND(exp, 0)) == INTEGER_CST)
    return 0;
  tree Op = TREE_OPERAND(exp, 0);
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  bool OpIsSigned = !TYPE_UNSIGNED(TREE_TYPE(Op));
  bool ExpIsSigned = !TYPE_UNSIGNED(TREE_TYPE(exp));
  if (DestLoc == 0) {
    // Scalar to scalar copy.
    assert(!isAggregateTreeType(TREE_TYPE(Op))
	   && "Aggregate to scalar nop_expr!");
    Value *OpVal = Emit(Op, DestLoc);
    if (Ty == Type::VoidTy) return 0;
    return CastToAnyType(OpVal, OpIsSigned, Ty, ExpIsSigned);
  } else if (isAggregateTreeType(TREE_TYPE(Op))) {
    // Aggregate to aggregate copy.
    DestLoc = CastToType(Instruction::BitCast, DestLoc, PointerType::get(Ty));
    Value *OpVal = Emit(Op, DestLoc);
    assert(OpVal == 0 && "Shouldn't cast scalar to aggregate!");
    return 0;
  } 

  // Scalar to aggregate copy.
  Value *OpVal = Emit(Op, 0);
  DestLoc = CastToType(Instruction::BitCast, DestLoc, 
                       PointerType::get(OpVal->getType()));
  new StoreInst(OpVal, DestLoc, CurBB);
  return 0;
}

Value *TreeToLLVM::EmitCONVERT_EXPR(tree exp, Value *DestLoc) {
  assert(!DestLoc && "Cannot handle aggregate casts!");
  Value *Op = Emit(TREE_OPERAND(exp, 0), 0);
  bool OpIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 0)));
  bool ExpIsSigned = !TYPE_UNSIGNED(TREE_TYPE(exp));
  return CastToAnyType(Op, OpIsSigned, ConvertType(TREE_TYPE(exp)),ExpIsSigned);
}

Value *TreeToLLVM::EmitVIEW_CONVERT_EXPR(tree exp, Value *DestLoc) {
  tree Op = TREE_OPERAND(exp, 0);

  if (isAggregateTreeType(TREE_TYPE(Op))) {
    const Type *OpTy = ConvertType(TREE_TYPE(Op));
    Value *Target = DestLoc ?
      // This is an aggregate-to-agg VIEW_CONVERT_EXPR, just evaluate in place.
      CastToType(Instruction::BitCast, DestLoc, PointerType::get(OpTy)) :
      // This is an aggregate-to-scalar VIEW_CONVERT_EXPR, evaluate, then load.
      CreateTemporary(OpTy);

    switch (TREE_CODE(Op)) {
    default: {
      Value *OpVal = Emit(Op, Target);
      assert(OpVal == 0 && "Expected an aggregate operand!");
      break;
    }

    // Lvalues
    case VAR_DECL:
    case PARM_DECL:
    case RESULT_DECL:
    case INDIRECT_REF:
    case ARRAY_REF:
    case ARRAY_RANGE_REF:
    case COMPONENT_REF:
    case BIT_FIELD_REF:
    case STRING_CST:
    case REALPART_EXPR:
    case IMAGPART_EXPR:
      // Same as EmitLoadOfLValue but taking the size from TREE_TYPE(exp), since
      // the size of TREE_TYPE(Op) may not be available.
      LValue LV = EmitLV(Op);
      assert(!LV.isBitfield() && "Expected an aggregate operand!");
      bool isVolatile = TREE_THIS_VOLATILE(Op);

      EmitAggregateCopy(Target, LV.Ptr, TREE_TYPE(exp), false, isVolatile);
      break;
    }

    if (DestLoc)
      return 0;

    const Type *ExpTy = ConvertType(TREE_TYPE(exp));
    return new LoadInst(CastToType(Instruction::BitCast, Target,
                                   PointerType::get(ExpTy)), "tmp", CurBB);
  }
  
  if (DestLoc) {
    // The input is a scalar the output is an aggregate, just eval the input,
    // then store into DestLoc.
    Value *OpVal = Emit(Op, 0);
    assert(OpVal && "Expected a scalar result!");
    DestLoc = CastToType(Instruction::BitCast, DestLoc, 
                         PointerType::get(OpVal->getType()));
    new StoreInst(OpVal, DestLoc, CurBB);
    return 0;
  }

  // Otherwise, this is a scalar to scalar conversion.
  Value *OpVal = Emit(Op, 0);
  assert(OpVal && "Expected a scalar result!");
  const Type *DestTy = ConvertType(TREE_TYPE(exp));
  
  // If the source is a pointer, use ptrtoint to get it to something
  // bitcast'able.  This supports things like v_c_e(foo*, float).
  if (isa<PointerType>(OpVal->getType())) {
    if (isa<PointerType>(DestTy))   // ptr->ptr is a simple bitcast.
      return new BitCastInst(OpVal, DestTy, "tmp", CurBB);
    // Otherwise, ptrtoint to intptr_t first.
    OpVal = new PtrToIntInst(OpVal, TD.getIntPtrType(), "tmp", CurBB);
  }
  
  // If the destination type is a pointer, use inttoptr.
  if (isa<PointerType>(DestTy))
    return new IntToPtrInst(OpVal, DestTy, "tmp", CurBB);

  // Otherwise, use a bitcast.
  return new BitCastInst(OpVal, DestTy, "tmp", CurBB);
}

Value *TreeToLLVM::EmitNEGATE_EXPR(tree exp, Value *DestLoc) {
  if (!DestLoc)
    return BinaryOperator::createNeg(Emit(TREE_OPERAND(exp, 0), 0), 
                                     "tmp", CurBB);
  
  // Emit the operand to a temporary.
  const Type *ComplexTy=cast<PointerType>(DestLoc->getType())->getElementType();
  Value *Tmp = CreateTemporary(ComplexTy);
  Emit(TREE_OPERAND(exp, 0), Tmp);

  // Handle complex numbers: -(a+ib) = -a + i*-b
  Value *R, *I;
  EmitLoadFromComplex(R, I, Tmp, TREE_THIS_VOLATILE(TREE_OPERAND(exp, 0)));
  R = BinaryOperator::createNeg(R, "tmp", CurBB);
  I = BinaryOperator::createNeg(I, "tmp", CurBB);
  EmitStoreToComplex(DestLoc, R, I, false);
  return 0;
}

Value *TreeToLLVM::EmitCONJ_EXPR(tree exp, Value *DestLoc) {
  assert(DestLoc && "CONJ_EXPR only applies to complex numbers.");
  // Emit the operand to a temporary.
  const Type *ComplexTy=cast<PointerType>(DestLoc->getType())->getElementType();
  Value *Tmp = CreateTemporary(ComplexTy);
  Emit(TREE_OPERAND(exp, 0), Tmp);
  
  // Handle complex numbers: ~(a+ib) = a + i*-b
  Value *R, *I;
  EmitLoadFromComplex(R, I, Tmp, TREE_THIS_VOLATILE(TREE_OPERAND(exp, 0)));
  I = BinaryOperator::createNeg(I, "tmp", CurBB);
  EmitStoreToComplex(DestLoc, R, I, false);
  return 0;
}

Value *TreeToLLVM::EmitABS_EXPR(tree exp) {
  Value *Op = Emit(TREE_OPERAND(exp, 0), 0);
  if (!Op->getType()->isFloatingPoint()) {
    Instruction *OpN = BinaryOperator::createNeg(Op, Op->getName()+"neg",CurBB);
    ICmpInst::Predicate pred = TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp,0))) ?
      ICmpInst::ICMP_UGE : ICmpInst::ICMP_SGE;
    Value *Cmp = new ICmpInst(pred, Op, OpN->getOperand(0), "abscond", CurBB);
    return new SelectInst(Cmp, Op, OpN, "abs", CurBB);
  } else {
    // Turn FP abs into fabs/fabsf.
    return EmitBuiltinUnaryFPOp(Op, "fabsf", "fabs");
  }
}

Value *TreeToLLVM::EmitBIT_NOT_EXPR(tree exp) {
  Value *Op = Emit(TREE_OPERAND(exp, 0), 0);
  return BinaryOperator::createNot(Op, Op->getName()+"not", CurBB);
}

Value *TreeToLLVM::EmitTRUTH_NOT_EXPR(tree exp) {
  Value *V = Emit(TREE_OPERAND(exp, 0), 0);
  if (V->getType() != Type::Int1Ty) 
    V = new ICmpInst(ICmpInst::ICMP_NE, V, 
                     Constant::getNullValue(V->getType()), "toBool", CurBB);
  V = BinaryOperator::createNot(V, V->getName()+"not", CurBB);
  return CastToUIntType(V, ConvertType(TREE_TYPE(exp)));
}

/// EmitCompare - 'exp' is a comparison of two values.  Opc is the base LLVM
/// comparison to use.  isUnord is true if this is a floating point comparison
/// that should also be true if either operand is a NaN.  Note that Opc can be
/// set to zero for special cases.
Value *TreeToLLVM::EmitCompare(tree exp, unsigned UIOpc, unsigned SIOpc, 
                               unsigned FPPred) {
  // Get the type of the operands
  tree Op0Ty = TREE_TYPE(TREE_OPERAND(exp,0));

  // Deal with complex types
  if (TREE_CODE(Op0Ty) == COMPLEX_TYPE)
    return EmitComplexBinOp(exp, 0);  // Complex ==/!=

  // Get the compare operands, in the right type. Comparison of struct is not
  // allowed, so this is safe as we already handled complex (struct) type.
  Value *LHS = Emit(TREE_OPERAND(exp, 0), 0);
  Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);
  bool LHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 0)));
  bool RHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 1)));
  RHS = CastToAnyType(RHS, RHSIsSigned, LHS->getType(), LHSIsSigned);
  assert(LHS->getType() == RHS->getType() && "Binop type equality failure!");

  // Handle the integer/pointer cases
  if (!FLOAT_TYPE_P(Op0Ty)) {
    // Determine which predicate to use based on signedness
    ICmpInst::Predicate pred = 
      ICmpInst::Predicate(TYPE_UNSIGNED(Op0Ty) ? UIOpc : SIOpc);

    // Get the compare instructions
    Value *Result = new ICmpInst(pred, LHS, RHS, "tmp", CurBB);
    
    // The GCC type is probably an int, not a bool.
    return CastToUIntType(Result, ConvertType(TREE_TYPE(exp)));
  }

  // Handle floating point comparisons, if we get here.
  Value *Result =
    new FCmpInst(FCmpInst::Predicate(FPPred), LHS, RHS, "tmp", CurBB);
  
  // The GCC type is probably an int, not a bool.
  return CastToUIntType(Result, ConvertType(TREE_TYPE(exp)));
}

/// EmitBinOp - 'exp' is a binary operator.
///
Value *TreeToLLVM::EmitBinOp(tree exp, Value *DestLoc, unsigned Opc) {
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  if (isa<PointerType>(Ty))
    return EmitPtrBinOp(exp, Opc);   // Pointer arithmetic!
  if (isa<StructType>(Ty))
    return EmitComplexBinOp(exp, DestLoc);
  assert(Ty->isFirstClassType() && DestLoc == 0 &&
         "Bad binary operation!");
  
  Value *LHS = Emit(TREE_OPERAND(exp, 0), 0);
  Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);
  
  // GCC has no problem with things like "xor uint X, int 17", and X-Y, where
  // X and Y are pointer types, but the result is an integer.  As such, convert
  // everything to the result type.
  bool LHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 0)));
  bool RHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 1)));
  bool TyIsSigned  = !TYPE_UNSIGNED(TREE_TYPE(exp));
  LHS = CastToAnyType(LHS, LHSIsSigned, Ty, TyIsSigned);
  RHS = CastToAnyType(RHS, RHSIsSigned, Ty, TyIsSigned);

  return BinaryOperator::create((Instruction::BinaryOps)Opc, LHS, RHS,
                                "tmp", CurBB);
}

/// EmitPtrBinOp - Handle binary expressions involving pointers, e.g. "P+4".
///
Value *TreeToLLVM::EmitPtrBinOp(tree exp, unsigned Opc) {
  Value *LHS = Emit(TREE_OPERAND(exp, 0), 0);
  
  // If this is an expression like (P+4), try to turn this into
  // "getelementptr P, 1".
  if ((Opc == Instruction::Add || Opc == Instruction::Sub) &&
      TREE_CODE(TREE_OPERAND(exp, 1)) == INTEGER_CST) {
    int64_t Offset = getINTEGER_CSTVal(TREE_OPERAND(exp, 1));
    
    // If POINTER_SIZE is 32-bits, sign extend the offset.
    if (POINTER_SIZE == 32)
      Offset = (Offset << 32) >> 32;
    
    // Figure out how large the element pointed to is.
    const Type *ElTy = cast<PointerType>(LHS->getType())->getElementType();
    // We can't get the type size (and thus convert to using a GEP instr) from
    // pointers to opaque structs if the type isn't abstract.
    if (ElTy->isSized()) {
      int64_t EltSize = TD.getTypeSize(ElTy);
      
      // If EltSize exactly divides Offset, then we know that we can turn this
      // into a getelementptr instruction.
      int64_t EltOffset = EltSize ? Offset/EltSize : 0;
      if (EltOffset*EltSize == Offset) {
        // If this is a subtract, we want to step backwards.
        if (Opc == Instruction::Sub)
          EltOffset = -EltOffset;
        Constant *C = ConstantInt::get(Type::Int64Ty, EltOffset);
        Value *V = new GetElementPtrInst(LHS, C, "tmp", CurBB);
        return CastToType(Instruction::BitCast, V, TREE_TYPE(exp));
      }
    }
  }
  
  
  Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);

  const Type *IntPtrTy = TD.getIntPtrType();
  bool LHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 0)));
  bool RHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 1)));
  LHS = CastToAnyType(LHS, LHSIsSigned, IntPtrTy, false);
  RHS = CastToAnyType(RHS, RHSIsSigned, IntPtrTy, false);
  Value *V = BinaryOperator::create((Instruction::BinaryOps)Opc, LHS, RHS,
                                    "tmp", CurBB);
  return CastToType(Instruction::IntToPtr, V, ConvertType(TREE_TYPE(exp)));
}


Value *TreeToLLVM::EmitTruthOp(tree exp, unsigned Opc) {
  Value *LHS = Emit(TREE_OPERAND(exp, 0), 0);
  Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);
  bool LHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 0)));
  bool RHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 1)));
  
  // This is a truth operation like the strict &&,||,^^.  Convert to bool as
  // a test against zero
  LHS = new ICmpInst(ICmpInst::ICMP_NE, LHS, 
                     Constant::getNullValue(LHS->getType()), "toBool", CurBB);
  RHS = new ICmpInst(ICmpInst::ICMP_NE, RHS, 
                     Constant::getNullValue(RHS->getType()), "toBool", CurBB);
  
  Value *Res = BinaryOperator::create((Instruction::BinaryOps)Opc, LHS, RHS,
                                      "tmp", CurBB);
  return CastToType(Instruction::ZExt, Res, ConvertType(TREE_TYPE(exp)));
}


Value *TreeToLLVM::EmitShiftOp(tree exp, Value *DestLoc, unsigned Opc) {
  assert(DestLoc == 0 && "aggregate shift?");
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  assert(!isa<PointerType>(Ty) && "Pointer arithmetic!?");
  
  Value *LHS = Emit(TREE_OPERAND(exp, 0), 0);
  Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);
  if (RHS->getType() != LHS->getType())
    RHS = CastInst::createIntegerCast(RHS, LHS->getType(), false,
                                      RHS->getName()+".cast", CurBB);
  
  return BinaryOperator::create((Instruction::BinaryOps)Opc, LHS, RHS, "tmp", 
                                 CurBB);
}

Value *TreeToLLVM::EmitRotateOp(tree exp, unsigned Opc1, unsigned Opc2) {
  Value *In  = Emit(TREE_OPERAND(exp, 0), 0);
  Value* Amt = Emit(TREE_OPERAND(exp, 1), 0);
  if (Amt->getType() != In->getType())
    Amt = CastInst::createIntegerCast(Amt, In->getType(), false,
                                      Amt->getName()+".cast", CurBB);

  Value *TypeSize =
    ConstantInt::get(In->getType(), In->getType()->getPrimitiveSizeInBits());
  
  // Do the two shifts.
  Value *V1 = BinaryOperator::create((Instruction::BinaryOps)Opc1, In, Amt, 
                                     "tmp", CurBB);
  Value *OtherShift = BinaryOperator::createSub(TypeSize, Amt, "tmp", CurBB);
  Value *V2 = BinaryOperator::create((Instruction::BinaryOps)Opc2, In, 
                                     OtherShift, "tmp", CurBB);
  
  // Or the two together to return them.
  Value *Merge = BinaryOperator::createOr(V1, V2, "tmp", CurBB);
  return CastToUIntType(Merge, ConvertType(TREE_TYPE(exp)));
}

Value *TreeToLLVM::EmitMinMaxExpr(tree exp, unsigned UIPred, unsigned SIPred, 
                                  unsigned FPPred) {
  Value *LHS = Emit(TREE_OPERAND(exp, 0), 0);
  Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);

  const Type *Ty = ConvertType(TREE_TYPE(exp));

  // The LHS, RHS and Ty could be integer, floating or pointer typed. We need
  // to convert the LHS and RHS into the destination type before doing the 
  // comparison. Use CastInst::getCastOpcode to get this right.
  bool TyIsSigned  = !TYPE_UNSIGNED(TREE_TYPE(exp));
  bool LHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 0)));
  bool RHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 1)));
  Instruction::CastOps opcode = 
    CastInst::getCastOpcode(LHS, LHSIsSigned, Ty, TyIsSigned);
  LHS = CastToType(opcode, LHS, Ty);
  opcode = CastInst::getCastOpcode(RHS, RHSIsSigned, Ty, TyIsSigned);
  RHS = CastToType(opcode, RHS, Ty);
  
  Value *Compare;
  if (LHS->getType()->isFloatingPoint())
    Compare = new FCmpInst(FCmpInst::Predicate(FPPred), LHS, RHS, "tmp", CurBB);
  else if TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 0)))
    Compare = new ICmpInst(ICmpInst::Predicate(UIPred), LHS, RHS, "tmp", CurBB);
  else
    Compare = new ICmpInst(ICmpInst::Predicate(SIPred), LHS, RHS, "tmp", CurBB);

  return new SelectInst(Compare, LHS, RHS,
                        TREE_CODE(exp) == MAX_EXPR ? "max" : "min", CurBB);
}


Value *TreeToLLVM::EmitFLOOR_MOD_EXPR(tree exp, Value *DestLoc) {
  // Notation: FLOOR_MOD_EXPR <-> Mod, TRUNC_MOD_EXPR <-> Rem.
  
  // We express Mod in terms of Rem as follows: if RHS exactly divides LHS,
  // or the values of LHS and RHS have the same sign, then Mod equals Rem.
  // Otherwise Mod equals Rem + RHS.  This means that LHS Mod RHS traps iff
  // LHS Rem RHS traps.
  
  if (TYPE_UNSIGNED(TREE_TYPE(exp)))
    // LHS and RHS values must have the same sign if their type is unsigned.
    return EmitBinOp(exp, DestLoc, Instruction::URem);
  
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  Constant *Zero = ConstantInt::get(Ty, 0);
  
  Value *LHS = Emit(TREE_OPERAND(exp, 0), 0);
  Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);
  
  // The two possible values for Mod.
  Value *Rem = BinaryOperator::create(Instruction::SRem, LHS, RHS, "rem",CurBB);
  Value *RemPlusRHS = BinaryOperator::create(Instruction::Add, Rem, RHS, "tmp",
                                             CurBB);
  
  // HaveSameSign: (LHS >= 0) == (RHS >= 0).
  Value *LHSIsPositive = new ICmpInst(ICmpInst::ICMP_SGE, LHS, Zero, "tmp",
                                      CurBB);
  Value *RHSIsPositive = new ICmpInst(ICmpInst::ICMP_SGE, RHS, Zero, "tmp",
                                      CurBB);
  Value *HaveSameSign = new ICmpInst(ICmpInst::ICMP_EQ, LHSIsPositive,
                                     RHSIsPositive, "tmp", CurBB);
  
  // RHS exactly divides LHS iff Rem is zero.
  Value *RemIsZero = new ICmpInst(ICmpInst::ICMP_EQ, Rem, Zero, "tmp", CurBB);
  
  Value *SameAsRem = BinaryOperator::create(Instruction::Or, HaveSameSign,
                                            RemIsZero, "tmp", CurBB);
  return new SelectInst(SameAsRem, Rem, RemPlusRHS, "mod", CurBB);
}

Value *TreeToLLVM::EmitROUND_DIV_EXPR(tree exp) {
  // Notation: ROUND_DIV_EXPR <-> RDiv, TRUNC_DIV_EXPR <-> Div.
  
  // RDiv calculates LHS/RHS by rounding to the nearest integer.  Ties
  // are broken by rounding away from zero.  In terms of Div this means:
  //   LHS RDiv RHS = (LHS + (RHS Div 2)) Div RHS
  // if the values of LHS and RHS have the same sign; and
  //   LHS RDiv RHS = (LHS - (RHS Div 2)) Div RHS
  // if the values of LHS and RHS differ in sign.  The intermediate
  // expressions in these formulae can overflow, so some tweaking is
  // required to ensure correct results.  The details depend on whether
  // we are doing signed or unsigned arithmetic.
  
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  Constant *Zero = ConstantInt::get(Ty, 0);
  Constant *Two = ConstantInt::get(Ty, 2);
  
  Value *LHS = Emit(TREE_OPERAND(exp, 0), 0);
  Value *RHS = Emit(TREE_OPERAND(exp, 1), 0);
  
  if (!TYPE_UNSIGNED(TREE_TYPE(exp))) {
    // In the case of signed arithmetic, we calculate RDiv as follows:
    //   LHS RDiv RHS = (sign) ( (|LHS| + (|RHS| UDiv 2)) UDiv |RHS| ),
    // where sign is +1 if LHS and RHS have the same sign, -1 if their
    // signs differ.  Doing the computation unsigned ensures that there
    // is no overflow.
    
    // On some machines INT_MIN Div -1 traps.  You might expect a trap for
    // INT_MIN RDiv -1 too, but this implementation will not generate one.
    // Quick quiz question: what value is returned for INT_MIN RDiv -1?
    
    // Determine the signs of LHS and RHS, and whether they have the same sign.
    Value *LHSIsPositive = new ICmpInst(ICmpInst::ICMP_SGE, LHS, Zero, "tmp",
                                        CurBB);
    Value *RHSIsPositive = new ICmpInst(ICmpInst::ICMP_SGE, RHS, Zero, "tmp",
                                        CurBB);
    Value *HaveSameSign = new ICmpInst(ICmpInst::ICMP_EQ, LHSIsPositive,
                                       RHSIsPositive, "tmp", CurBB);
    
    // Calculate |LHS| ...
    Value *MinusLHS = BinaryOperator::createNeg(LHS, "tmp", CurBB);
    Value *AbsLHS = new SelectInst(LHSIsPositive, LHS, MinusLHS, "abs_" +
                                   LHS->getName(), CurBB);
    // ... and |RHS|
    Value *MinusRHS = BinaryOperator::createNeg(RHS, "tmp", CurBB);
    Value *AbsRHS = new SelectInst(RHSIsPositive, RHS, MinusRHS, "abs_" +
                                   RHS->getName(), CurBB);
    
    // Calculate AbsRDiv = (|LHS| + (|RHS| UDiv 2)) UDiv |RHS|.
    Value *HalfAbsRHS = BinaryOperator::create(Instruction::UDiv, AbsRHS, Two,
                                               "tmp", CurBB);
    Value *Numerator = BinaryOperator::create(Instruction::Add, AbsLHS,
                                              HalfAbsRHS, "tmp", CurBB);
    Value *AbsRDiv = BinaryOperator::create(Instruction::UDiv, Numerator,
                                            AbsRHS, "tmp", CurBB);
    
    // Return AbsRDiv or -AbsRDiv according to whether LHS and RHS have the
    // same sign or not.
    Value *MinusAbsRDiv = BinaryOperator::createNeg(AbsRDiv, "tmp", CurBB);
    return new SelectInst(HaveSameSign, AbsRDiv, MinusAbsRDiv, "rdiv", CurBB);
  } else {
    // In the case of unsigned arithmetic, LHS and RHS necessarily have the
    // same sign, however overflow is a problem.  We want to use the formula
    //   LHS RDiv RHS = (LHS + (RHS Div 2)) Div RHS,
    // but if LHS + (RHS Div 2) overflows then we get the wrong result.  Since
    // the use of a conditional branch seems to be unavoidable, we choose the
    // simple solution of explicitly checking for overflow, and using
    //   LHS RDiv RHS = ((LHS + (RHS Div 2)) - RHS) Div RHS + 1
    // if it occurred.
    
    // Usually the numerator is LHS + (RHS Div 2); calculate this.
    Value *HalfRHS = BinaryOperator::create(Instruction::UDiv, RHS, Two, "tmp",
                                            CurBB);
    Value *Numerator = BinaryOperator::create(Instruction::Add, LHS, HalfRHS,
                                              "tmp", CurBB);
    
    // Did the calculation overflow?
    Value *Overflowed = new ICmpInst(ICmpInst::ICMP_ULT, Numerator, HalfRHS,
                                     "tmp", CurBB);
    
    // If so, use (LHS + (RHS Div 2)) - RHS for the numerator instead.
    Value *AltNumerator = BinaryOperator::create(Instruction::Sub, Numerator,
                                                 RHS, "tmp", CurBB);
    Numerator = new SelectInst(Overflowed, AltNumerator, Numerator, "tmp",
                               CurBB);
    
    // Quotient = Numerator / RHS.
    Value *Quotient = BinaryOperator::create(Instruction::UDiv, Numerator, RHS,
                                             "tmp", CurBB);
    
    // Return Quotient unless we overflowed, in which case return Quotient + 1.
    return BinaryOperator::create(Instruction::Add, Quotient,
                                  CastToUIntType(Overflowed, Ty), "rdiv",
                                  CurBB);
  }
}

//===----------------------------------------------------------------------===//
//               ... Inline Assembly and Register Variables ...
//===----------------------------------------------------------------------===//


/// Reads from register variables are handled by emitting an inline asm node
/// that copies the value out of the specified register.
Value *TreeToLLVM::EmitReadOfRegisterVariable(tree decl, Value *DestLoc) {
  const Type *Ty = ConvertType(TREE_TYPE(decl));
  
  // If there was an error, return something bogus.
  if (ValidateRegisterVariable(decl)) {
    if (Ty->isFirstClassType())
      return UndefValue::get(Ty);
    return 0;   // Just don't copy something into DestLoc.
  }
  
  // Turn this into a 'tmp = call Ty asm "", "={reg}"()'.
  FunctionType *FTy = FunctionType::get(Ty, std::vector<const Type*>(),false);
  
  const char *Name = IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(decl));
  InlineAsm *IA = InlineAsm::get(FTy, "", "={"+std::string(Name)+"}", false);
  return new CallInst(IA, "tmp", CurBB);
}

/// Stores to register variables are handled by emitting an inline asm node
/// that copies the value into the specified register.
void TreeToLLVM::EmitModifyOfRegisterVariable(tree decl, Value *RHS) {
  // If there was an error, bail out.
  if (ValidateRegisterVariable(decl))
    return;
  
  // Turn this into a 'call void asm sideeffect "", "{reg}"(Ty %RHS)'.
  std::vector<const Type*> ArgTys;
  ArgTys.push_back(ConvertType(TREE_TYPE(decl)));
  FunctionType *FTy = FunctionType::get(Type::VoidTy, ArgTys, false);
  
  const char *Name = IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(decl));
  InlineAsm *IA = InlineAsm::get(FTy, "", "{"+std::string(Name)+"}", true);
  new CallInst(IA, RHS, "", CurBB);
}

/// ConvertInlineAsmStr - Convert the specified inline asm string to an LLVM
/// InlineAsm string.  The GNU style inline asm template string has the
/// following format:
///   %N (for N a digit) means print operand N in usual manner.
///   %=  means a unique number for the inline asm.
///   %lN means require operand N to be a CODE_LABEL or LABEL_REF
///       and print the label name with no punctuation.
///   %cN means require operand N to be a constant
///       and print the constant expression with no punctuation.
///   %aN means expect operand N to be a memory address
///       (not a memory reference!) and print a reference to that address.
///   %nN means expect operand N to be a constant and print a constant
///       expression for minus the value of the operand, with no other
///       punctuation.
/// Other %xN expressions are turned into LLVM ${N:x} operands.
///
static std::string ConvertInlineAsmStr(tree exp, unsigned NumOperands) {
  static unsigned InlineAsmCounter = 0U;
  unsigned InlineAsmNum = InlineAsmCounter++;

  tree str = ASM_STRING(exp);
  if (TREE_CODE(str) == ADDR_EXPR) str = TREE_OPERAND(str, 0);

  // ASM_INPUT_P - This flag is set if this is a non-extended ASM, which means
  // that the asm string should not be interpreted, other than to escape $'s.
  if (ASM_INPUT_P(exp)) {
    const char *InStr = TREE_STRING_POINTER(str);
    std::string Result;
    while (1) {
      switch (*InStr++) {
      case 0: return Result;                 // End of string.
      default: Result += InStr[-1]; break;   // Normal character.
      case '$': Result += "$$"; break;       // Escape '$' characters.
      }
    }
  }
  
  // Expand [name] symbolic operand names.
  str = resolve_asm_operand_names(str, ASM_OUTPUTS(exp), ASM_INPUTS(exp));

  const char *InStr = TREE_STRING_POINTER(str);
  
  std::string Result;
  while (1) {
    switch (*InStr++) {
    case 0: return Result;                 // End of string.
    default: Result += InStr[-1]; break;   // Normal character.
    case '$': Result += "$$"; break;       // Escape '$' characters.
#ifdef ASSEMBLER_DIALECT
    // Note that we can't escape to ${, because that is the syntax for vars.
    case '{': Result += "$("; break;       // Escape '{' character.
    case '}': Result += "$)"; break;       // Escape '}' character.
    case '|': Result += "$|"; break;       // Escape '|' character.
#endif
    case '%':                              // GCC escape character.
      char EscapedChar = *InStr++;
      if (EscapedChar == '%') {            // Escaped '%' character
        Result += '%';
      } else if (EscapedChar == '=') {     // Unique ID for the asm instance.
        Result += utostr(InlineAsmNum);
      } else if (ISALPHA(EscapedChar)) {
        // % followed by a letter and some digits. This outputs an operand in a
        // special way depending on the letter.  We turn this into LLVM ${N:o}
        // syntax.
        char *EndPtr;
        unsigned long OpNum = strtoul(InStr, &EndPtr, 10);
        
        if (InStr == EndPtr) {
          error("%Hoperand number missing after %%-letter",&EXPR_LOCATION(exp));
          return Result;
        } else if (OpNum >= NumOperands) {
          error("%Hoperand number out of range", &EXPR_LOCATION(exp));
          return Result;
        }
        Result += "${" + utostr(OpNum) + ":" + EscapedChar + "}";
        InStr = EndPtr;
      } else if (ISDIGIT(EscapedChar)) {
        char *EndPtr;
        unsigned long OpNum = strtoul(InStr-1, &EndPtr, 10);
        InStr = EndPtr;
        Result += "$" + utostr(OpNum);
#ifdef PRINT_OPERAND_PUNCT_VALID_P
      } else if (PRINT_OPERAND_PUNCT_VALID_P((unsigned char)EscapedChar)) {
        Result += "${:";
        Result += EscapedChar;
        Result += "}";
#endif
      } else {
        output_operand_lossage("invalid %%-code");
      }
      break;
    }
  }
}

/// CanonicalizeConstraint - If we can canonicalize the constraint into
/// something simpler, do so now.  This turns register classes with a single
/// register into the register itself, expands builtin constraints to multiple
/// alternatives, etc.
static std::string CanonicalizeConstraint(const char *Constraint) {
  std::string Result;
  
  // Skip over modifier characters.
  bool DoneModifiers = false;
  while (!DoneModifiers) {
    switch (*Constraint) {
    default: DoneModifiers = true; break;
    case '=': assert(0 && "Should be after '='s");
    case '+': assert(0 && "'+' should already be expanded");
    case '&':
    case '%':
    case '*':
    case '?':
    case '!':
      ++Constraint;
      break;
    case '#':  // No constraint letters left.
      return Result;
    }
  }
  
  while (*Constraint) {
    char ConstraintChar = *Constraint++;

    // 'g' is just short-hand for 'imr'.
    if (ConstraintChar == 'g') {
      Result += "imr";
      continue;
    }
  
    // See if this is a regclass constraint.
    unsigned RegClass;
    if (ConstraintChar == 'r')
      // REG_CLASS_FROM_CONSTRAINT doesn't support 'r' for some reason.
      RegClass = GENERAL_REGS;
    else 
      RegClass = REG_CLASS_FROM_CONSTRAINT(Constraint[-1], Constraint-1);
  
    if (RegClass == NO_REGS) {  // not a reg class.
      Result += ConstraintChar;
      continue;
    }

    // Look to see if the specified regclass has exactly one member, and if so,
    // what it is.  Cache this information in AnalyzedRegClasses once computed.
    static std::map<unsigned, int> AnalyzedRegClasses;
  
    std::map<unsigned, int>::iterator I =
      AnalyzedRegClasses.lower_bound(RegClass);
  
    int RegMember;
    if (I != AnalyzedRegClasses.end() && I->first == RegClass) {
      // We've already computed this, reuse value.
      RegMember = I->second;
    } else {
      // Otherwise, scan the regclass, looking for exactly one member.
      RegMember = -1;  // -1 => not a single-register class.
      for (unsigned j = 0; j != FIRST_PSEUDO_REGISTER; ++j)
        if (TEST_HARD_REG_BIT(reg_class_contents[RegClass], j)) {
          if (RegMember == -1) {
            RegMember = j;
          } else {
            RegMember = -1;
            break;
          }
        }
      // Remember this answer for the next query of this regclass.
      AnalyzedRegClasses.insert(I, std::make_pair(RegClass, RegMember));
    }

    // If we found a single register register class, return the register.
    if (RegMember != -1) {
      Result += '{';
      Result += reg_names[RegMember];
      Result += '}';
    } else {
      Result += ConstraintChar;
    }
  }
  
  return Result;
}


Value *TreeToLLVM::EmitASM_EXPR(tree exp) {
  unsigned NumInputs = list_length(ASM_INPUTS(exp));
  unsigned NumOutputs = list_length(ASM_OUTPUTS(exp));
  unsigned NumInOut = 0;
  
  /// Constraints - The output/input constraints, concatenated together in array
  /// form instead of list form.
  const char **Constraints =
    (const char **)alloca((NumOutputs + NumInputs) * sizeof(const char *));
  
  // FIXME: CHECK ALTERNATIVES, something akin to check_operand_nalternatives.
  
  std::vector<Value*> CallOps;
  std::vector<const Type*> CallArgTypes;
  std::string NewAsmStr = ConvertInlineAsmStr(exp, NumOutputs+NumInputs);
  std::string ConstraintStr;
  
  // StoreCallResultAddr - The pointer to store the result of the call through.
  Value *StoreCallResultAddr = 0;
  const Type *CallResultType = Type::VoidTy;
  
  // Process outputs.
  int ValNum = 0;
  for (tree Output = ASM_OUTPUTS(exp); Output; 
       Output = TREE_CHAIN(Output), ++ValNum) {
    tree Operand = TREE_VALUE(Output);
    tree type = TREE_TYPE(Operand);
    // If there's an erroneous arg, emit no insn.
    if (type == error_mark_node) return 0;
    
    // Parse the output constraint.
    const char *Constraint =
      TREE_STRING_POINTER(TREE_VALUE(TREE_PURPOSE(Output)));
    Constraints[ValNum] = Constraint;
    bool IsInOut, AllowsReg, AllowsMem;
    if (!parse_output_constraint(&Constraint, ValNum, NumInputs, NumOutputs,
                                 &AllowsMem, &AllowsReg, &IsInOut))
      return 0;
    
    assert(Constraint[0] == '=' && "Not an output constraint?");

    // Output constraints must be addressible if they aren't simple register
    // constraints (this emits "address of register var" errors, etc).
    if (!AllowsReg && (AllowsMem || IsInOut))
      lang_hooks.mark_addressable(Operand);

    // Count the number of "+" constraints.
    if (IsInOut)
      ++NumInOut, ++NumInputs;

    // If this output register is pinned to a machine register, use that machine
    // register instead of the specified constraint.
    if (TREE_CODE(Operand) == VAR_DECL && DECL_HARD_REGISTER(Operand)) {
      int RegNum = 
        decode_reg_name(IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(Operand)));
      if (RegNum >= 0) {
        unsigned RegNameLen = strlen(reg_names[RegNum]);
        char *NewConstraint = (char*)alloca(RegNameLen+4);
        NewConstraint[0] = '=';
        NewConstraint[1] = '{';
        memcpy(NewConstraint+2, reg_names[RegNum], RegNameLen);
        NewConstraint[RegNameLen+2] = '}';
        NewConstraint[RegNameLen+3] = 0;
        Constraint = NewConstraint;
      }
    }
    
    // If we can simplify the constraint into something else, do so now.  This
    // avoids LLVM having to know about all the (redundant) GCC constraints.
    std::string SimplifiedConstraint = '='+CanonicalizeConstraint(Constraint+1);
    
    LValue Dest = EmitLV(Operand);
    assert(!Dest.isBitfield() && "Cannot assign into a bitfield!");
    if (ConstraintStr.empty() && !AllowsMem) {  // Reg dest and no output yet?
      assert(StoreCallResultAddr == 0 && "Already have a result val?");
      StoreCallResultAddr = Dest.Ptr;
      ConstraintStr += ",";
      ConstraintStr += SimplifiedConstraint;
      CallResultType = cast<PointerType>(Dest.Ptr->getType())->getElementType();
    } else {
      ConstraintStr += ",=";
      ConstraintStr += SimplifiedConstraint;
      CallOps.push_back(Dest.Ptr);
      CallArgTypes.push_back(Dest.Ptr->getType());
    }
  }
  
  // Process inputs.
  for (tree Input = ASM_INPUTS(exp); Input; Input = TREE_CHAIN(Input),++ValNum){
    tree Val = TREE_VALUE(Input);
    tree type = TREE_TYPE(Val);
    // If there's an erroneous arg, emit no insn.
    if (type == error_mark_node) return 0;
    
    const char *Constraint =
      TREE_STRING_POINTER(TREE_VALUE(TREE_PURPOSE(Input)));
    Constraints[ValNum] = Constraint;

    bool AllowsReg, AllowsMem;
    if (!parse_input_constraint(Constraints+ValNum, ValNum-NumOutputs,
                                NumInputs, NumOutputs, NumInOut,
                                Constraints, &AllowsMem, &AllowsReg))
      return 0;
    
    if (AllowsReg || !AllowsMem) {    // Register operand.
      Value *Op = Emit(Val, 0);
      CallOps.push_back(Op);
      CallArgTypes.push_back(Op->getType());
    } else {                          // Memory operand.
      lang_hooks.mark_addressable(TREE_VALUE(Input));
      LValue Src = EmitLV(Val);
      assert(!Src.isBitfield() && "Cannot read from a bitfield!");
      CallOps.push_back(Src.Ptr);
      CallArgTypes.push_back(Src.Ptr->getType());
    }
    
    ConstraintStr += ",";
    
    // If this output register is pinned to a machine register, use that machine
    // register instead of the specified constraint.
    int RegNum;
    if (TREE_CODE(Val) == VAR_DECL && DECL_HARD_REGISTER(Val) &&
        (RegNum = 
         decode_reg_name(IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(Val)))) >= 0){
      ConstraintStr += '{';
      ConstraintStr += reg_names[RegNum];
      ConstraintStr += '}';
    } else {
      // If there is a simpler form for the register constraint, use it.
      std::string Simplified = CanonicalizeConstraint(Constraint);
      ConstraintStr += Simplified;
    }
  }
  
  if (ASM_USES(exp)) {
    // FIXME: Figure out what ASM_USES means.
    error("%Hcode warrior/ms asm not supported yet in %qs", &EXPR_LOCATION(exp),
          TREE_STRING_POINTER(ASM_STRING(exp)));
    return 0;
  }
  
  // Process clobbers.

  // Some targets automatically clobber registers across an asm.
  tree Clobbers = targetm.md_asm_clobbers(ASM_CLOBBERS(exp));
  for (; Clobbers; Clobbers = TREE_CHAIN(Clobbers)) {
    const char *RegName = TREE_STRING_POINTER(TREE_VALUE(Clobbers));
    int RegCode = decode_reg_name(RegName);
    
    switch (RegCode) {
    case -1:     // Nothing specified?
    case -2:     // Invalid.
      error("%Hunknown register name %qs in %<asm%>", &EXPR_LOCATION(exp), 
            RegName);
      return 0;
    case -3:     // cc
      ConstraintStr += ",~{cc}";
      break;
    case -4:     // memory
      ConstraintStr += ",~{memory}";
      break;
    default:     // Normal register name.
      ConstraintStr += ",~{";
      ConstraintStr += reg_names[RegCode];
      ConstraintStr += "}";
      break;
    }
  }
  
  const FunctionType *FTy = 
    FunctionType::get(CallResultType, CallArgTypes, false);
  
  // Remove the leading comma if we have operands.
  if (!ConstraintStr.empty())
    ConstraintStr.erase(ConstraintStr.begin());
  
  // Make sure we're created a valid inline asm expression.
  if (!InlineAsm::Verify(FTy, ConstraintStr)) {
    error("%HInvalid or unsupported inline assembly!", &EXPR_LOCATION(exp));
    return 0;
  }
  
  Value *Asm = InlineAsm::get(FTy, NewAsmStr, ConstraintStr,
                              ASM_VOLATILE_P(exp) || !ASM_OUTPUTS(exp));   
  CallInst *CV = new CallInst(Asm, &CallOps[0], CallOps.size(),
                              StoreCallResultAddr ? "tmp" : "", CurBB);
  
  // If the call produces a value, store it into the destination.
  if (StoreCallResultAddr)
    new StoreInst(CV, StoreCallResultAddr, CurBB);
  
  // Give the backend a chance to upgrade the inline asm to LLVM code.  This
  // handles some common cases that LLVM has intrinsics for, e.g. x86 bswap ->
  // llvm.bswap.
  if (const TargetAsmInfo *TAI = TheTarget->getTargetAsmInfo())
    TAI->ExpandInlineAsm(CV);
  
  return 0;
}

//===----------------------------------------------------------------------===//
//               ... Helpers for Builtin Function Expansion ...
//===----------------------------------------------------------------------===//

Value *TreeToLLVM::BuildVector(const std::vector<Value*> &Ops) {
  assert((Ops.size() & (Ops.size()-1)) == 0 &&
         "Not a power-of-two sized vector!");
  bool AllConstants = true;
  for (unsigned i = 0, e = Ops.size(); i != e && AllConstants; ++i)
    AllConstants &= isa<Constant>(Ops[i]);
    
  // If this is a constant vector, create a ConstantVector.
  if (AllConstants) {
    std::vector<Constant*> CstOps;
    for (unsigned i = 0, e = Ops.size(); i != e; ++i)
      CstOps.push_back(cast<Constant>(Ops[i]));
    return ConstantVector::get(CstOps);
  }
  
  // Otherwise, insertelement the values to build the vector.
  Value *Result = 
    UndefValue::get(VectorType::get(Ops[0]->getType(), Ops.size()));
  
  for (unsigned i = 0, e = Ops.size(); i != e; ++i)
    Result = new InsertElementInst(Result, Ops[i], 
                                   ConstantInt::get(Type::Int32Ty, i),
                                   "tmp", CurBB);
  
  return Result;
}

/// BuildVector - This varargs function builds a literal vector ({} syntax) with
/// the specified null-terminated list of elements.  The elements must be all
/// the same element type and there must be a power of two of them.
Value *TreeToLLVM::BuildVector(Value *Elt, ...) {
  std::vector<Value*> Ops;
  va_list VA;
  va_start(VA, Elt);
  
  Ops.push_back(Elt);
  while (Value *Arg = va_arg(VA, Value *))
    Ops.push_back(Arg);
  va_end(VA);
  
  return BuildVector(Ops);
}

/// BuildVectorShuffle - Given two vectors and a variable length list of int
/// constants, create a shuffle of the elements of the inputs, where each dest
/// is specified by the indexes.  The int constant list must be as long as the
/// number of elements in the input vector.
///
/// Undef values may be specified by passing in -1 as the result value.
///
Value *TreeToLLVM::BuildVectorShuffle(Value *InVec1, Value *InVec2, ...) {
  assert(isa<VectorType>(InVec1->getType()) && 
         InVec1->getType() == InVec2->getType() && "Invalid shuffle!");
  unsigned NumElements = cast<VectorType>(InVec1->getType())->getNumElements();

  // Get all the indexes from varargs.
  std::vector<Constant*> Idxs;
  va_list VA;
  va_start(VA, InVec2);
  for (unsigned i = 0; i != NumElements; ++i) {
    int idx = va_arg(VA, int);
    if (idx == -1)
      Idxs.push_back(UndefValue::get(Type::Int32Ty));
    else {
      assert((unsigned)idx < 2*NumElements && "Element index out of range!");
      Idxs.push_back(ConstantInt::get(Type::Int32Ty, idx));
    }
  }
  va_end(VA);

  // Turn this into the appropriate shuffle operation.
  return new ShuffleVectorInst(InVec1, InVec2, ConstantVector::get(Idxs),
                               "tmp", CurBB);
}

//===----------------------------------------------------------------------===//
//                     ... Builtin Function Expansion ...
//===----------------------------------------------------------------------===//

/// EmitFrontendExpandedBuiltinCall - For MD builtins that do not have a
/// directly corresponding LLVM intrinsic, we allow the target to do some amount
/// of lowering.  This allows us to avoid having intrinsics for operations that
/// directly correspond to LLVM constructs.
///
/// This method returns true if the builtin is handled, otherwise false.
///
bool TreeToLLVM::EmitFrontendExpandedBuiltinCall(tree exp, tree fndecl,
                                                 Value *DestLoc,Value *&Result){
#ifdef LLVM_TARGET_INTRINSIC_LOWER
  // Get the result type and oeprand line in an easy to consume format.
  const Type *ResultType = ConvertType(TREE_TYPE(TREE_TYPE(fndecl)));
  std::vector<Value*> Operands;
  SmallVector<tree, 8> Args;
  for (tree Op = TREE_OPERAND(exp, 1); Op; Op = TREE_CHAIN(Op)) {
    tree Arg = TREE_VALUE(Op);
    Args.push_back(Arg);
    Operands.push_back(Emit(Arg, 0));
  }
  
  bool ResIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_TYPE(fndecl)));
  bool ExpIsSigned = !TYPE_UNSIGNED(TREE_TYPE(exp));
  unsigned FnCode = DECL_FUNCTION_CODE(fndecl);
  return LLVM_TARGET_INTRINSIC_LOWER(exp, FnCode, DestLoc, Result, ResultType,
                                     Operands, Args, CurBB, ResIsSigned,
                                     ExpIsSigned);
#endif
  return false;
}

/// TargetBuiltinCache - A cache of builtin intrisics indexed by the GCC builtin
/// number.
static std::vector<Constant*> TargetBuiltinCache;

void clearTargetBuiltinCache() {
  TargetBuiltinCache.clear();
}

/// EmitBuiltinCall - exp is a call to fndecl, a builtin function.  Try to emit
/// the call in a special way, setting Result to the scalar result if necessary.
/// If we can't handle the builtin, return false, otherwise return true.
bool TreeToLLVM::EmitBuiltinCall(tree exp, tree fndecl,
                                 Value *DestLoc, Value *&Result) {
  if (DECL_BUILT_IN_CLASS(fndecl) == BUILT_IN_MD) {
    unsigned FnCode = DECL_FUNCTION_CODE(fndecl);
    if (TargetBuiltinCache.size() <= FnCode)
      TargetBuiltinCache.resize(FnCode+1);
    
    // If we haven't converted this intrinsic over yet, do so now.
    if (TargetBuiltinCache[FnCode] == 0) {
      const char *TargetPrefix = "";
#ifdef LLVM_TARGET_INTRINSIC_PREFIX
      TargetPrefix = LLVM_TARGET_INTRINSIC_PREFIX;
#endif
      // If this builtin directly corresponds to an LLVM intrinsic, get the
      // IntrinsicID now.
      const char *BuiltinName = IDENTIFIER_POINTER(DECL_NAME(fndecl));
      Intrinsic::ID IntrinsicID;
#define GET_LLVM_INTRINSIC_FOR_GCC_BUILTIN
#include "llvm/Intrinsics.gen"
#undef GET_LLVM_INTRINSIC_FOR_GCC_BUILTIN
      
      if (IntrinsicID == Intrinsic::not_intrinsic) {
        if (EmitFrontendExpandedBuiltinCall(exp, fndecl, DestLoc, Result))
          return true;
        
        std::cerr << "TYPE: " << *ConvertType(TREE_TYPE(fndecl)) << "\n";
        error("%Hunsupported target builtin %<%s%> used", &EXPR_LOCATION(exp),
              BuiltinName);
        const Type *ResTy = ConvertType(TREE_TYPE(exp));
        if (ResTy->isFirstClassType())
          Result = UndefValue::get(ResTy);
        return true;
      }
      
      // Finally, map the intrinsic ID back to a name.
      TargetBuiltinCache[FnCode] = 
        Intrinsic::getDeclaration(TheModule, IntrinsicID);
    }

    Result = EmitCallOf(TargetBuiltinCache[FnCode], exp, DestLoc);
    return true;
  }
  
  switch (DECL_FUNCTION_CODE(fndecl)) {
  default: return false;
  // Varargs builtins.
  case BUILT_IN_VA_START:
  case BUILT_IN_STDARG_START:   return EmitBuiltinVAStart(exp);
  case BUILT_IN_VA_END:         return EmitBuiltinVAEnd(exp);
  case BUILT_IN_VA_COPY:        return EmitBuiltinVACopy(exp);
  case BUILT_IN_CONSTANT_P:     return EmitBuiltinConstantP(exp, Result);
  case BUILT_IN_ALLOCA:         return EmitBuiltinAlloca(exp, Result);
  case BUILT_IN_EXTEND_POINTER: return EmitBuiltinExtendPointer(exp, Result);
  case BUILT_IN_EXPECT:         return EmitBuiltinExpect(exp, DestLoc, Result);
  case BUILT_IN_MEMCPY:         return EmitBuiltinMemCopy(exp, Result, false);
  case BUILT_IN_MEMMOVE:        return EmitBuiltinMemCopy(exp, Result, true);
  case BUILT_IN_MEMSET:         return EmitBuiltinMemSet(exp, Result);
  case BUILT_IN_BZERO:          return EmitBuiltinBZero(exp, Result);
  case BUILT_IN_PREFETCH:       return EmitBuiltinPrefetch(exp);
  case BUILT_IN_FRAME_ADDRESS:  return EmitBuiltinReturnAddr(exp, Result,true);
  case BUILT_IN_RETURN_ADDRESS: return EmitBuiltinReturnAddr(exp, Result,false);
  case BUILT_IN_STACK_SAVE:     return EmitBuiltinStackSave(exp, Result);
  case BUILT_IN_STACK_RESTORE:  return EmitBuiltinStackRestore(exp);
    
#define HANDLE_UNARY_FP(F32, F64, V) \
        Result = EmitBuiltinUnaryFPOp(V, Intrinsic::F32, Intrinsic::F64)

  // Unary bit counting intrinsics.
  // NOTE: do not merge these case statements.  That will cause the memoized 
  // Function* to be incorrectly shared across the different typed functions.
  case BUILT_IN_CLZ:       // These GCC builtins always return int.
  case BUILT_IN_CLZL:
  case BUILT_IN_CLZLL: {
    Value *Amt = Emit(TREE_VALUE(TREE_OPERAND(exp, 1)), 0);
    EmitBuiltinUnaryIntOp(Amt, Result, Intrinsic::ctlz); 
    return true;
  }
  case BUILT_IN_CTZ:       // These GCC builtins always return int.
  case BUILT_IN_CTZL:
  case BUILT_IN_CTZLL: {
    Value *Amt = Emit(TREE_VALUE(TREE_OPERAND(exp, 1)), 0);
    EmitBuiltinUnaryIntOp(Amt, Result, Intrinsic::cttz); 
    return true;
  }
  case BUILT_IN_POPCOUNT:  // These GCC builtins always return int.
  case BUILT_IN_POPCOUNTL:
  case BUILT_IN_POPCOUNTLL: {
    Value *Amt = Emit(TREE_VALUE(TREE_OPERAND(exp, 1)), 0);
    EmitBuiltinUnaryIntOp(Amt, Result, Intrinsic::ctpop); 
    return true;
  }
  case BUILT_IN_SQRT: 
  case BUILT_IN_SQRTF:
  case BUILT_IN_SQRTL:
    // If errno math has been disabled, expand these to llvm.sqrt calls.
    if (!flag_errno_math) {
      Value *Amt = Emit(TREE_VALUE(TREE_OPERAND(exp, 1)), 0);
      HANDLE_UNARY_FP(sqrt_f32, sqrt_f64, Amt);
      Result = CastToFPType(Result, ConvertType(TREE_TYPE(exp)));
      return true; 
    }
    break;
  case BUILT_IN_POWI:
  case BUILT_IN_POWIF:
  case BUILT_IN_POWIL:
    Result = EmitBuiltinPOWI(exp);
    return true;
  case BUILT_IN_FFS:  // These GCC builtins always return int.
  case BUILT_IN_FFSL:
  case BUILT_IN_FFSLL: {      // FFS(X) -> (x == 0 ? 0 : CTTZ(x)+1)
    // The argument and return type of cttz should match the argument type of
    // the ffs, but should ignore the return type of ffs.
    Value *Amt = Emit(TREE_VALUE(TREE_OPERAND(exp, 1)), 0);
    EmitBuiltinUnaryIntOp(Amt, Result, Intrinsic::cttz); 
    Result = BinaryOperator::createAdd(Result, 
                                       ConstantInt::get(Type::Int32Ty, 1),
                                       "tmp", CurBB);
    Value *Cond;
    if (Amt->getType()->isFloatingPoint())
      Cond = new FCmpInst(FCmpInst::FCMP_OEQ, Amt,
                          Constant::getNullValue(Amt->getType()), "tmp", CurBB);
    else 
      Cond = new ICmpInst(ICmpInst::ICMP_EQ, Amt, 
                          Constant::getNullValue(Amt->getType()), "tmp", CurBB);

    Result = new SelectInst(Cond, Constant::getNullValue(Type::Int32Ty),
                            Result, "tmp", CurBB);
    return true;
  }


#undef HANDLE_UNARY_FP

#if 1  // FIXME: Should handle these GCC extensions eventually.
    case BUILT_IN_APPLY_ARGS:
    case BUILT_IN_APPLY:
    case BUILT_IN_RETURN:
    case BUILT_IN_SAVEREGS:
    case BUILT_IN_ARGS_INFO:
    case BUILT_IN_NEXT_ARG:
    case BUILT_IN_CLASSIFY_TYPE:
    case BUILT_IN_AGGREGATE_INCOMING_ADDRESS:
    case BUILT_IN_SETJMP:
    case BUILT_IN_LONGJMP:
    case BUILT_IN_UPDATE_SETJMP_BUF:
    case BUILT_IN_TRAP:
      
      // Various hooks for the DWARF 2 __throw routine.
    case BUILT_IN_UNWIND_INIT:
    case BUILT_IN_DWARF_CFA:
#ifdef DWARF2_UNWIND_INFO
    case BUILT_IN_DWARF_SP_COLUMN:
    case BUILT_IN_INIT_DWARF_REG_SIZES:
#endif
    case BUILT_IN_FROB_RETURN_ADDR:
    case BUILT_IN_EXTRACT_RETURN_ADDR:
    case BUILT_IN_EH_RETURN:
#ifdef EH_RETURN_DATA_REGNO
    case BUILT_IN_EH_RETURN_DATA_REGNO:
#endif
      // FIXME: HACK: Just ignore these.
    {
      const Type *Ty = ConvertType(TREE_TYPE(exp));
      if (Ty != Type::VoidTy)
        Result = Constant::getNullValue(Ty);
      return true;
    }
#endif  // FIXME: Should handle these GCC extensions eventually.
  }
  return false;
}

bool TreeToLLVM::EmitBuiltinUnaryIntOp(Value *InVal, Value *&Result,
                                       Intrinsic::ID Id) {
  const Type *Ty = InVal->getType();
  
  const Type* Tys[2];
  Tys[0] = 0;  // Result type is i32, not variable, signal so.
  Tys[1] = Ty; // Parameter type is iAny so actual type must be specified here
  Result = new CallInst(Intrinsic::getDeclaration(TheModule, Id, Tys, 2),
                        InVal, "tmp", CurBB);
  
  return true;
}

Value *TreeToLLVM::EmitBuiltinUnaryFPOp(Value *Amt, const char *F32Name,
                                        const char *F64Name) {
  const char *Name = 0;
  
  switch (Amt->getType()->getTypeID()) {
  default: assert(0 && "Unknown FP type!");
  case Type::FloatTyID:  Name = F32Name; break;
  case Type::DoubleTyID: Name = F64Name; break;
  }
  
  return new CallInst(cast<Function>(
      TheModule->getOrInsertFunction(Name,Amt->getType(),
                                     Amt->getType(),
                                     NULL)), Amt, "tmp", CurBB);
}

Value *TreeToLLVM::EmitBuiltinUnaryFPOp(Value *Amt,
                                        Intrinsic::ID F32ID,
                                        Intrinsic::ID F64ID) {
  Intrinsic::ID Id = Intrinsic::not_intrinsic;
  
  switch (Amt->getType()->getTypeID()) {
  default: assert(0 && "Unknown FP type!");
  case Type::FloatTyID:  Id = F32ID; break;
  case Type::DoubleTyID: Id = F64ID; break;
  }

  return new CallInst(Intrinsic::getDeclaration(TheModule, Id),
                      Amt, "tmp", CurBB);
}

Value *TreeToLLVM::EmitBuiltinPOWI(tree exp) {
  tree ArgList = TREE_OPERAND (exp, 1);
  if (!validate_arglist(ArgList, REAL_TYPE, INTEGER_TYPE, VOID_TYPE))
    return 0;

  Value *Val = Emit(TREE_VALUE(ArgList), 0);
  Value *Pow = Emit(TREE_VALUE(TREE_CHAIN(ArgList)), 0);
  Pow = CastToSIntType(Pow, Type::Int32Ty);

  Intrinsic::ID Id = Intrinsic::not_intrinsic;

  switch (Val->getType()->getTypeID()) {
  default: assert(0 && "Unknown FP type!");
  case Type::FloatTyID:  Id = Intrinsic::powi_f32; break;
  case Type::DoubleTyID: Id = Intrinsic::powi_f64; break;
  }
  
  return new CallInst(Intrinsic::getDeclaration(TheModule, Id),
                      Val, Pow, "tmp", CurBB);
}


bool TreeToLLVM::EmitBuiltinConstantP(tree exp, Value *&Result) {
  Result = Constant::getNullValue(ConvertType(TREE_TYPE(exp)));
  return true;
}

bool TreeToLLVM::EmitBuiltinExtendPointer(tree exp, Value *&Result) {
  tree arglist = TREE_OPERAND(exp, 1);
  Value *Amt = Emit(TREE_VALUE(arglist), 0);
  bool AmtIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_VALUE(arglist)));
  bool ExpIsSigned = !TYPE_UNSIGNED(TREE_TYPE(exp));
  Result = CastToAnyType(Amt, AmtIsSigned, ConvertType(TREE_TYPE(exp)), 
                         ExpIsSigned);
  return true;
}

/// EmitBuiltinMemCopy - Emit an llvm.memcpy or llvm.memmove intrinsic, 
/// depending on the value of isMemMove.
bool TreeToLLVM::EmitBuiltinMemCopy(tree_node *exp, Value *&Result, 
                                    bool isMemMove) {
  tree arglist = TREE_OPERAND(exp, 1);
  if (!validate_arglist(arglist, POINTER_TYPE, POINTER_TYPE, 
                        INTEGER_TYPE, VOID_TYPE))
    return false;
  
  tree Dst = TREE_VALUE(arglist);
  tree Src = TREE_VALUE(TREE_CHAIN(arglist));
  unsigned SrcAlign = get_pointer_alignment(Src, BIGGEST_ALIGNMENT);
  unsigned DstAlign = get_pointer_alignment(Dst, BIGGEST_ALIGNMENT);
  
  // If the DST or SRC pointers are not pointer type, do this out of line.
  if (SrcAlign == 0 || DstAlign == 0) return false;

  Value *DstV = Emit(Dst, 0);
  Value *SrcV = Emit(Src, 0);
  Value *Len = Emit(TREE_VALUE(TREE_CHAIN(TREE_CHAIN(arglist))), 0);
  if (isMemMove)
    EmitMemMove(DstV, SrcV, Len, std::min(SrcAlign, DstAlign)/8);
  else
    EmitMemCpy(DstV, SrcV, Len, std::min(SrcAlign, DstAlign)/8);
  Result = DstV;
  return true;
}

bool TreeToLLVM::EmitBuiltinMemSet(tree_node *exp, Value *&Result) {
  tree arglist = TREE_OPERAND(exp, 1);
  if (!validate_arglist(arglist, POINTER_TYPE, INTEGER_TYPE, 
                        INTEGER_TYPE, VOID_TYPE))
    return false;
  
  tree Dst = TREE_VALUE(arglist);
  unsigned DstAlign = get_pointer_alignment(Dst, BIGGEST_ALIGNMENT);
  
  // If the DST pointer is not a pointer type, do this out of line.
  if (DstAlign == 0) return false;
  
  Value *DstV = Emit(Dst, 0);
  Value *Val = Emit(TREE_VALUE(TREE_CHAIN(arglist)), 0);
  Value *Len = Emit(TREE_VALUE(TREE_CHAIN(TREE_CHAIN(arglist))), 0);
  EmitMemSet(DstV, Val, Len, DstAlign/8);
  Result = DstV;
  return true;
}

bool TreeToLLVM::EmitBuiltinBZero(tree_node *exp, Value *&Result) {
  tree arglist = TREE_OPERAND(exp, 1);
  if (!validate_arglist(arglist, POINTER_TYPE, INTEGER_TYPE, VOID_TYPE))
    return false;
  
  tree Dst = TREE_VALUE(arglist);
  unsigned DstAlign = get_pointer_alignment(Dst, BIGGEST_ALIGNMENT);
  
  // If the DST pointer is not a pointer type, do this out of line.
  if (DstAlign == 0) return false;
  
  Value *DstV = Emit(Dst, 0);
  Value *Val = Constant::getNullValue(Type::Int32Ty);
  Value *Len = Emit(TREE_VALUE(TREE_CHAIN(arglist)), 0);
  EmitMemSet(DstV, Val, Len, DstAlign/8);
  return true;
}

bool TreeToLLVM::EmitBuiltinPrefetch(tree exp) {
  tree arglist = TREE_OPERAND(exp, 1);
  if (!validate_arglist(arglist, POINTER_TYPE, 0))
    return false;

  Value *Ptr = Emit(TREE_VALUE(arglist), 0);
  Value *ReadWrite = 0;
  Value *Locality = 0;
  
  if (TREE_CHAIN(arglist)) { // Args 1/2 are optional
    ReadWrite = Emit(TREE_VALUE(TREE_CHAIN(arglist)), 0);
    if (!isa<ConstantInt>(ReadWrite)) {
      error("second argument to %<__builtin_prefetch%> must be a constant");
      ReadWrite = 0;
    } else if (cast<ConstantInt>(ReadWrite)->getZExtValue() > 1) {
      warning ("invalid second argument to %<__builtin_prefetch%>;"
	       " using zero");
      ReadWrite = 0;
    } else {
      ReadWrite = ConstantExpr::getIntegerCast(cast<Constant>(ReadWrite),
                                               Type::Int32Ty, false);
    }
    
    if (TREE_CHAIN(TREE_CHAIN(arglist))) {
      Locality = Emit(TREE_VALUE(TREE_CHAIN(TREE_CHAIN(arglist))), 0);
      if (!isa<ConstantInt>(Locality)) {
        error("third argument to %<__builtin_prefetch%> must be a constant");
        Locality = 0;
      } else if (cast<ConstantInt>(Locality)->getZExtValue() > 3) {
        warning("invalid third argument to %<__builtin_prefetch%>; using 3");
        Locality = 0;
      } else {
        Locality = ConstantExpr::getIntegerCast(cast<Constant>(Locality),
                                                Type::Int32Ty, false);
      }
    }
  }
  
  // Default to highly local read.
  if (ReadWrite == 0)
    ReadWrite = Constant::getNullValue(Type::Int32Ty);
  if (Locality == 0)
    Locality = ConstantInt::get(Type::Int32Ty, 3);
  
  Ptr = CastToType(Instruction::BitCast, Ptr, PointerType::get(Type::Int8Ty));
  
  Value *Ops[3] = { Ptr, ReadWrite, Locality };
  new CallInst(Intrinsic::getDeclaration(TheModule, Intrinsic::prefetch),
               Ops, 3, "", CurBB);
  return true;
}

/// EmitBuiltinReturnAddr - Emit an llvm.returnaddress or llvm.frameaddress
/// instruction, depending on whether isFrame is true or not.
bool TreeToLLVM::EmitBuiltinReturnAddr(tree exp, Value *&Result, bool isFrame) {
  tree arglist = TREE_OPERAND(exp, 1);
  if (!validate_arglist(arglist, INTEGER_TYPE, VOID_TYPE))
    return false;
  
  ConstantInt *Level = dyn_cast<ConstantInt>(Emit(TREE_VALUE(arglist), 0));
  if (!Level) {
    if (isFrame)
      error("invalid argument to %<__builtin_frame_address%>");
    else
      error("invalid argument to %<__builtin_return_address%>");
    return false;
  }
  
  Result = new CallInst(Intrinsic::getDeclaration(TheModule,
                                                  !isFrame ?
                                                  Intrinsic::returnaddress :
                                                  Intrinsic::frameaddress),
                        Level, "tmp", CurBB);
  Result = CastToType(Instruction::BitCast, Result, TREE_TYPE(exp));
  return true;
}

bool TreeToLLVM::EmitBuiltinStackSave(tree exp, Value *&Result) {
  tree arglist = TREE_OPERAND(exp, 1);
  if (!validate_arglist(arglist, VOID_TYPE))
    return false;
  
  Result = new CallInst(Intrinsic::getDeclaration(TheModule,
                                                  Intrinsic::stacksave),
                        "tmp", CurBB);
  return true;
}

bool TreeToLLVM::EmitBuiltinStackRestore(tree exp) {
  tree arglist = TREE_OPERAND(exp, 1);
  if (!validate_arglist(arglist, POINTER_TYPE, VOID_TYPE))
    return false;
  
  Value *Ptr = Emit(TREE_VALUE(arglist), 0);
  Ptr = CastToType(Instruction::BitCast, Ptr, PointerType::get(Type::Int8Ty));

  new CallInst(Intrinsic::getDeclaration(TheModule, Intrinsic::stackrestore),
               Ptr, "", CurBB);
  return true;
}


bool TreeToLLVM::EmitBuiltinAlloca(tree exp, Value *&Result) {
  tree arglist = TREE_OPERAND(exp, 1);
  if (!validate_arglist(arglist, INTEGER_TYPE, VOID_TYPE))
    return false;
  Value *Amt = Emit(TREE_VALUE(arglist), 0);
  Amt = CastToSIntType(Amt, Type::Int32Ty);
  Result = new AllocaInst(Type::Int8Ty, Amt, "tmp", CurBB);
  return true;
}

bool TreeToLLVM::EmitBuiltinExpect(tree exp, Value *DestLoc, Value *&Result) {
  // Ignore the hint for now, just expand the expr.  This is safe, but not
  // optimal.
  tree arglist = TREE_OPERAND(exp, 1);
  if (arglist == NULL_TREE || TREE_CHAIN(arglist) == NULL_TREE)
    return true;
  Result = Emit(TREE_VALUE(arglist), DestLoc);
  return true;
}

bool TreeToLLVM::EmitBuiltinVAStart(tree exp) {
  tree arglist = TREE_OPERAND(exp, 1);
  tree fntype = TREE_TYPE(current_function_decl);
  
  if (TYPE_ARG_TYPES(fntype) == 0 ||
      (TREE_VALUE(tree_last(TYPE_ARG_TYPES(fntype))) == void_type_node)) {
    error("`va_start' used in function with fixed args");
    return true;
  }
  
  tree last_parm = tree_last(DECL_ARGUMENTS(current_function_decl));
  tree chain = TREE_CHAIN(arglist);

  // Check for errors.
  if (fold_builtin_next_arg (chain))
    return true;
  
  tree arg = TREE_VALUE(chain);
  
  Value *ArgVal = Emit(TREE_VALUE(arglist), 0);

  Constant *llvm_va_start_fn = Intrinsic::getDeclaration(TheModule,
                                                         Intrinsic::vastart);
  const Type *FTy =
    cast<PointerType>(llvm_va_start_fn->getType())->getElementType();
  ArgVal = CastToType(Instruction::BitCast, ArgVal,
                      PointerType::get(Type::Int8Ty));
  new CallInst(llvm_va_start_fn, ArgVal, "", CurBB);
  return true;
}

bool TreeToLLVM::EmitBuiltinVAEnd(tree exp) {
  Value *Arg = Emit(TREE_VALUE(TREE_OPERAND(exp, 1)), 0);
  Arg = CastToType(Instruction::BitCast, Arg,
                   PointerType::get(Type::Int8Ty));
  new CallInst(Intrinsic::getDeclaration(TheModule, Intrinsic::vaend),
               Arg, "", CurBB);
  return true;
}

bool TreeToLLVM::EmitBuiltinVACopy(tree exp) {
  tree Arg1T = TREE_VALUE(TREE_OPERAND(exp, 1));
  tree Arg2T = TREE_VALUE(TREE_CHAIN(TREE_OPERAND(exp, 1)));
  
  Value *Arg1 = Emit(Arg1T, 0);   // Emit the address of the destination.
  // The second arg of llvm.va_copy is a pointer to a valist.
  Value *Arg2;
  if (!isAggregateTreeType(TREE_TYPE(Arg2T))) {
    // Emit it as a value, then store it to a temporary slot.
    Value *V2 = Emit(Arg2T, 0);
    Arg2 = CreateTemporary(V2->getType());
    new StoreInst(V2, Arg2, CurBB);
  } else {
    // If the target has aggregate valists, emit the srcval directly into a
    // temporary.
    const Type *VAListTy = cast<PointerType>(Arg1->getType())->getElementType();
    Arg2 = CreateTemporary(VAListTy);
    Emit(Arg2T, Arg2);
  }
  
  static const Type *VPTy = PointerType::get(Type::Int8Ty);

  Arg1 = CastToType(Instruction::BitCast, Arg1, VPTy);
  Arg2 = CastToType(Instruction::BitCast, Arg2, VPTy);
  
  new CallInst(Intrinsic::getDeclaration(TheModule, Intrinsic::vacopy),
               Arg1, Arg2, "", CurBB);
  return true;
}


//===----------------------------------------------------------------------===//
//                      ... Complex Math Expressions ...
//===----------------------------------------------------------------------===//

void TreeToLLVM::EmitLoadFromComplex(Value *&Real, Value *&Imag,
                                     Value *SrcComplex, bool isVolatile) {
  Value *I0 = ConstantInt::get(Type::Int32Ty, 0);
  Value *I1 = ConstantInt::get(Type::Int32Ty, 1);
  
  Value *RealPtr = new GetElementPtrInst(SrcComplex, I0, I0, "real", CurBB);
  Real = new LoadInst(RealPtr, "real", isVolatile, CurBB);
  
  Value *ImagPtr = new GetElementPtrInst(SrcComplex, I0, I1, "real", CurBB);
  Imag = new LoadInst(ImagPtr, "imag", isVolatile, CurBB);
}

void TreeToLLVM::EmitStoreToComplex(Value *DestComplex, Value *Real,
                                    Value *Imag, bool isVolatile) {
  Value *I0 = ConstantInt::get(Type::Int32Ty, 0);
  Value *I1 = ConstantInt::get(Type::Int32Ty, 1);
  
  Value *RealPtr = new GetElementPtrInst(DestComplex, I0, I0, "real", CurBB);
  new StoreInst(Real, RealPtr, isVolatile, CurBB);
  
  Value *ImagPtr = new GetElementPtrInst(DestComplex, I0, I1, "real", CurBB);
  new StoreInst(Imag, ImagPtr, isVolatile, CurBB);
}


void TreeToLLVM::EmitCOMPLEX_EXPR(tree exp, Value *DestLoc) {
  Value *Real = Emit(TREE_OPERAND(exp, 0), 0);
  Value *Imag = Emit(TREE_OPERAND(exp, 1), 0);
  EmitStoreToComplex(DestLoc, Real, Imag, false);
}

void TreeToLLVM::EmitCOMPLEX_CST(tree exp, Value *DestLoc) {
  Value *Real = Emit(TREE_REALPART(exp), 0);
  Value *Imag = Emit(TREE_IMAGPART(exp), 0);
  EmitStoreToComplex(DestLoc, Real, Imag, false);
}

// EmitComplexBinOp - Note that this operands on binops like ==/!=, which return
// a bool, not a complex value.
Value *TreeToLLVM::EmitComplexBinOp(tree exp, Value *DestLoc) {
  const Type *ComplexTy = ConvertType(TREE_TYPE(TREE_OPERAND(exp, 0)));
  
  Value *LHSTmp = CreateTemporary(ComplexTy);
  Value *RHSTmp = CreateTemporary(ComplexTy);
  Emit(TREE_OPERAND(exp, 0), LHSTmp);
  Emit(TREE_OPERAND(exp, 1), RHSTmp);
  
  Value *LHSr, *LHSi;
  EmitLoadFromComplex(LHSr, LHSi, LHSTmp,
                      TREE_THIS_VOLATILE(TREE_OPERAND(exp, 0)));
  Value *RHSr, *RHSi;
  EmitLoadFromComplex(RHSr, RHSi, RHSTmp,
                      TREE_THIS_VOLATILE(TREE_OPERAND(exp, 1)));
  
  Value *DSTr, *DSTi;
  switch (TREE_CODE(exp)) {
  default: TODO(exp);
  case PLUS_EXPR: // (a+ib) + (c+id) = (a+c) + i(b+d)
    DSTr = BinaryOperator::createAdd(LHSr, RHSr, "tmpr", CurBB);
    DSTi = BinaryOperator::createAdd(LHSi, RHSi, "tmpi", CurBB);
    break;
  case MINUS_EXPR: // (a+ib) - (c+id) = (a-c) + i(b-d)
    DSTr = BinaryOperator::createSub(LHSr, RHSr, "tmpr", CurBB);
    DSTi = BinaryOperator::createSub(LHSi, RHSi, "tmpi", CurBB);
    break;
  case MULT_EXPR: { // (a+ib) * (c+id) = (ac-bd) + i(ad+cb)
    Value *Tmp1 = BinaryOperator::createMul(LHSr, RHSr, "tmp", CurBB); // a*c
    Value *Tmp2 = BinaryOperator::createMul(LHSi, RHSi, "tmp", CurBB); // b*d
    DSTr = BinaryOperator::createSub(Tmp1, Tmp2, "tmp", CurBB);  // ac-bd

    Value *Tmp3 = BinaryOperator::createMul(LHSr, RHSi, "tmp", CurBB); // a*d
    Value *Tmp4 = BinaryOperator::createMul(RHSr, LHSi, "tmp", CurBB); // c*b
    DSTi = BinaryOperator::createAdd(Tmp3, Tmp4, "tmp", CurBB); // ad+cb
    break;
  }
  case RDIV_EXPR: { // (a+ib) / (c+id) = ((ac+bd)/(cc+dd)) + i((bc-ad)/(cc+dd))
    Value *Tmp1 = BinaryOperator::createMul(LHSr, RHSr, "tmp", CurBB); // a*c
    Value *Tmp2 = BinaryOperator::createMul(LHSi, RHSi, "tmp", CurBB); // b*d
    Value *Tmp3 = BinaryOperator::createAdd(Tmp1, Tmp2, "tmp", CurBB); // ac+bd
    
    Value *Tmp4 = BinaryOperator::createMul(RHSr, RHSr, "tmp", CurBB); // c*c
    Value *Tmp5 = BinaryOperator::createMul(RHSi, RHSi, "tmp", CurBB); // d*d
    Value *Tmp6 = BinaryOperator::createAdd(Tmp4, Tmp5, "tmp", CurBB); // cc+dd
    DSTr = BinaryOperator::createFDiv(Tmp3, Tmp6, "tmp", CurBB);

    Value *Tmp7 = BinaryOperator::createMul(LHSi, RHSr, "tmp", CurBB); // b*c
    Value *Tmp8 = BinaryOperator::createMul(LHSr, RHSi, "tmp", CurBB); // a*d
    Value *Tmp9 = BinaryOperator::createSub(Tmp7, Tmp8, "tmp", CurBB); // bc-ad
    DSTi = BinaryOperator::createFDiv(Tmp9, Tmp6, "tmp", CurBB);
    break;
  }
  case EQ_EXPR:   // (a+ib) == (c+id) = (a == c) & (b == d)
    DSTr = new FCmpInst(FCmpInst::FCMP_OEQ, LHSr, RHSr, "tmpr", CurBB);
    DSTi = new FCmpInst(FCmpInst::FCMP_OEQ, LHSi, RHSi, "tmpi", CurBB);
    return BinaryOperator::createAnd(DSTr, DSTi, "tmp", CurBB);
  case NE_EXPR:   // (a+ib) != (c+id) = (a != c) | (b != d) 
    DSTr = new FCmpInst(FCmpInst::FCMP_ONE, LHSr, RHSr, "tmpr", CurBB);
    DSTi = new FCmpInst(FCmpInst::FCMP_ONE, LHSi, RHSi, "tmpi", CurBB);
    return BinaryOperator::createOr(DSTr, DSTi, "tmp", CurBB);
  }
  
  EmitStoreToComplex(DestLoc, DSTr, DSTi, false);
  return 0;
}


//===----------------------------------------------------------------------===//
//                         ... L-Value Expressions ...
//===----------------------------------------------------------------------===//

LValue TreeToLLVM::EmitLV_DECL(tree exp) {
  if (TREE_CODE(exp) == PARM_DECL || TREE_CODE(exp) == VAR_DECL || 
      TREE_CODE(exp) == CONST_DECL) {
    // If a static var's type was incomplete when the decl was written,
    // but the type is complete now, lay out the decl now.
    if (DECL_SIZE(exp) == 0 && COMPLETE_OR_UNBOUND_ARRAY_TYPE_P(TREE_TYPE(exp))
        && (TREE_STATIC(exp) || DECL_EXTERNAL(exp))) {
      layout_decl(exp, 0);
      
      // This mirrors code in layout_decl for munging the RTL.  Here we actually
      // emit a NEW declaration for the global variable, now that it has been
      // laid out.  We then tell the compiler to "forward" any uses of the old
      // global to this new one.
      if (Value *Val = DECL_LLVM_IF_SET(exp)) {
        //fprintf(stderr, "***\n*** SHOULD HANDLE GLOBAL VARIABLES!\n***\n");
        //assert(0 && "Reimplement this with replace all uses!");
#if 0
        SET_DECL_LLVM(exp, 0);
        // Create a new global variable declaration
        llvm_assemble_external(exp);
        V2GV(Val)->ForwardedGlobal = V2GV(DECL_LLVM(exp));
#endif
      }
    }
  }

  assert(!isGCC_SSA_Temporary(exp) &&
         "Cannot use an SSA temporary as an l-value");
  
  Value *Decl = DECL_LLVM(exp);
  if (Decl == 0) {
    if (errorcount || sorrycount) {
      const PointerType *Ty = PointerType::get(ConvertType(TREE_TYPE(exp)));
      return ConstantPointerNull::get(Ty);
    }
    assert(0 && "INTERNAL ERROR: Referencing decl that hasn't been laid out");
    abort();
  }
  
  // Ensure variable marked as used even if it doesn't go through a parser.  If
  // it hasn't been used yet, write out an external definition.
  if (!TREE_USED(exp)) {
    assemble_external(exp);
    TREE_USED(exp) = 1;
    Decl = DECL_LLVM(exp);
  }
  
  if (GlobalValue *GV = dyn_cast<GlobalValue>(Decl)) {
    // If this is an aggregate CONST_DECL, emit it to LLVM now.  GCC happens to
    // get this case right by forcing the initializer into memory.
    if (TREE_CODE(exp) == CONST_DECL) {
      if (DECL_INITIAL(exp) && GV->isDeclaration()) {
        emit_global_to_llvm(exp);
        Decl = DECL_LLVM(exp);     // Decl could have change if it changed type.
      }
    } else {
      // Otherwise, inform cgraph that we used the global.
      mark_decl_referenced(exp);
      if (tree ID = DECL_ASSEMBLER_NAME(exp))
        mark_referenced(ID);
    }
  }
  
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  // If we have "extern void foo", make the global have type {} instead of
  // type void.
  if (Ty == Type::VoidTy) Ty = StructType::get(std::vector<const Type*>(),
                                               false);
  const PointerType *PTy = PointerType::get(Ty);
  return CastToType(Instruction::BitCast, Decl, PTy);
}

LValue TreeToLLVM::EmitLV_ARRAY_REF(tree exp) {
  tree Array = TREE_OPERAND(exp, 0);
  tree Index = TREE_OPERAND(exp, 1);
  assert((TREE_CODE (TREE_TYPE(Array)) == ARRAY_TYPE ||
          TREE_CODE (TREE_TYPE(Array)) == POINTER_TYPE ||
          TREE_CODE (TREE_TYPE(Array)) == REFERENCE_TYPE) &&
         "Unknown ARRAY_REF!");
  
  // As an LLVM extension, we allow ARRAY_REF with a pointer as the first
  // operand.  This construct maps directly to a getelementptr instruction.
  Value *ArrayAddr;
  
  if (TREE_CODE(TREE_TYPE(Array)) == ARRAY_TYPE) {
    // First subtract the lower bound, if any, in the type of the index.
    tree LowerBound = array_ref_low_bound(exp);
    if (!integer_zerop(LowerBound))
      Index = fold(build2(MINUS_EXPR, TREE_TYPE(Index), Index, LowerBound));
    
    LValue ArrayAddrLV = EmitLV(Array);
    assert(!ArrayAddrLV.isBitfield() && "Arrays cannot be bitfields!");
    ArrayAddr = ArrayAddrLV.Ptr;
  } else {
    ArrayAddr = Emit(Array, 0);
  }
  
  Value *IndexVal = Emit(Index, 0);

  // FIXME: If UnitSize is a variable, or if it disagrees with the LLVM array
  // element type, insert explicit pointer arithmetic here.
  //tree ElementSizeInBytes = array_ref_element_size(exp);
  
  const Type *IntPtrTy = getTargetData().getIntPtrType();
  if (IndexVal->getType() != IntPtrTy) {
    if (TYPE_UNSIGNED(TREE_TYPE(Index))) // if the index is unsigned
      // ZExt it to retain its value in the larger type
      IndexVal = CastToUIntType(IndexVal, IntPtrTy);
    else
      // SExt it to retain its value in the larger type
      IndexVal = CastToSIntType(IndexVal, IntPtrTy);
  }

  // If this is an index into an array, codegen as a GEP.
  if (TREE_CODE(TREE_TYPE(Array)) == ARRAY_TYPE) {
    Value *Ptr;

    // Check for variable sized array reference.
    tree length = arrayLength(TREE_TYPE(Array));
    if (length && !host_integerp(length, 1)) {
      // Make sure that ArrayAddr is of type ElementTy*, then do a 2-index gep.
      ArrayAddr = BitCastToType(ArrayAddr, PointerType::get(Type::Int8Ty));
      Value *Scale = Emit(array_ref_element_size(exp), 0);
      if (Scale->getType() != IntPtrTy)
        Scale = CastToUIntType(Scale, IntPtrTy);

      IndexVal = BinaryOperator::createMul(IndexVal, Scale, "tmp", CurBB);
      Ptr = new GetElementPtrInst(ArrayAddr, IndexVal, "tmp", CurBB);
    } else {
      // Otherwise, this is not a variable-sized array, use a GEP to index.
      Ptr = new GetElementPtrInst(ArrayAddr, ConstantInt::get(Type::Int32Ty, 0),
                                  IndexVal, "tmp", CurBB);
    }

    // The result type is an ElementTy* in the case of an ARRAY_REF, an array
    // of ElementTy in the case of ARRAY_RANGE_REF.  Return the correct type.
    return BitCastToType(Ptr, PointerType::get(ConvertType(TREE_TYPE(exp))));
  }

  // Otherwise, this is an index off a pointer, codegen as a 2-idx GEP.
  assert(TREE_CODE(TREE_TYPE(Array)) == POINTER_TYPE ||
         TREE_CODE(TREE_TYPE(Array)) == REFERENCE_TYPE);
  tree IndexedType = TREE_TYPE(TREE_TYPE(Array));
  
  // If we are indexing over a fixed-size type, just use a GEP.
  if (TREE_CODE(TYPE_SIZE(IndexedType)) == INTEGER_CST) {
    const Type *PtrIndexedTy = PointerType::get(ConvertType(IndexedType));
    ArrayAddr = BitCastToType(ArrayAddr, PtrIndexedTy);
    return new GetElementPtrInst(ArrayAddr, IndexVal, "tmp", CurBB);
  }
  
  // Otherwise, just do raw, low-level pointer arithmetic.  FIXME: this could be
  // much nicer in cases like:
  //   float foo(int w, float A[][w], int g) { return A[g][0]; }
  
  ArrayAddr = BitCastToType(ArrayAddr, PointerType::get(Type::Int8Ty));
  Value *TypeSize = Emit(array_ref_element_size(exp), 0);

  if (TypeSize->getType() != IntPtrTy)
    TypeSize = CastToUIntType(TypeSize, IntPtrTy);
  
  IndexVal = BinaryOperator::createMul(IndexVal, TypeSize, "tmp", CurBB);
  Value *Ptr = new GetElementPtrInst(ArrayAddr, IndexVal, "tmp", CurBB);

  return BitCastToType(Ptr, PointerType::get(ConvertType(TREE_TYPE(exp))));
}

/// getFieldOffsetInBits - Return the offset (in bits) of a FIELD_DECL in a
/// structure.
static unsigned getFieldOffsetInBits(tree Field) {
  assert(DECL_FIELD_BIT_OFFSET(Field) != 0 && DECL_FIELD_OFFSET(Field) != 0);
  unsigned Result = TREE_INT_CST_LOW(DECL_FIELD_BIT_OFFSET(Field));
  if (TREE_CODE(DECL_FIELD_OFFSET(Field)) == INTEGER_CST)
    Result += TREE_INT_CST_LOW(DECL_FIELD_OFFSET(Field))*8;
  return Result;
}

/// getComponentRefOffsetInBits - Return the offset (in bits) of the field
/// referenced in a COMPONENT_REF exp.
static unsigned getComponentRefOffsetInBits(tree exp) {
  assert(TREE_CODE(exp) == COMPONENT_REF && "not a COMPONENT_REF!");
  tree field = TREE_OPERAND(exp, 1);
  assert(TREE_CODE(field) == FIELD_DECL && "not a FIELD_DECL!");
  tree field_offset = component_ref_field_offset (exp);
  assert(DECL_FIELD_BIT_OFFSET(field) && field_offset);
  unsigned Result = TREE_INT_CST_LOW(DECL_FIELD_BIT_OFFSET(field));
  if (TREE_CODE(field_offset) == INTEGER_CST)
    Result += TREE_INT_CST_LOW(field_offset)*8;
  return Result;
}

LValue TreeToLLVM::EmitLV_COMPONENT_REF(tree exp) {
  LValue StructAddrLV = EmitLV(TREE_OPERAND(exp, 0));
  tree FieldDecl = TREE_OPERAND(exp, 1);

  assert((TREE_CODE(DECL_CONTEXT(FieldDecl)) == RECORD_TYPE ||
          TREE_CODE(DECL_CONTEXT(FieldDecl)) == UNION_TYPE));
   
  // Ensure that the struct type has been converted, so that the fielddecls
  // are laid out.  Note that we convert to the context of the Field, not to the
  // type of Operand #0, because GCC doesn't always have the field match up with
  // operand #0's type.
  const Type *StructTy = ConvertType(DECL_CONTEXT(FieldDecl));
  
  assert((!StructAddrLV.isBitfield() || 
          StructAddrLV.BitStart == 0) && "structs cannot be bitfields!");

  StructAddrLV.Ptr = CastToType(Instruction::BitCast, StructAddrLV.Ptr, 
                                PointerType::get(StructTy));
  const Type *FieldTy = ConvertType(TREE_TYPE(FieldDecl));
  
  // BitStart - This is the actual offset of the field from the start of the
  // struct, in bits.  For bitfields this may be on a non-byte boundary.
  unsigned BitStart = getComponentRefOffsetInBits(exp);
  Value *FieldPtr;
  
  tree field_offset = component_ref_field_offset (exp);
  // If this is a normal field at a fixed offset from the start, handle it.
  if (TREE_CODE(field_offset) == INTEGER_CST) {
    assert(DECL_LLVM_SET_P(FieldDecl) && "Struct not laid out for LLVM?");
    ConstantInt *CI = cast<ConstantInt>(DECL_LLVM(FieldDecl));
    uint32_t MemberIndex = CI->getZExtValue();
    assert(MemberIndex < StructTy->getNumContainedTypes() &&
           "Field Idx out of range!");
    FieldPtr = new GetElementPtrInst(StructAddrLV.Ptr,
                                     Constant::getNullValue(Type::Int32Ty), CI,
                                     "tmp", CurBB);
    
    // Now that we did an offset from the start of the struct, subtract off
    // the offset from BitStart.
    if (MemberIndex) {
      const StructLayout *SL = TD.getStructLayout(cast<StructType>(StructTy));
      BitStart -= SL->getElementOffset(MemberIndex) * 8;
    }
    
  } else {
    Value *Offset = Emit(field_offset, 0);
    Value *Ptr = CastToType(Instruction::PtrToInt, StructAddrLV.Ptr, 
                            Offset->getType());
    Ptr = BinaryOperator::createAdd(Ptr, Offset, "tmp", CurBB);
    FieldPtr = CastToType(Instruction::IntToPtr, Ptr,PointerType::get(FieldTy));
  }

  if (tree DeclaredType = DECL_BIT_FIELD_TYPE(FieldDecl)) {
    const Type *LLVMFieldTy = 
      cast<PointerType>(FieldPtr->getType())->getElementType();
    
    // If this is a bitfield, the declared type must be an integral type.
    FieldTy = ConvertType(DeclaredType);
    // If the field result is a bool, cast to a ubyte instead.  It is not
    // possible to access all bits of a memory object with a bool (only the low
    // bit) but it is possible to access them with a byte.
    if (FieldTy == Type::Int1Ty)
      FieldTy = Type::Int8Ty;
    assert(FieldTy->isInteger() && "Invalid bitfield");

    // If the LLVM notion of the field type is larger than the actual field type
    // being accessed, use the LLVM type.  This avoids pointer casts and other
    // bad things that are difficult to clean up later.  This occurs in cases
    // like "struct X{ unsigned long long x:50; unsigned y:2; }" when accessing
    // y.  We want to access the field as a ulong, not as a uint with an offset.
    if (LLVMFieldTy->isInteger() &&
        LLVMFieldTy->getPrimitiveSizeInBits() > 
          FieldTy->getPrimitiveSizeInBits())
      FieldTy = LLVMFieldTy;
    
    // We are now loading/storing through a casted pointer type, whose 
    // signedness depends on the signedness of the field.  Force the field to 
    // be unsigned.  This solves performance problems where you have, for 
    // example:  struct { int A:1; unsigned B:2; };  Consider a store to A then
    // a store to B.  In this case, without this conversion, you'd have a 
    // store through an int*, followed by a load from a uint*.  Forcing them
    // both to uint* allows the store to be forwarded to the load.
    
    // If this is a bitfield, the field may span multiple fields in the LLVM
    // type.  As such, cast the pointer to be a pointer to the declared type.
    FieldPtr = CastToType(Instruction::BitCast, FieldPtr, 
                          PointerType::get(FieldTy));
    
    // If this is a normal bitfield reference, return it as such.
    if (DECL_SIZE(FieldDecl) && TREE_CODE(DECL_SIZE(FieldDecl)) == INTEGER_CST){
      unsigned LLVMValueBitSize = FieldTy->getPrimitiveSizeInBits();
      unsigned BitfieldSize = TREE_INT_CST_LOW(DECL_SIZE(FieldDecl));
      // Finally, because bitfields can span LLVM fields, and because the start
      // of the first LLVM field (where FieldPtr currently points) may be up to
      // 63 bits away from the start of the bitfield), it is possible that
      // *FieldPtr doesn't contain all of the bits for this bitfield. If needed,
      // adjust FieldPtr so that it is close enough to the bitfield that
      // *FieldPtr contains all of the needed bits.  Be careful to make sure
      // that the pointer remains appropriately aligned.
      if (BitStart+BitfieldSize > LLVMValueBitSize) {
        // In this case, we know that the alignment of the field is less than
        // the size of the field.  To get the pointer close enough, add some
        // number of alignment units to the pointer.
        unsigned ByteAlignment = TD.getABITypeAlignment(FieldTy);
        assert(ByteAlignment*8 <= LLVMValueBitSize && "Unknown overlap case!");
        unsigned NumAlignmentUnits = BitStart/(ByteAlignment*8);
        assert(NumAlignmentUnits && "Not adjusting pointer?");
        
        // Compute the byte offset, and add it to the pointer.
        unsigned ByteOffset = NumAlignmentUnits*ByteAlignment;
        
        Constant *Offset = ConstantInt::get(TD.getIntPtrType(), ByteOffset);
        FieldPtr = CastToType(Instruction::PtrToInt, FieldPtr, 
                              Offset->getType());
        FieldPtr = BinaryOperator::createAdd(FieldPtr, Offset, "tmp", CurBB);
        FieldPtr = CastToType(Instruction::IntToPtr, FieldPtr, 
                              PointerType::get(FieldTy));
        
        // Adjust bitstart to account for the pointer movement.
        BitStart -= ByteOffset*8;

        // Check that this worked.
        assert(BitStart < LLVMValueBitSize &&
               BitStart+BitfieldSize <= LLVMValueBitSize &&
               "Couldn't get bitfield into value!");
      }
      
      // Okay, everything is good.  Return this as a bitfield if we can't
      // return it as a normal l-value. (e.g. "struct X { int X : 32 };" ).
      if (BitfieldSize != LLVMValueBitSize || BitStart != 0)
        return LValue(FieldPtr, BitStart, BitfieldSize);
    }
  } else {
    // Make sure we return a pointer to the right type.
    FieldPtr = CastToType(Instruction::BitCast, FieldPtr,
                          PointerType::get(ConvertType(TREE_TYPE(exp))));
  }
  
  assert(BitStart == 0 &&
         "It's a bitfield reference or we didn't get to the field!");
  return LValue(FieldPtr);
}

LValue TreeToLLVM::EmitLV_BIT_FIELD_REF(tree exp) {
  LValue Ptr = EmitLV(TREE_OPERAND(exp, 0));
  assert(!Ptr.isBitfield() && "BIT_FIELD_REF operands cannot be bitfields!");
  
  unsigned BitStart = (unsigned)TREE_INT_CST_LOW(TREE_OPERAND(exp, 2));
  unsigned BitSize = (unsigned)TREE_INT_CST_LOW(TREE_OPERAND(exp, 1));
  const Type *ValTy = ConvertType(TREE_TYPE(exp));
  
  unsigned ValueSizeInBits = 8*TD.getTypeSize(ValTy);
  assert(BitSize <= ValueSizeInBits &&
         "ValTy isn't large enough to hold the value loaded!");

  // BIT_FIELD_REF values can have BitStart values that are quite large.  We
  // know that the thing we are loading is ValueSizeInBits large.  If BitStart
  // is larger than ValueSizeInBits, bump the pointer over to where it should
  // be.
  if (unsigned UnitOffset = BitStart / ValueSizeInBits) {
    // TODO: If Ptr.Ptr is a struct type or something, we can do much better
    // than this.  e.g. check out when compiling unwind-dw2-fde-darwin.c.
    Ptr.Ptr = CastToType(Instruction::BitCast, Ptr.Ptr, 
                         PointerType::get(ValTy));
    Ptr.Ptr = new GetElementPtrInst(Ptr.Ptr,
                                    ConstantInt::get(Type::Int32Ty, UnitOffset),
                                    "tmp", CurBB);
    BitStart -= UnitOffset*ValueSizeInBits;
  }
  
  // If this is referring to the whole field, return the whole thing.
  if (BitStart == 0 && BitSize == ValueSizeInBits)
    return LValue(BitCastToType(Ptr.Ptr, PointerType::get(ValTy)));
  
  return LValue(BitCastToType(Ptr.Ptr, PointerType::get(ValTy)), BitStart, 
                BitSize);
}

LValue TreeToLLVM::EmitLV_XXXXPART_EXPR(tree exp, unsigned Idx) {
  LValue Ptr = EmitLV(TREE_OPERAND(exp, 0));
  assert(!Ptr.isBitfield() && "BIT_FIELD_REF operands cannot be bitfields!");

  return LValue(new GetElementPtrInst(Ptr.Ptr, 
                                      ConstantInt::get(Type::Int32Ty, 0),
                                      ConstantInt::get(Type::Int32Ty, Idx),
                                      "tmp", CurBB));
}

LValue TreeToLLVM::EmitLV_VIEW_CONVERT_EXPR(tree exp) {
  // The address is the address of the operand.
  LValue LV = EmitLV(TREE_OPERAND(exp, 0));
  // The type is the type of the expression.
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  LV.Ptr = BitCastToType(LV.Ptr, PointerType::get(Ty));
  return LV;
}

//===----------------------------------------------------------------------===//
//                       ... Constant Expressions ...
//===----------------------------------------------------------------------===//

/// EmitCONSTRUCTOR - emit the constructor into the location specified by
/// DestLoc.
Value *TreeToLLVM::EmitCONSTRUCTOR(tree exp, Value *DestLoc) {
  tree type = TREE_TYPE(exp);
  const Type *Ty = ConvertType(type);
  if (const VectorType *PTy = dyn_cast<VectorType>(Ty)) {
    assert(DestLoc == 0 && "Dest location for packed value?");
    
    std::vector<Value *> BuildVecOps;
    
    // Insert zero initializers for any uninitialized values.
    Constant *Zero = Constant::getNullValue(PTy->getElementType());
    BuildVecOps.resize(cast<VectorType>(Ty)->getNumElements(), Zero);
    
    // Insert all of the elements here.
    for (tree Ops = TREE_OPERAND(exp, 0); Ops; Ops = TREE_CHAIN(Ops)) {
      if (!TREE_PURPOSE(Ops)) continue;  // Not actually initialized?
      
      unsigned FieldNo = TREE_INT_CST_LOW(TREE_PURPOSE(Ops));

      // Update the element.
      if (FieldNo < BuildVecOps.size())
        BuildVecOps[FieldNo] = Emit(TREE_VALUE(Ops), 0);
    }
    
    return BuildVector(BuildVecOps);
  }

  assert(!Ty->isFirstClassType() && "Constructor for scalar type??");
  
  // Start out with the value zero'd out.
  EmitAggregateZero(DestLoc, type);
  
  tree elt = CONSTRUCTOR_ELTS(exp);
  switch (TREE_CODE(TREE_TYPE(exp))) {
  case ARRAY_TYPE:
  case RECORD_TYPE:
  default:
    if (elt) {
      // We don't handle elements yet.
      TODO(exp);
    }
    return 0;
  case UNION_TYPE:
    // Store each element of the constructor into the corresponding field of
    // DEST.
    if (elt == NULL_TREE) return 0;  // no elements
    assert(TREE_CHAIN(elt) == 0 &&"Union CONSTRUCTOR should have one element!");
    if (!TREE_PURPOSE(elt)) return 0;  // Not actually initialized?
      
    if (!ConvertType(TREE_TYPE(TREE_PURPOSE(elt)))->isFirstClassType()) {
      Value *V = Emit(TREE_VALUE(elt), DestLoc);
      assert(V == 0 && "Aggregate value returned in a register?");
    } else {
      // Scalar value.  Evaluate to a register, then do the store.
      Value *V = Emit(TREE_VALUE(elt), 0);
      DestLoc = CastToType(Instruction::BitCast, DestLoc, 
                           PointerType::get(V->getType()));
      new StoreInst(V, DestLoc, CurBB);
    }
    break;
  }
  return 0;
}

Constant *TreeConstantToLLVM::Convert(tree exp) {
  // Some front-ends use constants other than the standard language-independent
  // varieties, but which may still be output directly.  Give the front-end a
  // chance to convert EXP to a language-independent representation.
  exp = lang_hooks.expand_constant (exp);

  assert((TREE_CONSTANT(exp) || TREE_CODE(exp) == STRING_CST) &&
         "Isn't a constant!");
  switch (TREE_CODE(exp)) {
  case FDESC_EXPR:    // Needed on itanium
  default: 
    debug_tree(exp); 
    assert(0 && "Unknown constant to convert!");
    abort();
  case INTEGER_CST:   return ConvertINTEGER_CST(exp);
  case REAL_CST:      return ConvertREAL_CST(exp);
  case VECTOR_CST:    return ConvertVECTOR_CST(exp);
  case STRING_CST:    return ConvertSTRING_CST(exp);
  case COMPLEX_CST:   return ConvertCOMPLEX_CST(exp);
  case NOP_EXPR:      return ConvertNOP_EXPR(exp);
  case CONVERT_EXPR:  return ConvertCONVERT_EXPR(exp);
  case PLUS_EXPR:
  case MINUS_EXPR:    return ConvertBinOp_CST(exp);
  case CONSTRUCTOR:   return ConvertCONSTRUCTOR(exp);
  case VIEW_CONVERT_EXPR: return Convert(TREE_OPERAND(exp, 0));
  case ADDR_EXPR:     
    return ConstantExpr::getBitCast(EmitLV(TREE_OPERAND(exp, 0)),
                                 ConvertType(TREE_TYPE(exp)));
  }
}

Constant *TreeConstantToLLVM::ConvertINTEGER_CST(tree exp) {
  // Convert it to a uint64_t.
  uint64_t IntValue = getINTEGER_CSTVal(exp);
  
  // Build the value as a ulong constant, then constant fold it to the right
  // type.  This handles overflow and other things appropriately.
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  ConstantInt *C = ConstantInt::get(Type::Int64Ty, IntValue);
  // The destination type can be a pointer, integer or floating point 
  // so we need a generalized cast here
  Instruction::CastOps opcode = CastInst::getCastOpcode(C, false, Ty,
      !TYPE_UNSIGNED(TREE_TYPE(exp)));
  return ConstantExpr::getCast(opcode, C, Ty);
}

Constant *TreeConstantToLLVM::ConvertREAL_CST(tree exp) {
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  assert(Ty->isFloatingPoint() && "Integer REAL_CST?");
  long RealArr[2];
  union {
    int UArr[2];
    double V;
  };
  REAL_VALUE_TO_TARGET_DOUBLE(TREE_REAL_CST(exp), RealArr);
  
  // Here's how this works:
  // REAL_VALUE_TO_TARGET_DOUBLE() will generate the floating point number
  //  as an array of integers in the hosts's representation.  Each integer
  // in the array will hold 32 bits of the result REGARDLESS OF THE HOST'S
  //  INTEGER SIZE.
  //
  // This, then, makes the conversion pretty simple.  The tricky part is
  // getting the byte ordering correct and make sure you don't print any
  // more than 32 bits per integer on platforms with ints > 32 bits.
  //
  bool HostBigEndian = false;
#ifdef HOST_WORDS_BIG_ENDIAN
  HostBigEndian = true;
#endif
  
  UArr[0] = RealArr[0];   // Long -> int convert
  UArr[1] = RealArr[1];

  if (WORDS_BIG_ENDIAN != HostBigEndian)
    std::swap(UArr[0], UArr[1]);
  
  return ConstantFP::get(Ty, V);
}

Constant *TreeConstantToLLVM::ConvertVECTOR_CST(tree exp) {
  std::vector<Constant*> Elts;

  for (tree elt = TREE_VECTOR_CST_ELTS(exp); elt; elt = TREE_CHAIN(elt))
    Elts.push_back(Convert(TREE_VALUE(elt)));
  return ConstantVector::get(Elts);
}

Constant *TreeConstantToLLVM::ConvertSTRING_CST(tree exp) {
  const ArrayType *StrTy = cast<ArrayType>(ConvertType(TREE_TYPE(exp)));
  const Type *ElTy = StrTy->getElementType();
  
  unsigned Len = (unsigned)TREE_STRING_LENGTH(exp);
  
  std::vector<Constant*> Elts;
  if (ElTy == Type::Int8Ty) {
    const signed char *InStr = (const signed char *)TREE_STRING_POINTER(exp);
    for (unsigned i = 0; i != Len; ++i)
      Elts.push_back(ConstantInt::get(Type::Int8Ty, InStr[i]));
  } else if (ElTy == Type::Int8Ty) {
    const unsigned char *InStr =(const unsigned char *)TREE_STRING_POINTER(exp);
    for (unsigned i = 0; i != Len; ++i)
      Elts.push_back(ConstantInt::get(Type::Int8Ty, InStr[i]));
  } else if (ElTy == Type::Int16Ty) {
    const signed short *InStr = (const signed short *)TREE_STRING_POINTER(exp);
    for (unsigned i = 0; i != Len; ++i)
      Elts.push_back(ConstantInt::get(Type::Int16Ty, InStr[i]));
  } else if (ElTy == Type::Int16Ty) {
    const unsigned short *InStr =
      (const unsigned short *)TREE_STRING_POINTER(exp);
    for (unsigned i = 0; i != Len; ++i)
      Elts.push_back(ConstantInt::get(Type::Int16Ty, InStr[i]));
  } else if (ElTy == Type::Int32Ty) {
    const signed *InStr = (const signed *)TREE_STRING_POINTER(exp);
    for (unsigned i = 0; i != Len; ++i)
      Elts.push_back(ConstantInt::get(Type::Int32Ty, InStr[i]));
  } else if (ElTy == Type::Int32Ty) {
    const unsigned *InStr = (const unsigned *)TREE_STRING_POINTER(exp);
    for (unsigned i = 0; i != Len; ++i)
      Elts.push_back(ConstantInt::get(Type::Int32Ty, InStr[i]));
  } else {
    assert(0 && "Unknown character type!");
  }
  
  unsigned ConstantSize = StrTy->getNumElements();
  
  if (Len != ConstantSize) {
    // If this is a variable sized array type, set the length to Len.
    if (ConstantSize == 0) {
      tree Domain = TYPE_DOMAIN(TREE_TYPE(exp));
      if (!Domain || !TYPE_MAX_VALUE(Domain)) {
        ConstantSize = Len;
        StrTy = ArrayType::get(ElTy, Len);
      }
    }
    
    if (ConstantSize < Len) {
      // Only some chars are being used, truncate the string: char X[2] = "foo";
      Elts.resize(ConstantSize);
    } else {
      // Fill the end of the string with nulls.
      Constant *C = Constant::getNullValue(ElTy);
      for (; Len != ConstantSize; ++Len)
        Elts.push_back(C);
    }
  }
  return ConstantArray::get(StrTy, Elts);
}

Constant *TreeConstantToLLVM::ConvertCOMPLEX_CST(tree exp) {
  std::vector<Constant*> Elts;
  Elts.push_back(Convert(TREE_REALPART(exp)));
  Elts.push_back(Convert(TREE_IMAGPART(exp)));
  return ConstantStruct::get(Elts, false);
}

Constant *TreeConstantToLLVM::ConvertNOP_EXPR(tree exp) {
  Constant *Elt = Convert(TREE_OPERAND(exp, 0));
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  bool EltIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 0)));
  bool TyIsSigned = !TYPE_UNSIGNED(TREE_TYPE(exp));
  
  // If this is a structure-to-structure cast, just return the uncasted value.
  if (!Elt->getType()->isFirstClassType() || !Ty->isFirstClassType())
    return Elt;
  
  // Elt and Ty can be integer, float or pointer here: need generalized cast
  Instruction::CastOps opcode = CastInst::getCastOpcode(Elt, EltIsSigned,
                                                        Ty, TyIsSigned);
  return ConstantExpr::getCast(opcode, Elt, Ty);
}

Constant *TreeConstantToLLVM::ConvertCONVERT_EXPR(tree exp) {
  Constant *Elt = Convert(TREE_OPERAND(exp, 0));
  bool EltIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp, 0)));
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  bool TyIsSigned = !TYPE_UNSIGNED(TREE_TYPE(exp));
  Instruction::CastOps opcode = CastInst::getCastOpcode(Elt, EltIsSigned, Ty, 
                                                        TyIsSigned); 
  return ConstantExpr::getCast(opcode, Elt, Ty);
}

Constant *TreeConstantToLLVM::ConvertBinOp_CST(tree exp) {
  Constant *LHS = Convert(TREE_OPERAND(exp, 0));
  bool LHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp,0)));
  Constant *RHS = Convert(TREE_OPERAND(exp, 1));
  bool RHSIsSigned = !TYPE_UNSIGNED(TREE_TYPE(TREE_OPERAND(exp,1)));
  Instruction::CastOps opcode;
  if (isa<PointerType>(LHS->getType())) {
    const Type *IntPtrTy = getTargetData().getIntPtrType();
    opcode = CastInst::getCastOpcode(LHS, LHSIsSigned, IntPtrTy, false);
    LHS = ConstantExpr::getCast(opcode, LHS, IntPtrTy);
    opcode = CastInst::getCastOpcode(RHS, RHSIsSigned, IntPtrTy, false);
    RHS = ConstantExpr::getCast(opcode, RHS, IntPtrTy);
  }

  Constant *Result;
  switch (TREE_CODE(exp)) {
  default: assert(0 && "Unexpected case!");
  case PLUS_EXPR:   Result = ConstantExpr::getAdd(LHS, RHS); break;
  case MINUS_EXPR:  Result = ConstantExpr::getSub(LHS, RHS); break;
  }
  
  const Type *Ty = ConvertType(TREE_TYPE(exp));
  bool TyIsSigned = !TYPE_UNSIGNED(TREE_TYPE(exp));
  opcode = CastInst::getCastOpcode(Result, LHSIsSigned, Ty, TyIsSigned);
  return ConstantExpr::getCast(opcode, Result, Ty);
}

Constant *TreeConstantToLLVM::ConvertCONSTRUCTOR(tree exp) {
  if (CONSTRUCTOR_ELTS(exp) == 0)  // All zeros?
    return Constant::getNullValue(ConvertType(TREE_TYPE(exp)));

  switch (TREE_CODE(TREE_TYPE(exp))) {
  default: 
    debug_tree(exp);
    assert(0 && "Unknown ctor!");
  case VECTOR_TYPE:
  case ARRAY_TYPE:  return ConvertArrayCONSTRUCTOR(exp);
  case RECORD_TYPE: return ConvertRecordCONSTRUCTOR(exp);
  case UNION_TYPE:  return ConvertUnionCONSTRUCTOR(exp);
  }
}

Constant *TreeConstantToLLVM::ConvertArrayCONSTRUCTOR(tree exp) {
  // Vectors are like arrays, but the domain is stored via an array
  // type indirectly.
  assert(TREE_CODE(TREE_TYPE(exp)) != VECTOR_TYPE &&
         "VECTOR_TYPE's haven't been tested!");

  // If we have a lower bound for the range of the type, get it.  */
  tree Domain = TYPE_DOMAIN(TREE_TYPE(exp));
  tree min_element = size_zero_node;
  if (Domain && TYPE_MIN_VALUE(Domain))
    min_element = fold_convert(sizetype, TYPE_MIN_VALUE(Domain));

  std::vector<Constant*> ResultElts;
  Constant *SomeVal = 0;
  
  if (Domain && TYPE_MAX_VALUE(Domain)) {
    tree max_element = fold_convert(sizetype, TYPE_MAX_VALUE(Domain));
    tree size = size_binop (MINUS_EXPR, max_element, min_element);
    size = size_binop (PLUS_EXPR, size, size_one_node);

    if (host_integerp(size, 1))
      ResultElts.resize(tree_low_cst(size, 1));
  }

  unsigned NextFieldToFill = 0;
  for (tree elt = CONSTRUCTOR_ELTS(exp); elt; elt = TREE_CHAIN(elt)) {
    // Find and decode the constructor's value.
    Constant *Val = Convert(TREE_VALUE(elt));
    SomeVal = Val;
    
    // Get the index position of the element within the array.  Note that this
    // can be NULL_TREE, which means that it belongs in the next available slot.
    tree index = TREE_PURPOSE(elt);
    
    // The first and last field to fill in, inclusive.
    unsigned FieldOffset, FieldLastOffset;
    if (index && TREE_CODE(index) == RANGE_EXPR) {
      tree first = fold_convert (sizetype, TREE_OPERAND(index, 0));
      tree last  = fold_convert (sizetype, TREE_OPERAND(index, 1));

      first = size_binop (MINUS_EXPR, first, min_element);
      last  = size_binop (MINUS_EXPR, last, min_element);

      assert(host_integerp(first, 1) && host_integerp(last, 1) &&
             "Unknown range_expr!");
      FieldOffset     = tree_low_cst(first, 1);
      FieldLastOffset = tree_low_cst(last, 1);
    } else if (index) {
      index = size_binop (MINUS_EXPR, fold_convert (sizetype, index),
                          min_element);
      assert(host_integerp(index, 1));
      FieldOffset = tree_low_cst(index, 1);
      FieldLastOffset = FieldOffset;
    } else {
      FieldOffset = NextFieldToFill;
      FieldLastOffset = FieldOffset;
    }
    
    // Process all of the elements in the range.
    for (--FieldOffset; FieldOffset != FieldLastOffset; ) {
      ++FieldOffset;
      if (FieldOffset == ResultElts.size())
        ResultElts.push_back(Val);
      else {
        if (FieldOffset >= ResultElts.size())
          ResultElts.resize(FieldOffset+1);
        ResultElts[FieldOffset] = Val;
      }
      
      NextFieldToFill = FieldOffset+1;
    }
  }
  
  // Zero length array.
  if (ResultElts.empty())
    return ConstantArray::get(cast<ArrayType>(ConvertType(TREE_TYPE(exp))),
                              ResultElts);
  assert(SomeVal && "If we had some initializer, we should have some value!");
  
  // Do a post-pass over all of the elements.  We're taking care of two things
  // here:
  //   #1. If any elements did not have initializers specified, provide them
  //       with a null init.
  //   #2. If any of the elements have different types, return a struct instead
  //       of an array.  This can occur in cases where we have an array of
  //       unions, and the various unions had different pieces init'd.
  const Type *ElTy = SomeVal->getType();
  Constant *Filler = Constant::getNullValue(ElTy);
  bool AllEltsSameType = true;
  for (unsigned i = 0, e = ResultElts.size(); i != e; ++i) {
    if (ResultElts[i] == 0)
      ResultElts[i] = Filler;
    else if (ResultElts[i]->getType() != ElTy)
      AllEltsSameType = false;
  }
  
  if (AllEltsSameType)
    return ConstantArray::get(ArrayType::get(ElTy, ResultElts.size()),
                              ResultElts);
  return ConstantStruct::get(ResultElts, false);
}

/// InsertBitFieldValue - Process the assignment of a bitfield value into an
/// LLVM struct value.  Val may be null if nothing has been assigned into this
/// field, so FieldTy indicates the type of the LLVM struct type to use.
static Constant *InsertBitFieldValue(uint64_t ValToInsert,
                                     unsigned NumBitsToInsert,
                                     unsigned OffsetToBitFieldStart,
                                     unsigned FieldBitSize,
                                     Constant *Val, const Type *FieldTy) {
  if (FieldTy->isInteger()) {
    uint64_t ExistingVal;
    if (Val == 0)
      ExistingVal = 0;
    else {
      assert(isa<ConstantInt>(Val) && "Bitfield shared with non-bit-field?");
      ExistingVal = cast<ConstantInt>(Val)->getZExtValue();
    }
    
    // Compute the value to insert, and the mask to use.
    uint64_t FieldMask;
    if (BITS_BIG_ENDIAN) {
      FieldMask = ~0ULL >> (64-NumBitsToInsert);
      FieldMask   <<= FieldBitSize-(OffsetToBitFieldStart+NumBitsToInsert);
      ValToInsert <<= FieldBitSize-(OffsetToBitFieldStart+NumBitsToInsert);
    } else {
      FieldMask = ~0ULL >> (64-NumBitsToInsert);
      FieldMask   <<= OffsetToBitFieldStart;
      ValToInsert <<= OffsetToBitFieldStart;
    }

    // Insert the new value into the field and return it.
    uint64_t NewVal = (ExistingVal & ~FieldMask) | ValToInsert;
    return ConstantExpr::getTruncOrBitCast(ConstantInt::get(Type::Int64Ty, 
                                                            NewVal), FieldTy);
  } else {
    // Otherwise, this is initializing part of an array of bytes.  Recursively
    // insert each byte.
    assert(isa<ArrayType>(FieldTy) && 
           cast<ArrayType>(FieldTy)->getElementType() == Type::Int8Ty &&
           "Not an array of bytes?");
    // Expand the already parsed initializer into its elements if it is a 
    // ConstantArray or ConstantAggregateZero.
    std::vector<Constant*> Elts;
    Elts.resize(cast<ArrayType>(FieldTy)->getNumElements());
    
    if (Val) {
      if (ConstantArray *CA = dyn_cast<ConstantArray>(Val)) {
        for (unsigned i = 0, e = Elts.size(); i != e; ++i)
          Elts[i] = CA->getOperand(i);
      } else {
        assert(isa<ConstantAggregateZero>(Val) && "Unexpected initializer!");
        Constant *Elt = Constant::getNullValue(Type::Int8Ty);
        for (unsigned i = 0, e = Elts.size(); i != e; ++i)
          Elts[i] = Elt;
      }
    }
    
    // Loop over all of our elements, inserting pieces into each one as
    // appropriate.
    unsigned i = OffsetToBitFieldStart/8;  // Skip to first byte
    OffsetToBitFieldStart &= 7;
    
    for (; NumBitsToInsert; ++i) {
      assert(i < Elts.size() && "Inserting out of range!");
      
      unsigned NumEltBitsToInsert = std::min(8-OffsetToBitFieldStart,
                                             NumBitsToInsert);
      
      uint64_t EltValToInsert;
      if (BITS_BIG_ENDIAN) {
        // If this is a big-endian bit-field, take the top NumBitsToInsert
        // bits from the bitfield value.
        EltValToInsert = ValToInsert >> (NumBitsToInsert-NumEltBitsToInsert);
        
        // Clear the handled bits from BitfieldVal.
        ValToInsert &= (1ULL << (NumBitsToInsert-NumEltBitsToInsert))-1;
      } else {
        // If this is little-endian bit-field, take the bottom NumBitsToInsert
        // bits from the bitfield value.
        EltValToInsert = ValToInsert & (1ULL << NumEltBitsToInsert)-1;
        ValToInsert >>= NumEltBitsToInsert;
      }

      Elts[i] = InsertBitFieldValue(EltValToInsert, NumEltBitsToInsert,
                                    OffsetToBitFieldStart, 8, Elts[i], 
                                    Type::Int8Ty);
      
      // Advance for next element.
      OffsetToBitFieldStart = 0;
      NumBitsToInsert -= NumEltBitsToInsert;
    }
    
    return ConstantArray::get(cast<ArrayType>(FieldTy), Elts);
  }
}


/// ProcessBitFieldInitialization - Handle a static initialization of a 
/// RECORD_TYPE field that is a bitfield.
static void ProcessBitFieldInitialization(tree Field, Value *Val,
                                          const StructType *STy,
                                          std::vector<Constant*> &ResultElts) {
  // Get the offset and size of the bitfield, in bits.
  unsigned BitfieldBitOffset = getFieldOffsetInBits(Field);
  unsigned BitfieldSize      = TREE_INT_CST_LOW(DECL_SIZE(Field));
  
  // Get the value to insert into the bitfield.
  assert(Val->getType()->isInteger() && "Bitfield initializer isn't int!");
  assert(isa<ConstantInt>(Val) && "Non-constant bitfield initializer!");
  uint64_t BitfieldVal = cast<ConstantInt>(Val)->getZExtValue();
  
  // Ensure that the top bits (which don't go into the bitfield) are zero.
  BitfieldVal &= ~0ULL >> (64-BitfieldSize);
  
  // Get the struct field layout info for this struct.
  const StructLayout *STyLayout = getTargetData().getStructLayout(STy);
  
  // If this is a bitfield, we know that FieldNo is the *first* LLVM field
  // that contains bits from the bitfield overlayed with the declared type of
  // the bitfield.  This bitfield value may be spread across multiple fields, or
  // it may be just this field, or it may just be a small part of this field.
  unsigned FieldNo = cast<ConstantInt>(DECL_LLVM(Field))->getZExtValue();
  assert(FieldNo < ResultElts.size() && "Invalid struct field number!");
  
  // Get the offset and size of the LLVM field.
  uint64_t STyFieldBitOffs = STyLayout->getElementOffset(FieldNo)*8;
  
  assert(BitfieldBitOffset >= STyFieldBitOffs &&
         "This bitfield doesn't start in this LLVM field!");
  unsigned OffsetToBitFieldStart = BitfieldBitOffset-STyFieldBitOffs;
  
  // Loop over all of the fields this bitfield is part of.  This is usually just
  // one, but can be several in some cases.
  for (; BitfieldSize; ++FieldNo) {
    assert(STyFieldBitOffs == STyLayout->getElementOffset(FieldNo)*8 &&
           "Bitfield LLVM fields are not exactly consecutive in memory!");
    
    // Compute overlap of this bitfield with this LLVM field, then call a
    // function to insert (the STy element may be an array of bytes or
    // something).
    const Type *STyFieldTy = STy->getElementType(FieldNo);
    unsigned STyFieldBitSize = getTargetData().getTypeSize(STyFieldTy)*8;
    
    // If the bitfield starts after this field, advance to the next field.  This
    // can happen because we start looking at the first element overlapped by
    // an aligned instance of the declared type. 
    if (OffsetToBitFieldStart >= STyFieldBitSize) {
      OffsetToBitFieldStart -= STyFieldBitSize;
      STyFieldBitOffs       += STyFieldBitSize;
      continue;
    }
    
    // We are inserting part of 'Val' into this LLVM field.  The start bit
    // is OffsetToBitFieldStart.  Compute the number of bits from this bitfield
    // we are inserting into this LLVM field.
    unsigned NumBitsToInsert =
      std::min(BitfieldSize, STyFieldBitSize-OffsetToBitFieldStart);
    
    // Compute the NumBitsToInsert-wide value that we are going to insert
    // into this field as an ulong integer constant value.
    uint64_t ValToInsert;
    if (BITS_BIG_ENDIAN) {
      // If this is a big-endian bit-field, take the top NumBitsToInsert
      // bits from the bitfield value.
      ValToInsert = BitfieldVal >> (BitfieldSize-NumBitsToInsert);
      
      // Clear the handled bits from BitfieldVal.
      BitfieldVal &= (1ULL << (BitfieldSize-NumBitsToInsert))-1;
    } else {
      // If this is little-endian bit-field, take the bottom NumBitsToInsert
      // bits from the bitfield value.
      ValToInsert = BitfieldVal & (1ULL << NumBitsToInsert)-1;
      BitfieldVal >>= NumBitsToInsert;
    }
    
    ResultElts[FieldNo] = InsertBitFieldValue(ValToInsert, NumBitsToInsert,
                                              OffsetToBitFieldStart,
                                              STyFieldBitSize,
                                              ResultElts[FieldNo], STyFieldTy);
    
    // If this bitfield splits across multiple LLVM fields, update these
    // values for the next field.
    BitfieldSize          -= NumBitsToInsert;
    STyFieldBitOffs       += STyFieldBitSize;
    OffsetToBitFieldStart = 0;
  }
}

/// ConvertStructFieldInitializerToType - Convert the input value to a new type,
/// when constructing the field initializers for a constant CONSTRUCTOR object.
static Constant *ConvertStructFieldInitializerToType(Constant *Val, 
                                                     const Type *FieldTy) {
  const TargetData &TD = getTargetData();
  assert(TD.getTypeSize(FieldTy) == TD.getTypeSize(Val->getType()) &&
         "Mismatched initializer type isn't same size as initializer!");

  // If this is an integer initializer for an array of ubytes, we are
  // initializing an unaligned integer field.  Break the integer initializer up
  // into pieces.
  if (ConstantInt *CI = dyn_cast<ConstantInt>(Val)) {
    if (const ArrayType *ATy = dyn_cast<ArrayType>(FieldTy))
      if (ATy->getElementType() == Type::Int8Ty) {
        std::vector<Constant*> ArrayElts;
        uint64_t Val = CI->getZExtValue();
        for (unsigned i = 0, e = ATy->getNumElements(); i != e; ++i) {
          unsigned char EltVal;
         
          if (TD.isLittleEndian()) {
            EltVal = (Val >> 8*i) & 0xFF;
          } else {
            EltVal = (Val >> 8*(e-i-1)) & 0xFF;
          }
          
          ArrayElts.push_back(ConstantInt::get(Type::Int8Ty, EltVal));
        }

        return ConstantArray::get(ATy, ArrayElts);
      }
  }
  
  // Otherwise, we can get away with this initialization.
  assert(TD.getABITypeAlignment(FieldTy) >= 
         TD.getABITypeAlignment(Val->getType()) &&
         "Field initialize is over aligned for LLVM type!");
  return Val;
}


Constant *TreeConstantToLLVM::ConvertRecordCONSTRUCTOR(tree exp) {
  const StructType *STy = cast<StructType>(ConvertType(TREE_TYPE(exp)));

  std::vector<Constant*> ResultElts;
  ResultElts.resize(STy->getNumElements());

  tree NextField = TYPE_FIELDS(TREE_TYPE(exp));
  for (tree elt = CONSTRUCTOR_ELTS(exp); elt; elt = TREE_CHAIN(elt)) {
    tree Field = TREE_PURPOSE(elt);    // The fielddecl for the field.
    unsigned FieldNo;
    if (Field == 0) {           // If an explicit field is specified, use it.
      Field = NextField;
      // Advance to the next FIELD_DECL, skipping over other structure members
      // (e.g. enums).
      for (; 1; Field = TREE_CHAIN(Field)) {
        assert(Field && "Fell off end of record!");
        if (TREE_CODE(Field) == FIELD_DECL) break;
      }
    }

    // Decode the field's value.
    Constant *Val = Convert(TREE_VALUE(elt));

    // If the field is a bitfield, it could be spread across multiple fields and
    // may start at some bit offset.
    if (DECL_BIT_FIELD_TYPE(Field)) {
      ProcessBitFieldInitialization(Field, Val, STy, ResultElts);
    } else {
      // If not, things are much simpler.
      assert(DECL_LLVM_SET_P(Field) && "Struct not laid out for LLVM?");
      unsigned FieldNo = cast<ConstantInt>(DECL_LLVM(Field))->getZExtValue();
      assert(FieldNo < ResultElts.size() && "Invalid struct field number!");

      // Example: struct X { int A; char C[]; } x = { 4, "foo" };
      assert(TYPE_SIZE(TREE_TYPE(Field)) ||
             (FieldNo == ResultElts.size()-1 &&
              isStructWithVarSizeArrayAtEnd(STy))
             && "field with no size is not array at end of struct!");

      // If this is an initialization of a global that ends with a variable
      // sized array at its end, and the initializer has a non-zero number of
      // elements, then Val contains the actual type for the array.  Otherwise,
      // we know that the initializer has to match the element type of the LLVM
      // structure field.  If not, then there is something that is not
      // straight-forward going on.  For example, we could be initializing an
      // unaligned integer field (e.g. due to attribute packed) with an
      // integer.  The struct field will have type [4 x ubyte] instead of
      // "int" for example.  If we ignored this, we would lay out the
      // initializer wrong.
      if (TYPE_SIZE(TREE_TYPE(Field)) &&
          Val->getType() != STy->getElementType(FieldNo))
        Val = ConvertStructFieldInitializerToType(Val,
                                                  STy->getElementType(FieldNo));

      ResultElts[FieldNo] = Val;
    }

    NextField = TREE_CHAIN(Field);
  }
  
  // Fill in null elements with zeros.
  for (unsigned i = 0, e = ResultElts.size(); i != e; ++i)
    if (ResultElts[i] == 0)
      ResultElts[i] = Constant::getNullValue(STy->getElementType(i));
  
  return ConstantStruct::get(ResultElts, false);
}

Constant *TreeConstantToLLVM::ConvertUnionCONSTRUCTOR(tree exp) {
  assert(CONSTRUCTOR_ELTS(exp) && "Union CONSTRUCTOR has no elements? Zero?");
  
  tree elt = CONSTRUCTOR_ELTS(exp);
  assert(TREE_CHAIN(elt) == 0 && "Union CONSTRUCTOR with multiple elements?");

  std::vector<Constant*> Elts;
  // Convert the constant itself.
  Elts.push_back(Convert(TREE_VALUE(elt)));

  // If the union has a fixed size, and if the value we converted isn't large
  // enough to fill all the bits, add a zero initialized array at the end to pad
  // it out.
  tree UnionType = TREE_TYPE(exp);
  if (TYPE_SIZE(UnionType) && TREE_CODE(TYPE_SIZE(UnionType)) == INTEGER_CST) {
    unsigned UnionSize = ((unsigned)TREE_INT_CST_LOW(TYPE_SIZE(UnionType))+7)/8;
    unsigned InitSize = getTargetData().getTypeSize(Elts[0]->getType());
    if (UnionSize != InitSize) {
      const Type *FillTy;
      assert(UnionSize > InitSize && "Init shouldn't be larger than union!");
      if (UnionSize - InitSize == 1)
        FillTy = Type::Int8Ty;
      else
        FillTy = ArrayType::get(Type::Int8Ty, UnionSize - InitSize);
      Elts.push_back(Constant::getNullValue(FillTy));
    }
  }
  return ConstantStruct::get(Elts, false);
}

//===----------------------------------------------------------------------===//
//                  ... Constant Expressions L-Values ...
//===----------------------------------------------------------------------===//

Constant *TreeConstantToLLVM::EmitLV(tree exp) {
  switch (TREE_CODE(exp)) {
  default: 
    debug_tree(exp); 
    assert(0 && "Unknown constant lvalue to convert!");
    abort();
  case FUNCTION_DECL:
  case CONST_DECL:
  case VAR_DECL:      return EmitLV_Decl(exp);
  case LABEL_DECL:    return EmitLV_LABEL_DECL(exp);
  case STRING_CST:    return EmitLV_STRING_CST(exp);
  case COMPONENT_REF: return EmitLV_COMPONENT_REF(exp);
  case ARRAY_RANGE_REF:
  case ARRAY_REF:     return EmitLV_ARRAY_REF(exp);
  case INDIRECT_REF:
    // The lvalue is just the address.
    return Convert(TREE_OPERAND(exp, 0));
  case COMPOUND_LITERAL_EXPR: // FIXME: not gimple - defined by C front-end
    return EmitLV(COMPOUND_LITERAL_EXPR_DECL(exp));
  }
}

Constant *TreeConstantToLLVM::EmitLV_Decl(tree exp) {
  GlobalValue *Val = cast<GlobalValue>(DECL_LLVM(exp));

  // Ensure variable marked as used even if it doesn't go through a parser.  If
  // it hasn't been used yet, write out an external definition.
  if (!TREE_USED(exp)) {
    assemble_external(exp);
    TREE_USED(exp) = 1;
    Val = cast<GlobalValue>(DECL_LLVM(exp));
  }
  
  // If this is an aggregate CONST_DECL, emit it to LLVM now.  GCC happens to
  // get this case right by forcing the initializer into memory.
  if (TREE_CODE(exp) == CONST_DECL) {
    if (DECL_INITIAL(exp) && Val->isDeclaration()) {
      emit_global_to_llvm(exp);
      // Decl could have change if it changed type.
      Val = cast<GlobalValue>(DECL_LLVM(exp));
    }
  } else {
    // Otherwise, inform cgraph that we used the global.
    mark_decl_referenced(exp);
    if (tree ID = DECL_ASSEMBLER_NAME(exp))
      mark_referenced(ID);
  }
  
  return Val;
}

/// EmitLV_LABEL_DECL - Someone took the address of a label.
Constant *TreeConstantToLLVM::EmitLV_LABEL_DECL(tree exp) {
  assert(TheTreeToLLVM &&
         "taking the address of a label while not compiling the function!");
    
  // Figure out which function this is for, verify it's the one we're compiling.
  if (DECL_CONTEXT(exp)) {
    assert(TREE_CODE(DECL_CONTEXT(exp)) == FUNCTION_DECL &&
           "Address of label in nested function?");
    assert(TheTreeToLLVM->getFUNCTION_DECL() == DECL_CONTEXT(exp) &&
           "Taking the address of a label that isn't in the current fn!?");
  }
  
  BasicBlock *BB = getLabelDeclBlock(exp);
  Constant *C = TheTreeToLLVM->getIndirectGotoBlockNumber(BB);
  return ConstantExpr::getIntToPtr(C, PointerType::get(Type::Int8Ty));
}

Constant *TreeConstantToLLVM::EmitLV_STRING_CST(tree exp) {
  Constant *Init = TreeConstantToLLVM::ConvertSTRING_CST(exp);

  // Support -fwritable-strings.
  bool StringIsConstant = !flag_writable_strings;

  GlobalVariable **SlotP = 0;

  if (StringIsConstant) {
    // Cache the string constants to avoid making obvious duplicate strings that
    // have to be folded by the optimizer.
    static std::map<Constant*, GlobalVariable*> StringCSTCache;
    GlobalVariable *&Slot = StringCSTCache[Init];
    if (Slot) return Slot;
    SlotP = &Slot;
  }
    
  // Create a new string global.
  GlobalVariable *GV = new GlobalVariable(Init->getType(), StringIsConstant,
                                          GlobalVariable::InternalLinkage,
                                          Init, "str", TheModule);
  if (SlotP) *SlotP = GV;
  return GV;
}

Constant *TreeConstantToLLVM::EmitLV_ARRAY_REF(tree exp) {
  tree Array = TREE_OPERAND(exp, 0);
  tree Index = TREE_OPERAND(exp, 1);
  assert((TREE_CODE (TREE_TYPE(Array)) == ARRAY_TYPE ||
          TREE_CODE (TREE_TYPE(Array)) == POINTER_TYPE ||
          TREE_CODE (TREE_TYPE(Array)) == REFERENCE_TYPE) &&
         "Unknown ARRAY_REF!");
  
  // As an LLVM extension, we allow ARRAY_REF with a pointer as the first
  // operand.  This construct maps directly to a getelementptr instruction.
  Constant *ArrayAddr;
  if (TREE_CODE (TREE_TYPE(Array)) == ARRAY_TYPE) {
    // First subtract the lower bound, if any, in the type of the index.
    tree LowerBound = array_ref_low_bound(exp);
    if (!integer_zerop(LowerBound))
      Index = fold(build2(MINUS_EXPR, TREE_TYPE(Index), Index, LowerBound));
    ArrayAddr = EmitLV(Array);
  } else {
    ArrayAddr = Convert(Array);
  }
  
  Constant *IndexVal = Convert(Index);
  
  // FIXME: If UnitSize is a variable, or if it disagrees with the LLVM array
  // element type, insert explicit pointer arithmetic here.
  //tree ElementSizeInBytes = array_ref_element_size(exp);
  
  if (IndexVal->getType() != Type::Int32Ty &&
      IndexVal->getType() != Type::Int64Ty)
    IndexVal = ConstantExpr::getSExtOrBitCast(IndexVal, Type::Int64Ty);
  
  // Check for variable sized array reference.
  if (TREE_CODE(TREE_TYPE(Array)) == ARRAY_TYPE) {
    tree length = arrayLength(TREE_TYPE(Array));
    assert(!length || host_integerp(length, 1) &&
           "Cannot have globals with variable size!");
  }

  std::vector<Value*> Idx;
  if (TREE_CODE (TREE_TYPE(Array)) == ARRAY_TYPE)
    Idx.push_back(ConstantInt::get(Type::Int32Ty, 0));
  Idx.push_back(IndexVal);
  return ConstantExpr::getGetElementPtr(ArrayAddr, &Idx[0], Idx.size());
}

Constant *TreeConstantToLLVM::EmitLV_COMPONENT_REF(tree exp) {
  Constant *StructAddrLV = EmitLV(TREE_OPERAND(exp, 0));
  
  // Ensure that the struct type has been converted, so that the fielddecls
  // are laid out.
  const Type *StructTy = ConvertType(TREE_TYPE(TREE_OPERAND(exp, 0)));
  
  tree FieldDecl = TREE_OPERAND(exp, 1);
  
  StructAddrLV = ConstantExpr::getBitCast(StructAddrLV,
                                          PointerType::get(StructTy));
  const Type *FieldTy = ConvertType(TREE_TYPE(FieldDecl));
  
  // BitStart - This is the actual offset of the field from the start of the
  // struct, in bits.  For bitfields this may be on a non-byte boundary.
  unsigned BitStart = getComponentRefOffsetInBits(exp);
  unsigned BitSize  = 0;
  Constant *FieldPtr;
  const TargetData &TD = getTargetData();

  tree field_offset = component_ref_field_offset (exp);
  // If this is a normal field at a fixed offset from the start, handle it.
  if (TREE_CODE(field_offset) == INTEGER_CST) {
    assert(DECL_LLVM_SET_P(FieldDecl) && "Struct not laid out for LLVM?");
    ConstantInt *CI = cast<ConstantInt>(DECL_LLVM(FieldDecl));
    uint64_t MemberIndex = CI->getZExtValue();
    
    std::vector<Value*> Idxs;
    Idxs.push_back(Constant::getNullValue(Type::Int32Ty));
    Idxs.push_back(CI);
    FieldPtr = ConstantExpr::getGetElementPtr(StructAddrLV, &Idxs[0],
                                              Idxs.size());
    
    // Now that we did an offset from the start of the struct, subtract off
    // the offset from BitStart.
    if (MemberIndex) {
      const StructLayout *SL = TD.getStructLayout(cast<StructType>(StructTy));
      BitStart -= SL->getElementOffset(MemberIndex) * 8;
    }
  } else {
    Constant *Offset = Convert(field_offset);
    Constant *Ptr = ConstantExpr::getPtrToInt(StructAddrLV, Offset->getType());
    Ptr = ConstantExpr::getAdd(Ptr, Offset);
    FieldPtr = ConstantExpr::getIntToPtr(Ptr, PointerType::get(FieldTy));
  }
  
  if (DECL_BIT_FIELD_TYPE(FieldDecl)) {
    FieldTy = ConvertType(DECL_BIT_FIELD_TYPE(FieldDecl));
    FieldPtr = ConstantExpr::getBitCast(FieldPtr, PointerType::get(FieldTy));
  }
  
  assert(BitStart == 0 &&
         "It's a bitfield reference or we didn't get to the field!");
  return FieldPtr;
}

/* APPLE LOCAL end LLVM (ENTIRE FILE!)  */

