/* APPLE LOCAL begin LLVM (ENTIRE FILE!)  */
/* High-level LLVM backend interface 
Copyright (C) 2005 Free Software Foundation, Inc.
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

#include "llvm-internal.h"
#include "llvm-debug.h"
#include "llvm-file-ostream.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/ModuleProvider.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/Bytecode/WriteBytecodePass.h"
#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/SchedulerRegistry.h"
#include "llvm/Support/Streams.h"
#include "llvm/Target/SubtargetFeature.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetMachineRegistry.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/LoadValueNumbering.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Streams.h"
#include <cassert>
#undef VISIBILITY_HIDDEN
extern "C" {
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "flags.h"
#include "tree.h"
#include "diagnostic.h"
#include "output.h"
#include "toplev.h"
#include "timevar.h"
#include "tm.h"
#include "function.h"
#include "tree-inline.h"
}

// Global state for the LLVM backend.
Module *TheModule = 0;
DebugInfo *TheDebugInfo = 0;
TargetMachine *TheTarget = 0;
TypeConverter *TheTypeConverter = 0;
llvm::OStream *AsmOutFile = 0;

std::vector<std::pair<Function*, int> > StaticCtors, StaticDtors;
std::vector<GlobalValue*> AttributeUsedGlobals;
std::map<std::string, GlobalVariable*> EmittedGlobalVars;
std::map<std::string, Function*> EmittedFunctions;

/// PerFunctionPasses - This is the list of cleanup passes run per-function
/// as each is compiled.  In cases where we are not doing IPO, it includes the 
/// code generator.
static FunctionPassManager *PerFunctionPasses = 0;
static PassManager *PerModulePasses = 0;
static FunctionPassManager *CodeGenPasses = 0;

void llvm_initialize_backend(void) {
  // Initialize LLVM options.
  std::vector<const char*> Args;
  Args.push_back(progname); // program name

  // Allow targets to specify PIC options and other stuff to the corresponding
  // LLVM backends.
#ifdef LLVM_SET_TARGET_OPTIONS
  LLVM_SET_TARGET_OPTIONS(Args);
#endif
  
  if (time_report)
    Args.push_back("--time-passes");
  if (fast_math_flags_set_p())
    Args.push_back("--enable-unsafe-fp-math");
  if (!flag_omit_frame_pointer)
    Args.push_back("--disable-fp-elim");
    
  // If there are options that should be passed through to the LLVM backend
  // directly from the command line, do so now.  This is mainly for debugging
  // purposes, and shouldn't really be for general use.
  std::vector<std::string> ArgStrings;
  if (llvm_optns) {
    std::string Opts = llvm_optns;
    for (std::string Opt = getToken(Opts); !Opt.empty(); Opt = getToken(Opts))
      ArgStrings.push_back(Opt);
  }
  for (unsigned i = 0, e = ArgStrings.size(); i != e; ++i)
    Args.push_back(ArgStrings[i].c_str());
  Args.push_back(0);  // Null terminator.
  
  int pseudo_argc = Args.size()-1;
  cl::ParseCommandLineOptions(pseudo_argc, (char**)&Args[0]);

  TheModule = new Module("");

  // If the target wants to override the architecture, e.g. turning
  // powerpc-darwin-... into powerpc64-darwin-... when -m64 is enabled, do so
  // now.
  std::string TargetTriple = TARGET_NAME;
#ifdef LLVM_OVERRIDE_TARGET_ARCH
  std::string Arch = LLVM_OVERRIDE_TARGET_ARCH();
  if (!Arch.empty()) {
    std::string::size_type DashPos = TargetTriple.find('-');
    if (DashPos != std::string::npos)// If we have a sane t-t, replace the arch.
      TargetTriple = Arch + TargetTriple.substr(DashPos);
  }
#endif
  TheModule->setTargetTriple(TargetTriple);
  
  TheModule->setEndianness(BITS_BIG_ENDIAN ? Module::BigEndian
                                           : Module::LittleEndian);
  TheModule->setPointerSize(POINTER_SIZE == 32 ? Module::Pointer32
                                               : Module::Pointer64);
  TheTypeConverter = new TypeConverter();
  
  // Create the TargetMachine we will be generating code with.
  // FIXME: Figure out how to select the target and pass down subtarget info.
  std::string Err;
  const TargetMachineRegistry::Entry *TME = 
    TargetMachineRegistry::getClosestStaticTargetForModule(*TheModule, Err);
  if (!TME) {
    cerr << "Did not get a target machine!\n";
    exit(1);
  }
  
  // Figure out the subtarget feature string we pass to the target.
  std::string FeatureStr;
  // The target can set LLVM_SET_SUBTARGET_FEATURES to configure the LLVM
  // backend.
#ifdef LLVM_SET_SUBTARGET_FEATURES
  SubtargetFeatures Features;
  LLVM_SET_SUBTARGET_FEATURES(Features);
  FeatureStr = Features.getString();
#endif
  TheTarget = TME->CtorFn(*TheModule, FeatureStr);

  if (optimize) {
    RegisterScheduler::setDefault(createDefaultScheduler);
  } else {
    RegisterScheduler::setDefault(createBFS_DAGScheduler);
  }
  
  RegisterRegAlloc::setDefault(createLinearScanRegisterAllocator);
 
  if (!optimize && debug_info_level > DINFO_LEVEL_NONE)
    TheDebugInfo = new DebugInfo(TheModule);
}

void llvm_lang_dependent_init(const char *Name) {
  if (Name)
    TheModule->setModuleIdentifier(Name);
}

oFILEstream *AsmOutStream = 0;

// llvm_asm_file_start - Start the .s file.
void llvm_asm_file_start(void) {
  timevar_push(TV_LLVM_INIT);
  AsmOutStream = new oFILEstream(asm_out_file);
  AsmOutFile = new OStream(*AsmOutStream);
  
  // Create and set up the per-function pass manager.
  // FIXME: Move the code generator to be function-at-a-time.
  PerFunctionPasses =
    new FunctionPassManager(new ExistingModuleProvider(TheModule));
  PerFunctionPasses->add(new TargetData(*TheTarget->getTargetData()));

  // In -O0 if checking is disabled, we don't even have per-function passes.
  bool HasPerFunctionPasses = false;
#ifdef ENABLE_CHECKING
  PerFunctionPasses->add(createVerifierPass());
  HasPerFunctionPasses = true;
#endif

  if (optimize > 0) {
    HasPerFunctionPasses = true;
    PerFunctionPasses->add(createCFGSimplificationPass());
    if (optimize == 1)
      PerFunctionPasses->add(createPromoteMemoryToRegisterPass());
    else
      PerFunctionPasses->add(createScalarReplAggregatesPass());
    PerFunctionPasses->add(createInstructionCombiningPass());
    //    PerFunctionPasses->add(createCFGSimplificationPass());
  }

  // FIXME: AT -O0/O1, we should stream out functions at a time.
  PerModulePasses = new PassManager();
  PerModulePasses->add(new TargetData(*TheTarget->getTargetData()));
  bool HasPerModulePasses = false;

  if (optimize > 0) {
    HasPerModulePasses = true;
    PassManager *PM = PerModulePasses;
    if (flag_unit_at_a_time)
      PM->add(createRaiseAllocationsPass());     // call %malloc -> malloc inst
    PM->add(createCFGSimplificationPass());    // Clean up disgusting code
    PM->add(createPromoteMemoryToRegisterPass());// Kill useless allocas
    if (flag_unit_at_a_time) {
      PM->add(createGlobalOptimizerPass());      // Optimize out global vars
      PM->add(createGlobalDCEPass());            // Remove unused fns and globs
      PM->add(createIPConstantPropagationPass());// IP Constant Propagation
      PM->add(createDeadArgEliminationPass());   // Dead argument elimination
    }
    PM->add(createInstructionCombiningPass()); // Clean up after IPCP & DAE
    // DISABLE PREDSIMPLIFY UNTIL PR967 is fixed.
    //PM->add(createPredicateSimplifierPass());  // Canonicalize registers
    PM->add(createCFGSimplificationPass());    // Clean up after IPCP & DAE
    if (flag_unit_at_a_time)
      PM->add(createPruneEHPass());              // Remove dead EH info

    if (optimize > 1) {
      if (flag_inline_trees > 1)   // respect -fno-inline-functions
        PM->add(createFunctionInliningPass());   // Inline small functions
      if (flag_unit_at_a_time)
        PM->add(createSimplifyLibCallsPass());   // Library Call Optimizations

      if (optimize > 2)
    	PM->add(createArgumentPromotionPass());  // Scalarize uninlined fn args
      
      PM->add(createRaisePointerReferencesPass());// Recover type information
    }
    
    PM->add(createTailDuplicationPass());      // Simplify cfg by copying code
    PM->add(createCFGSimplificationPass());    // Merge & remove BBs
    PM->add(createScalarReplAggregatesPass()); // Break up aggregate allocas
    PM->add(createInstructionCombiningPass()); // Combine silly seq's
    PM->add(createCondPropagationPass());      // Propagate conditionals
    PM->add(createTailCallEliminationPass());  // Eliminate tail calls
    PM->add(createCFGSimplificationPass());    // Merge & remove BBs
    PM->add(createReassociatePass());          // Reassociate expressions
    PM->add(createLICMPass());                 // Hoist loop invariants
    PM->add(createLoopUnswitchPass());         // Unswitch loops.
    PM->add(createInstructionCombiningPass()); // Clean up after LICM/reassoc
    PM->add(createIndVarSimplifyPass());       // Canonicalize indvars
    if (flag_unroll_loops)
      PM->add(createLoopUnrollPass());           // Unroll small loops
    PM->add(createInstructionCombiningPass()); // Clean up after the unroller
    PM->add(createLoadValueNumberingPass());   // GVN for load instructions
    PM->add(createGCSEPass());                 // Remove common subexprs
    PM->add(createSCCPPass());                 // Constant prop with SCCP
    
    // Run instcombine after redundancy elimination to exploit opportunities
    // opened up by them.
    PM->add(createInstructionCombiningPass());
    PM->add(createCondPropagationPass());      // Propagate conditionals
    PM->add(createDeadStoreEliminationPass()); // Delete dead stores
    PM->add(createAggressiveDCEPass());        // SSA based 'Aggressive DCE'
    PM->add(createCFGSimplificationPass());    // Merge & remove BBs
    
    if (optimize > 1 && flag_unit_at_a_time)
      PM->add(createConstantMergePass());        // Merge dup global constants 
  }
  
  if (emit_llvm_bc) {
    // Emit an LLVM .bc file to the output.  This is used when passed
    // -emit-llvm -c to the GCC driver.
    PerModulePasses->add(new WriteBytecodePass(AsmOutFile));

    // Disable emission of .ident into the output file... which is completely
    // wrong for llvm/.bc emission cases.
    flag_no_ident = 1;
    HasPerModulePasses = true;
  } else if (emit_llvm) {
    // Emit an LLVM .ll file to the output.  This is used when passed 
    // -emit-llvm -S to the GCC driver.
    PerModulePasses->add(new PrintModulePass(AsmOutFile));
    
    // Disable emission of .ident into the output file... which is completely
    // wrong for llvm/.bc emission cases.
    flag_no_ident = 1;
    HasPerModulePasses = true;
  } else {
    FunctionPassManager *PM;
    
    // If there are passes we have to run on the entire module, we do codegen
    // as a separate "pass" after that happens.
    // FIXME: This is disabled right now until bugs can be worked out.  Reenable
    // this for fast -O0 compiles!
    if (HasPerModulePasses || 1) {
      CodeGenPasses = PM =
        new FunctionPassManager(new ExistingModuleProvider(TheModule));
      PM->add(new TargetData(*TheTarget->getTargetData()));
    } else {
      // If there are no module-level passes that have to be run, we codegen as
      // each function is parsed.
      PM = PerFunctionPasses;
      HasPerFunctionPasses = true;
    }

    // Normal mode, emit a .s file by running the code generator.
    if (TheTarget->addPassesToEmitFile(*PM, *AsmOutStream, 
                                       TargetMachine::AssemblyFile,
                                       /*FAST*/optimize == 0)) {
      cerr << "Error interfacing to target machine!";
      exit(1);
    }
    
  }
  
  if (HasPerFunctionPasses) {
    PerFunctionPasses->doInitialization();
  } else {
    delete PerFunctionPasses;
    PerFunctionPasses = 0;
  }
  if (!HasPerModulePasses) {
    delete PerModulePasses;
    PerModulePasses = 0;
  }
  
  timevar_pop(TV_LLVM_INIT);
}

/// ConvertStructorsList - Convert a list of static ctors/dtors to an
/// initializer suitable for the llvm.global_[cd]tors globals.
static void CreateStructorsList(std::vector<std::pair<Function*, int> > &Tors,
                                const char *Name) {
  std::vector<Constant*> InitList;
  std::vector<Constant*> StructInit;
  StructInit.resize(2);
  for (unsigned i = 0, e = Tors.size(); i != e; ++i) {
    StructInit[0] = ConstantInt::get(Type::Int32Ty, Tors[i].second);
    StructInit[1] = Tors[i].first;
    InitList.push_back(ConstantStruct::get(StructInit, false));
  }
  Constant *Array =
    ConstantArray::get(ArrayType::get(InitList[0]->getType(), InitList.size()),
                       InitList);
  new GlobalVariable(Array->getType(), false, GlobalValue::AppendingLinkage,
                     Array, Name, TheModule);
}

// llvm_asm_file_end - Finish the .s file.
void llvm_asm_file_end(void) {
  timevar_push(TV_LLVM_PERFILE);
  
  // Add an llvm.global_ctors global if needed.
  if (!StaticCtors.empty())
    CreateStructorsList(StaticCtors, "llvm.global_ctors");
  // Add an llvm.global_dtors global if needed.
  if (!StaticDtors.empty())
    CreateStructorsList(StaticDtors, "llvm.global_dtors");
  
  if (!AttributeUsedGlobals.empty()) {
    std::vector<Constant*> GlobalInit;
    const Type *SBP = PointerType::get(Type::Int8Ty);
    for (unsigned i = 0, e = AttributeUsedGlobals.size(); i != e; ++i)
      GlobalInit.push_back(ConstantExpr::getBitCast(AttributeUsedGlobals[i], 
                                                    SBP));
    ArrayType *AT = ArrayType::get(SBP, AttributeUsedGlobals.size());
    Constant *Init = ConstantArray::get(AT, GlobalInit);
    new GlobalVariable(AT, false, GlobalValue::AppendingLinkage, Init,
                       "llvm.used", TheModule);
  }
  
  // Finish off the per-function pass.
  if (PerFunctionPasses)
    PerFunctionPasses->doFinalization();
    
  // Run module-level optimizers, if any are present.
  if (PerModulePasses)
    PerModulePasses->run(*TheModule);
  
  // Run the code generator, if present.
  if (CodeGenPasses) {
    CodeGenPasses->doInitialization();
    for (Module::iterator I = TheModule->begin(), E = TheModule->end();
         I != E; ++I)
      if (!I->isExternal())
        CodeGenPasses->run(*I);
    CodeGenPasses->doFinalization();
  }
  
  AsmOutStream->flush();
  delete AsmOutStream;
  AsmOutStream = 0;
  delete AsmOutFile;
  AsmOutFile = 0;
  timevar_pop(TV_LLVM_PERFILE);
}

// llvm_emit_code_for_current_function - Top level interface for emitting a
// function to the .s file.
void llvm_emit_code_for_current_function(tree fndecl) {
  if (cfun->static_chain_decl || cfun->nonlocal_goto_save_area)
    sorry("%Jnested functions not supported by LLVM", fndecl);
  
  if (errorcount || sorrycount) {
    TREE_ASM_WRITTEN(fndecl) = 1;
    return;  // Do not process broken code.
  }
  timevar_push(TV_LLVM_FUNCS);

  // Convert the AST to raw/ugly LLVM code.
  Function *Fn;
  {
    TreeToLLVM Emitter(fndecl);

    // Set up parameters and prepare for return, for the function.
    Emitter.StartFunctionBody();
    
    // Emit the body of the function.
    Emitter.Emit(DECL_SAVED_TREE(fndecl), 0);
  
    // Wrap things up.
    Fn = Emitter.FinishFunctionBody();
  }

#if 0
  if (dump_file) {
    fprintf (dump_file,
             "\n\n;;\n;; Full LLVM generated for this function:\n;;\n");
    Fn->dump();
  }
#endif
  
  if (PerFunctionPasses)
    PerFunctionPasses->run(*Fn);
  
  // TODO: Nuke the .ll code for the function at -O[01] if we don't want to
  // inline it or something else.
  
  // There's no need to defer outputting this function any more; we
  // know we want to output it.
  DECL_DEFER_OUTPUT(fndecl) = 0;
  
  // Finally, we have written out this function!
  TREE_ASM_WRITTEN(fndecl) = 1;
  timevar_pop(TV_LLVM_FUNCS);
}

/// emit_global_to_llvm - Emit the specified VAR_DECL or aggregate CONST_DECL to
/// LLVM as a global variable.  This function implements the end of
/// assemble_variable.
void emit_global_to_llvm(tree decl) {
  if (errorcount || sorrycount) return;

  // FIXME: Support alignment on globals: DECL_ALIGN.
  // FIXME: DECL_PRESERVE_P indicates the var is marked with attribute 'used'.
  if (TREE_CODE(decl) == VAR_DECL && DECL_THREAD_LOCAL(decl))
    sorry("thread-local data not supported by LLVM yet!");

  // Global register variables don't turn into LLVM GlobalVariables.
  if (TREE_CODE(decl) == VAR_DECL && DECL_REGISTER(decl))
    return;

  timevar_push(TV_LLVM_GLOBALS);

  // Get or create the global variable now.
  GlobalVariable *GV = cast<GlobalVariable>(DECL_LLVM(decl));
  
  // Convert the initializer over.
  Constant *Init;
  if (DECL_INITIAL(decl) == 0 || DECL_INITIAL(decl) == error_mark_node) {
    // This global should be zero initialized.  Reconvert the type in case the
    // forward def of the global and the real def differ in type (e.g. declared
    // as 'int A[]', and defined as 'int A[100]').
    Init = Constant::getNullValue(ConvertType(TREE_TYPE(decl)));
  } else {
    assert((TREE_CONSTANT(DECL_INITIAL(decl)) || 
            TREE_CODE(DECL_INITIAL(decl)) == STRING_CST) &&
           "Global initializer should be constant!");
    
    // Temporarily set an initializer for the global, so we don't infinitely
    // recurse.  If we don't do this, we can hit cases where we see "oh a global
    // with an initializer hasn't been initialized yet, call emit_global_to_llvm
    // on it".  When constructing the initializer it might refer to itself.
    // this can happen for things like void *G = &G;
    //
    GV->setInitializer(UndefValue::get(GV->getType()->getElementType()));
    Init = TreeConstantToLLVM::Convert(DECL_INITIAL(decl));
  }

  // If we had a forward definition that has a type that disagrees with our
  // initializer, insert a cast now.  This sort of thing occurs when we have a
  // global union, and the LLVM type followed a union initializer that is
  // different from the union element used for the type.
  if (GV->getType()->getElementType() != Init->getType()) {
    GV->removeFromParent();
    GlobalVariable *NGV = new GlobalVariable(Init->getType(), GV->isConstant(),
                                             GlobalValue::ExternalLinkage, 0,
                                             GV->getName(), TheModule);
    GV->replaceAllUsesWith(ConstantExpr::getBitCast(NGV, GV->getType()));
    delete GV;
    SET_DECL_LLVM(decl, NGV);
    EmittedGlobalVars[NGV->getName()] = NGV;
    GV = NGV;
  }
 
  // Set the initializer.
  GV->setInitializer(Init);
  
  // Set the linkage.
  if (!TREE_PUBLIC(decl)) {
    GV->setLinkage(GlobalValue::InternalLinkage);
  } else if (DECL_WEAK(decl) || DECL_ONE_ONLY(decl) ||
             (DECL_COMMON(decl) &&  // DECL_COMMON is only meaningful if no init
              (!DECL_INITIAL(decl) || DECL_INITIAL(decl) == error_mark_node))) {
    // llvm-gcc also includes DECL_VIRTUAL_P here.
    GV->setLinkage(GlobalValue::WeakLinkage);
  } else if (DECL_COMDAT(decl)) {
    GV->setLinkage(GlobalValue::LinkOnceLinkage);
  }

#ifdef TARGET_ADJUST_LLVM_LINKAGE
  TARGET_ADJUST_LLVM_LINKAGE(GV,decl);
#endif /* TARGET_ADJUST_LLVM_LINKAGE */
  
  // Set the section for the global.
  if (TREE_CODE(decl) == VAR_DECL || TREE_CODE(decl) == CONST_DECL) {
    if (DECL_SECTION_NAME(decl)) {
      GV->setSection(TREE_STRING_POINTER(DECL_SECTION_NAME(decl)));
#ifdef LLVM_IMPLICIT_TARGET_GLOBAL_VAR_SECTION
    } else if (const char *Section = 
                LLVM_IMPLICIT_TARGET_GLOBAL_VAR_SECTION(decl)) {
      GV->setSection(Section);
#endif
    }
    
    // Set the alignment for the global.
    if (DECL_ALIGN_UNIT(decl) && 
        getTargetData().getTypeAlignment(GV->getType()->getElementType()) !=
        DECL_ALIGN_UNIT(decl))
      GV->setAlignment(DECL_ALIGN_UNIT(decl));

    // Handle used decls
    if (DECL_PRESERVE_P (decl))
      AttributeUsedGlobals.push_back(GV);
  }
  
  if (TheDebugInfo) TheDebugInfo->EmitGlobalVariable(GV, decl); 
  
  timevar_pop(TV_LLVM_GLOBALS);
}


/// ValidateRegisterVariable - Check that a static "asm" variable is
/// well-formed.  If not, emit error messages and return true.  If so, return
/// false.
bool ValidateRegisterVariable(tree decl) {
  const char *Name = IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(decl));
  int RegNumber = decode_reg_name(Name);
  const Type *Ty = ConvertType(TREE_TYPE(decl));

  // If this has already been processed, don't emit duplicate error messages.
  if (DECL_LLVM_SET_P(decl)) {
    // Error state encoded into DECL_LLVM.
    return cast<ConstantInt>(DECL_LLVM(decl))->getZExtValue();
  }
  
  /* Detect errors in declaring global registers.  */
  if (RegNumber == -1)
    error("%Jregister name not specified for %qD", decl, decl);
  else if (RegNumber < 0)
    error("%Jinvalid register name for %qD", decl, decl);
  else if (TYPE_MODE(TREE_TYPE(decl)) == BLKmode)
    error("%Jdata type of %qD isn%'t suitable for a register", decl, decl);
#if 0 // FIXME: enable this.
  else if (!HARD_REGNO_MODE_OK(RegNumber, TYPE_MODE(TREE_TYPE(decl))))
    error("%Jregister specified for %qD isn%'t suitable for data type",
          decl, decl);
#endif
  else if (DECL_INITIAL(decl) != 0 && TREE_STATIC(decl))
    error("global register variable has initial value");
  else if (!Ty->isFirstClassType())
    sorry("%JLLVM cannot handle register variable %qD, report a bug",
          decl, decl);
  else {
    if (TREE_THIS_VOLATILE(decl))
      warning("volatile register variables don%'t work as you might wish");
    
    SET_DECL_LLVM(decl, ConstantInt::getFalse());
    return false;  // Everything ok.
  }
  SET_DECL_LLVM(decl, ConstantInt::getTrue());
  return true;
}


// make_decl_llvm - Create the DECL_RTL for a VAR_DECL or FUNCTION_DECL.  DECL
// should have static storage duration.  In other words, it should not be an
// automatic variable, including PARM_DECLs.
//
// There is, however, one exception: this function handles variables explicitly
// placed in a particular register by the user.
//
// This function corresponds to make_decl_rtl in varasm.c, and is implicitly
// called by DECL_LLVM if a decl doesn't have an LLVM set.
//
void make_decl_llvm(tree decl) {
#ifdef ENABLE_CHECKING
  // Check that we are not being given an automatic variable.
  // A weak alias has TREE_PUBLIC set but not the other bits.
  if (TREE_CODE(decl) == PARM_DECL || TREE_CODE(decl) == RESULT_DECL
      || (TREE_CODE(decl) == VAR_DECL && !TREE_STATIC(decl) &&
          !TREE_PUBLIC(decl) && !DECL_EXTERNAL(decl) && !DECL_REGISTER(decl)))
    abort();
  // And that we were not given a type or a label.  */
  else if (TREE_CODE(decl) == TYPE_DECL || TREE_CODE(decl) == LABEL_DECL)
    abort ();
#endif
  
  // For a duplicate declaration, we can be called twice on the
  // same DECL node.  Don't discard the LLVM already made.
  if (DECL_LLVM_SET_P(decl)) return;

  // Global register variable with asm name, e.g.:
  // register unsigned long esp __asm__("ebp");
  if (TREE_CODE(decl) != FUNCTION_DECL && DECL_REGISTER(decl)) {
    // This  just verifies that the variable is ok.  The actual "load/store"
    // code paths handle accesses to the variable.
    ValidateRegisterVariable(decl);
    return;
  }
  
  timevar_push(TV_LLVM_GLOBALS);

  const char *Name = "";
  if (DECL_NAME(decl))
    if (tree AssemblerName = DECL_ASSEMBLER_NAME(decl))
      Name = IDENTIFIER_POINTER(AssemblerName);
  
  // Now handle ordinary static variables and functions (in memory).
  // Also handle vars declared register invalidly.
  if (Name[0] == 1) {
#ifdef REGISTER_PREFIX
    if (strlen (REGISTER_PREFIX) != 0) {
      int reg_number = decode_reg_name(Name);
      if (reg_number >= 0 || reg_number == -3)
        error("%Jregister name given for non-register variable %qD",
              decl, decl);
    }
#endif
  }
  
  // Specifying a section attribute on a variable forces it into a
  // non-.bss section, and thus it cannot be common.
  if (TREE_CODE(decl) == VAR_DECL && DECL_SECTION_NAME(decl) != NULL_TREE &&
      DECL_INITIAL(decl) == NULL_TREE && DECL_COMMON(decl))
    DECL_COMMON(decl) = 0;
  
  // Variables can't be both common and weak.
  if (TREE_CODE(decl) == VAR_DECL && DECL_WEAK(decl))
    DECL_COMMON(decl) = 0;
  
  // Okay, now we need to create an LLVM global variable or function for this
  // object.  Note that this is quite possibly a forward reference to the
  // object, so its type may change later.
  if (TREE_CODE(decl) == FUNCTION_DECL) {
    assert(Name[0] && "Function with empty name!");
    // If this function has already been created, reuse the decl.  This happens
    // when we have something like __builtin_memset and memset in the same file.
    Function *&FnEntry = EmittedFunctions[Name];
    if (FnEntry == 0) {
      unsigned CC;
      const FunctionType *Ty = 
        TheTypeConverter->ConvertFunctionType(TREE_TYPE(decl), CC);
      FnEntry = new Function(Ty, Function::ExternalLinkage, Name, TheModule);
      FnEntry->setCallingConv(CC);

      // Check for external weak linkage
      if (DECL_EXTERNAL(decl) && DECL_WEAK(decl))
        FnEntry->setLinkage(Function::ExternalWeakLinkage);
      
#ifdef TARGET_ADJUST_LLVM_LINKAGE
      TARGET_ADJUST_LLVM_LINKAGE(FnEntry,decl);
#endif /* TARGET_ADJUST_LLVM_LINKAGE */

      assert(FnEntry->getName() == Name &&"Preexisting fn with the same name!");
    }
    SET_DECL_LLVM(decl, FnEntry);
  } else {
    assert((TREE_CODE(decl) == VAR_DECL ||
            TREE_CODE(decl) == CONST_DECL) && "Not a function or var decl?");
    const Type *Ty = ConvertType(TREE_TYPE(decl));
    GlobalVariable *GV ;

    // If we have "extern void foo", make the global have type {} instead of
    // type void.
    if (Ty == Type::VoidTy) Ty = StructType::get(std::vector<const Type*>(),
                                                 false);
    if (Name[0] == 0) {   // Global has no name.
      GV = new GlobalVariable(Ty, false, GlobalValue::ExternalLinkage, 0,
                              Name, TheModule);

      // Check for external weak linkage
      if (DECL_EXTERNAL(decl) && DECL_WEAK(decl))
        GV->setLinkage(GlobalValue::ExternalWeakLinkage);
      
#ifdef TARGET_ADJUST_LLVM_LINKAGE
      TARGET_ADJUST_LLVM_LINKAGE(GV,decl);
#endif /* TARGET_ADJUST_LLVM_LINKAGE */
    } else {
      // If the global has a name, prevent multiple vars with the same name from
      // being created.
      GlobalVariable *&GVE = EmittedGlobalVars[Name];
    
      if (GVE == 0) {
        GVE = GV = new GlobalVariable(Ty, false, GlobalValue::ExternalLinkage,0,
                                      Name, TheModule);

        // Check for external weak linkage
        if (DECL_EXTERNAL(decl) && DECL_WEAK(decl))
          GV->setLinkage(GlobalValue::ExternalWeakLinkage);
        
#ifdef TARGET_ADJUST_LLVM_LINKAGE
        TARGET_ADJUST_LLVM_LINKAGE(GV,decl);
#endif /* TARGET_ADJUST_LLVM_LINKAGE */
      } else {
        GV = GVE;  // Global already created, reuse it.
      }
    }
    
    if ((TREE_READONLY(decl) && !TREE_SIDE_EFFECTS(decl)) || 
        TREE_CODE(decl) == CONST_DECL) {
      if (DECL_EXTERNAL(decl)) {
        // Mark external globals constant even though they could be marked
        // non-constant in the defining translation unit.  The definition of the
        // global determines whether the global is ultimately constant or not,
        // marking this constant will allow us to do some extra (legal)
        // optimizations that we would otherwise not be able to do.  (In C++,
        // any global that is 'C++ const' may not be readonly: it could have a
        // dynamic initializer.
        //
        GV->setConstant(true);
      } else {
        // Mark readonly globals with constant initializers constant.
        if (DECL_INITIAL(decl) != error_mark_node && // uninitialized?
            DECL_INITIAL(decl) &&
            (TREE_CONSTANT(DECL_INITIAL(decl)) ||
             TREE_CODE(DECL_INITIAL(decl)) == STRING_CST))
          GV->setConstant(true);
      }
    }

    SET_DECL_LLVM(decl, GV);
  }
  timevar_pop(TV_LLVM_GLOBALS);
}

/// llvm_get_decl_name - Used by varasm.c, returns the specified declaration's
/// name.
const char *llvm_get_decl_name(void *LLVM) {
  return ((Value*)LLVM)->getName().c_str();
}

// llvm_mark_decl_weak - Used by varasm.c, called when a decl is found to be
// weak, but it already had an llvm object created for it. This marks the LLVM
// object weak as well.
void llvm_mark_decl_weak(tree decl) {
  assert(DECL_LLVM_SET_P(decl) && DECL_WEAK(decl) &&
         isa<GlobalValue>(DECL_LLVM(decl)) && "Decl isn't marked weak!");
  GlobalValue *GV = cast<GlobalValue>(DECL_LLVM(decl));

  // Do not mark something that is already known to be linkonce or internal.
  if (GV->hasExternalLinkage()) {
    if (GV->isExternal())
      GV->setLinkage(GlobalValue::ExternalWeakLinkage);
    else
      GV->setLinkage(GlobalValue::WeakLinkage);
  }
}

// llvm_emit_ctor_dtor - Called to emit static ctors/dtors to LLVM code.  fndecl
// is a 'void()' FUNCTION_DECL for the code, initprio is the init priority, and
// isCtor indicates whether this is a ctor or dtor.
//
void llvm_emit_ctor_dtor(tree FnDecl, int InitPrio, int isCtor) {
  mark_decl_referenced(FnDecl);  // Inform cgraph that we used the global.
  Function *F = cast<Function>(DECL_LLVM(FnDecl));
  (isCtor ? &StaticCtors:&StaticDtors)->push_back(std::make_pair(F, InitPrio));
}

void llvm_emit_typedef(tree decl) {
  // Need hooks for debug info?
  return;
}

// llvm_emit_file_scope_asm - Emit the specified string as a file-scope inline
// asm block.
//
void llvm_emit_file_scope_asm(tree string) {
  if (TREE_CODE(string) == ADDR_EXPR)
    string = TREE_OPERAND(string, 0);
  if (TheModule->getModuleInlineAsm().empty())
    TheModule->setModuleInlineAsm(TREE_STRING_POINTER(string));
  else
    TheModule->setModuleInlineAsm(TheModule->getModuleInlineAsm() + "\n" +
                                  TREE_STRING_POINTER(string));
}


// print_llvm - Print the specified LLVM chunk like an operand, called by
// print-tree.c for tree dumps.
//
void print_llvm(FILE *file, void *LLVM) {
  oFILEstream FS(file);
  FS << "LLVM: ";
  WriteAsOperand(FS, (Value*)LLVM, true, TheModule);
}

/* APPLE LOCAL end LLVM (ENTIRE FILE!)  */
