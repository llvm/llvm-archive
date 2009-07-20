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

// LLVM headers
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/ModuleProvider.h"
#include "llvm/PassManager.h"
#include "llvm/ValueSymbolTable.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/Target/SubtargetFeature.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/StandardPasses.h"
#include "llvm/Support/Streams.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/System/Program.h"

// System headers
#include <cassert>

// GCC headers
#undef VISIBILITY_HIDDEN
#define IN_GCC

#include "config.h"
extern "C" {
#include "system.h"
}
#include "coretypes.h"
#include "target.h"
#include "tree.h"

#include "flags.h"
#include "diagnostic.h"
#include "output.h"
#include "toplev.h"
#include "timevar.h"
#include "tm.h"
#include "function.h"
#include "tree-inline.h"
#include "langhooks.h"
#include "cgraph.h"
#include "params.h"
#include "plugin-version.h"

// Plugin headers
#include "llvm-internal.h"
#include "llvm-debug.h"
//TODO#include "llvm-file-ostream.h"
#include "bits_and_bobs.h"

// Non-zero if bytecode from PCH is successfully read.
int flag_llvm_pch_read;

// Non-zero if libcalls should not be simplified.
int flag_no_simplify_libcalls;

// Non-zero if red-zone is disabled.
static int flag_disable_red_zone = 0;

// Non-zero if implicit floating point instructions are disabled.
static int flag_no_implicit_float = 0;

// Global state for the LLVM backend.
Module *TheModule = 0;
DebugInfo *TheDebugInfo = 0;
TargetMachine *TheTarget = 0;
TargetFolder *TheFolder = 0;
TypeConverter *TheTypeConverter = 0;
llvm::OStream *AsmOutFile = 0;
llvm::OStream *AsmIntermediateOutFile = 0;

/// DisableLLVMOptimizations - Allow the user to specify:
/// "-mllvm -disable-llvm-optzns" on the llvm-gcc command line to force llvm
/// optimizations off.
static cl::opt<bool> DisableLLVMOptimizations("disable-llvm-optzns");

std::vector<std::pair<Constant*, int> > StaticCtors, StaticDtors;
SmallSetVector<Constant*, 32> AttributeUsedGlobals;
std::vector<Constant*> AttributeAnnotateGlobals;

/// PerFunctionPasses - This is the list of cleanup passes run per-function
/// as each is compiled.  In cases where we are not doing IPO, it includes the 
/// code generator.
static FunctionPassManager *PerFunctionPasses = 0;
static PassManager *PerModulePasses = 0;
static FunctionPassManager *CodeGenPasses = 0;

static void createPerFunctionOptimizationPasses();
static void createPerModuleOptimizationPasses();
static void destroyOptimizationPasses();

//TODO//===----------------------------------------------------------------------===//
//TODO//                   Matching LLVM Values with GCC DECL trees
//TODO//===----------------------------------------------------------------------===//
//TODO//
//TODO// LLVMValues is a vector of LLVM Values. GCC tree nodes keep track of LLVM
//TODO// Values using this vector's index. It is easier to save and restore the index
//TODO// than the LLVM Value pointer while using PCH.
//TODO
//TODO// Collection of LLVM Values
//TODOstatic std::vector<Value *> LLVMValues;
//TODOtypedef DenseMap<Value *, unsigned> LLVMValuesMapTy;
//TODOstatic LLVMValuesMapTy LLVMValuesMap;
//TODO
//TODO/// LocalLLVMValueIDs - This is the set of local IDs we have in our mapping,
//TODO/// this allows us to efficiently identify and remove them.  Local IDs are IDs
//TODO/// for values that are local to the current function being processed.  These do
//TODO/// not need to go into the PCH file, but DECL_LLVM still needs a valid index
//TODO/// while converting the function.  Using "Local IDs" allows the IDs for
//TODO/// function-local decls to be recycled after the function is done.
//TODOstatic std::vector<unsigned> LocalLLVMValueIDs;
//TODO
//TODO// Remember the LLVM value for GCC tree node.
//TODOvoid llvm_set_decl(tree Tr, Value *V) {
//TODO
//TODO  // If there is not any value then do not add new LLVMValues entry.
//TODO  // However clear Tr index if it is non zero.
//TODO  if (!V) {
//TODO    if (GET_DECL_LLVM_INDEX(Tr))
//TODO      SET_DECL_LLVM_INDEX(Tr, 0);
//TODO    return;
//TODO  }
//TODO
//TODO  unsigned &ValueSlot = LLVMValuesMap[V];
//TODO  if (ValueSlot) {
//TODO    // Already in map
//TODO    SET_DECL_LLVM_INDEX(Tr, ValueSlot);
//TODO    return;
//TODO  }
//TODO
//TODO  LLVMValues.push_back(V);
//TODO  unsigned Index = LLVMValues.size();
//TODO  SET_DECL_LLVM_INDEX(Tr, Index);
//TODO  LLVMValuesMap[V] = Index;
//TODO
//TODO  // Remember local values.
//TODO  if (!isa<Constant>(V))
//TODO    LocalLLVMValueIDs.push_back(Index);
//TODO}
//TODO
//TODO// Return TRUE if there is a LLVM Value associate with GCC tree node.
//TODObool llvm_set_decl_p(tree Tr) {
//TODO  unsigned Index = GET_DECL_LLVM_INDEX(Tr);
//TODO  if (Index == 0)
//TODO    return false;
//TODO
//TODO  return LLVMValues[Index - 1] != 0;
//TODO}
//TODO
//TODO// Get LLVM Value for the GCC tree node based on LLVMValues vector index.
//TODO// If there is not any value associated then use make_decl_llvm() to
//TODO// make LLVM value. When GCC tree node is initialized, it has 0 as the
//TODO// index value. This is why all recorded indices are offset by 1.
//TODOValue *llvm_get_decl(tree Tr) {
//TODO
//TODO  unsigned Index = GET_DECL_LLVM_INDEX(Tr);
//TODO  if (Index == 0) {
//TODO    make_decl_llvm(Tr);
//TODO    Index = GET_DECL_LLVM_INDEX(Tr);
//TODO
//TODO    // If there was an error, we may have disabled creating LLVM values.
//TODO    if (Index == 0) return 0;
//TODO  }
//TODO  assert((Index - 1) < LLVMValues.size() && "Invalid LLVM value index");
//TODO  assert(LLVMValues[Index - 1] && "Trying to use deleted LLVM value!");
//TODO
//TODO  return LLVMValues[Index - 1];
//TODO}
//TODO
//TODO/// changeLLVMConstant - Replace Old with New everywhere, updating all maps
//TODO/// (except for AttributeAnnotateGlobals, which is a different kind of animal).
//TODO/// At this point we know that New is not in any of these maps.
//TODOvoid changeLLVMConstant(Constant *Old, Constant *New) {
//TODO  assert(Old->use_empty() && "Old value has uses!");
//TODO
//TODO  if (AttributeUsedGlobals.count(Old)) {
//TODO    AttributeUsedGlobals.remove(Old);
//TODO    AttributeUsedGlobals.insert(New);
//TODO  }
//TODO
//TODO  for (unsigned i = 0, e = StaticCtors.size(); i != e; ++i) {
//TODO    if (StaticCtors[i].first == Old)
//TODO      StaticCtors[i].first = New;
//TODO  }
//TODO
//TODO  for (unsigned i = 0, e = StaticDtors.size(); i != e; ++i) {
//TODO    if (StaticDtors[i].first == Old)
//TODO      StaticDtors[i].first = New;
//TODO  }
//TODO
//TODO  assert(!LLVMValuesMap.count(New) && "New cannot be in the LLVMValues map!");
//TODO
//TODO  // Find Old in the table.
//TODO  LLVMValuesMapTy::iterator I = LLVMValuesMap.find(Old);
//TODO  if (I == LLVMValuesMap.end()) return;
//TODO
//TODO  unsigned Idx = I->second-1;
//TODO  assert(Idx < LLVMValues.size() && "Out of range index!");
//TODO  assert(LLVMValues[Idx] == Old && "Inconsistent LLVMValues mapping!");
//TODO
//TODO  LLVMValues[Idx] = New;
//TODO
//TODO  // Remove the old value from the value map.
//TODO  LLVMValuesMap.erase(I);
//TODO
//TODO  // Insert the new value into the value map.  We know that it can't already
//TODO  // exist in the mapping.
//TODO  if (New)
//TODO    LLVMValuesMap[New] = Idx+1;
//TODO}
//TODO
//TODO// Read LLVM Types string table
//TODOvoid readLLVMValues() {
//TODO  GlobalValue *V = TheModule->getNamedGlobal("llvm.pch.values");
//TODO  if (!V)
//TODO    return;
//TODO
//TODO  GlobalVariable *GV = cast<GlobalVariable>(V);
//TODO  ConstantStruct *ValuesFromPCH = cast<ConstantStruct>(GV->getOperand(0));
//TODO
//TODO  for (unsigned i = 0; i < ValuesFromPCH->getNumOperands(); ++i) {
//TODO    Value *Va = ValuesFromPCH->getOperand(i);
//TODO
//TODO    if (!Va) {
//TODO      // If V is empty then insert NULL to represent empty entries.
//TODO      LLVMValues.push_back(Va);
//TODO      continue;
//TODO    }
//TODO    if (ConstantArray *CA = dyn_cast<ConstantArray>(Va)) {
//TODO      std::string Str = CA->getAsString();
//TODO      Va = TheModule->getValueSymbolTable().lookup(Str);
//TODO    }
//TODO    assert (Va != NULL && "Invalid Value in LLVMValues string table");
//TODO    LLVMValues.push_back(Va);
//TODO  }
//TODO
//TODO  // Now, llvm.pch.values is not required so remove it from the symbol table.
//TODO  GV->eraseFromParent();
//TODO}
//TODO
//TODO// GCC tree's uses LLVMValues vector's index to reach LLVM Values.
//TODO// Create a string table to hold these LLVM Values' names. This string
//TODO// table will be used to recreate LTypes vector after loading PCH.
//TODOvoid writeLLVMValues() {
//TODO  if (LLVMValues.empty())
//TODO    return;
//TODO
//TODO  LLVMContext &Context = getGlobalContext();
//TODO
//TODO  std::vector<Constant *> ValuesForPCH;
//TODO  for (std::vector<Value *>::iterator I = LLVMValues.begin(),
//TODO         E = LLVMValues.end(); I != E; ++I)  {
//TODO    if (Constant *C = dyn_cast_or_null<Constant>(*I))
//TODO      ValuesForPCH.push_back(C);
//TODO    else
//TODO      // Non constant values, e.g. arguments, are not at global scope.
//TODO      // When PCH is read, only global scope values are used.
//TODO      ValuesForPCH.push_back(Context.getNullValue(Type::Int32Ty));
//TODO  }
//TODO
//TODO  // Create string table.
//TODO  Constant *LLVMValuesTable = Context.getConstantStruct(ValuesForPCH, false);
//TODO
//TODO  // Create variable to hold this string table.
//TODO  new GlobalVariable(*TheModule, LLVMValuesTable->getType(), true,
//TODO                     GlobalValue::ExternalLinkage,
//TODO                     LLVMValuesTable,
//TODO                     "llvm.pch.values");
//TODO}
//TODO
//TODO/// eraseLocalLLVMValues - drop all non-global values from the LLVM values map.
//TODOvoid eraseLocalLLVMValues() {
//TODO  // Erase all the local values, these are stored in LocalLLVMValueIDs.
//TODO  while (!LocalLLVMValueIDs.empty()) {
//TODO    unsigned Idx = LocalLLVMValueIDs.back()-1;
//TODO    LocalLLVMValueIDs.pop_back();
//TODO
//TODO    if (Value *V = LLVMValues[Idx]) {
//TODO      assert(!isa<Constant>(V) && "Found global value");
//TODO      LLVMValuesMap.erase(V);
//TODO    }
//TODO
//TODO    if (Idx == LLVMValues.size()-1)
//TODO      LLVMValues.pop_back();
//TODO    else
//TODO      LLVMValues[Idx] = 0;
//TODO  }
//TODO}


// Forward decl visibility style to global.
void handleVisibility(tree decl, GlobalValue *GV) {
  // If decl has visibility specified explicitely (via attribute) - honour
  // it. Otherwise (e.g. visibility specified via -fvisibility=hidden) honour
  // only if symbol is local.
  if (TREE_PUBLIC(decl) &&
      (DECL_VISIBILITY_SPECIFIED(decl) || !DECL_EXTERNAL(decl))) {
    if (DECL_VISIBILITY(decl) == VISIBILITY_HIDDEN)
      GV->setVisibility(GlobalValue::HiddenVisibility);
    else if (DECL_VISIBILITY(decl) == VISIBILITY_PROTECTED)
      GV->setVisibility(GlobalValue::ProtectedVisibility);
    else if (DECL_VISIBILITY(decl) == VISIBILITY_DEFAULT)
      GV->setVisibility(Function::DefaultVisibility);
  }
}

//TODO#ifndef LLVM_TARGET_NAME
//TODO#error LLVM_TARGET_NAME macro not specified by GCC backend
//TODO#endif
//TODO
//TODOnamespace llvm {
//TODO#define Declare2(TARG, MOD)   extern "C" void LLVMInitialize ## TARG ## MOD()
//TODO#define Declare(T, M) Declare2(T, M)
//TODO  Declare(LLVM_TARGET_NAME, TargetInfo);
//TODO  Declare(LLVM_TARGET_NAME, Target);
//TODO  Declare(LLVM_TARGET_NAME, AsmPrinter);
//TODO#undef Declare
//TODO#undef Declare2
//TODO}

/// LazilyConfigureLLVM - Set LLVM configuration options, if not already set.
/// already created.
static void LazilyConfigureLLVM(void) {
  static bool Configured = false;
  if (Configured)
    return;

  // Initialize LLVM command line options.
  std::vector<const char*> Args;
  Args.push_back(progname); // program name

//TODO  // Allow targets to specify PIC options and other stuff to the corresponding
//TODO  // LLVM backends.
//TODO#ifdef LLVM_SET_RED_ZONE_FLAG
//TODO  LLVM_SET_RED_ZONE_FLAG(flag_disable_red_zone)
//TODO#endif
//TODO#ifdef LLVM_SET_TARGET_OPTIONS
//TODO  LLVM_SET_TARGET_OPTIONS(Args);
//TODO#endif
//TODO#ifdef LLVM_SET_MACHINE_OPTIONS
//TODO  LLVM_SET_MACHINE_OPTIONS(Args);
//TODO#endif
//TODO#ifdef LLVM_SET_IMPLICIT_FLOAT
//TODO  LLVM_SET_IMPLICIT_FLOAT(flag_no_implicit_float)
//TODO#endif

  if (time_report)
    Args.push_back("--time-passes");
  if (fast_math_flags_set_p())
    Args.push_back("--enable-unsafe-fp-math");
  if (!flag_omit_frame_pointer)
    Args.push_back("--disable-fp-elim");
  if (!flag_zero_initialized_in_bss)
    Args.push_back("--nozero-initialized-in-bss");
  if (flag_debug_asm)
    Args.push_back("--asm-verbose");
//TODO  if (flag_debug_pass_structure)
//TODO    Args.push_back("--debug-pass=Structure");
//TODO  if (flag_debug_pass_arguments)
//TODO    Args.push_back("--debug-pass=Arguments");
  if (optimize_size || optimize < 3)
    // Reduce inline limit. Default limit is 200.
    Args.push_back("--inline-threshold=50");
  if (flag_unwind_tables)
    Args.push_back("--unwind-tables");

//TODO  // If there are options that should be passed through to the LLVM backend
//TODO  // directly from the command line, do so now.  This is mainly for debugging
//TODO  // purposes, and shouldn't really be for general use.
//TODO  std::vector<std::string> ArgStrings;
//TODO
//TODO  if (flag_limited_precision > 0) {
//TODO    std::string Arg("--limit-float-precision="+utostr(flag_limited_precision));
//TODO    ArgStrings.push_back(Arg);
//TODO  }
//TODO
//TODO  if (flag_stack_protect > 0) {
//TODO    std::string Arg("--stack-protector-buffer-size=" +
//TODO                    utostr(PARAM_VALUE(PARAM_SSP_BUFFER_SIZE)));
//TODO    ArgStrings.push_back(Arg);
//TODO  }
//TODO
//TODO  for (unsigned i = 0, e = ArgStrings.size(); i != e; ++i)
//TODO    Args.push_back(ArgStrings[i].c_str());
//TODO
//TODO  std::vector<std::string> LLVM_Optns; // Avoid deallocation before opts parsed!
//TODO  if (llvm_optns) {
//TODO    SplitString(llvm_optns, LLVM_Optns);
//TODO    for(unsigned i = 0, e = LLVM_Optns.size(); i != e; ++i)
//TODO      Args.push_back(LLVM_Optns[i].c_str());
//TODO  }

  Args.push_back(0);  // Null terminator.
  int pseudo_argc = Args.size()-1;
  llvm::cl::ParseCommandLineOptions(pseudo_argc, (char**)&Args[0]);

  Configured = true;
}

/// LazilyInitializeModule - Create a module to output LLVM IR to, if it wasn't
/// already created.
static void LazilyInitializeModule(void) {
  static bool Initialized = false;
  if (Initialized)
    return;

  LazilyConfigureLLVM;

  TheModule = new Module("", getGlobalContext());

  // If the target wants to override the architecture, e.g. turning
  // powerpc-darwin-... into powerpc64-darwin-... when -m64 is enabled, do so
  // now.
  std::string TargetTriple = "x86_64-linux-gnu"; // FIXME!
//TODO  std::string TargetTriple = TARGET_NAME;
//TODO#ifdef LLVM_OVERRIDE_TARGET_ARCH
//TODO  std::string Arch = LLVM_OVERRIDE_TARGET_ARCH();
//TODO  if (!Arch.empty()) {
//TODO    std::string::size_type DashPos = TargetTriple.find('-');
//TODO    if (DashPos != std::string::npos)// If we have a sane t-t, replace the arch.
//TODO      TargetTriple = Arch + TargetTriple.substr(DashPos);
//TODO  }
//TODO#endif
//TODO#ifdef LLVM_OVERRIDE_TARGET_VERSION
//TODO  char *NewTriple;
//TODO  bool OverRidden = LLVM_OVERRIDE_TARGET_VERSION(TargetTriple.c_str(),
//TODO                                                 &NewTriple);
//TODO  if (OverRidden)
//TODO    TargetTriple = std::string(NewTriple);
//TODO#endif
  TheModule->setTargetTriple(TargetTriple);

  TheTypeConverter = new TypeConverter();

  // Create the TargetMachine we will be generating code with.
  // FIXME: Figure out how to select the target and pass down subtarget info.
  std::string Err;
  const Target *TME =
    TargetRegistry::getClosestStaticTargetForModule(*TheModule, Err);
  if (!TME) {
    cerr << "Did not get a target machine! Triplet is " << TargetTriple << '\n';
    exit(1);
  }

  // Figure out the subtarget feature string we pass to the target.
  std::string FeatureStr;
//TODO  // The target can set LLVM_SET_SUBTARGET_FEATURES to configure the LLVM
//TODO  // backend.
//TODO#ifdef LLVM_SET_SUBTARGET_FEATURES
//TODO  SubtargetFeatures Features;
//TODO  LLVM_SET_SUBTARGET_FEATURES(Features);
//TODO  FeatureStr = Features.getString();
//TODO#endif
  TheTarget = TME->createTargetMachine(*TheModule, FeatureStr);
  assert(TheTarget->getTargetData()->isBigEndian() == BYTES_BIG_ENDIAN);

  TheFolder = new TargetFolder(TheTarget->getTargetData(), getGlobalContext());

  // Install information about target datalayout stuff into the module for
  // optimizer use.
  TheModule->setDataLayout(TheTarget->getTargetData()->
                           getStringRepresentation());

//TODO  if (optimize)
//TODO    RegisterRegAlloc::setDefault(createLinearScanRegisterAllocator);
//TODO  else
//TODO    RegisterRegAlloc::setDefault(createLocalRegisterAllocator);

//TODO  // FIXME - Do not disable debug info while writing pch.
//TODO  if (!flag_pch_file &&
//TODO      debug_info_level > DINFO_LEVEL_NONE)
//TODO    TheDebugInfo = new DebugInfo(TheModule);
//TODO}
//TODO
//TODO/// Set backend options that may only be known at codegen time.
//TODOvoid performLateBackendInitialization(void) {
//TODO  // The Ada front-end sets flag_exceptions only after processing the file.
//TODO  ExceptionHandling = flag_exceptions;
//TODO  for (Module::iterator I = TheModule->begin(), E = TheModule->end();
//TODO       I != E; ++I)
//TODO    if (!I->isDeclaration()) {
//TODO      if (flag_disable_red_zone)
//TODO        I->addFnAttr(Attribute::NoRedZone);
//TODO      if (flag_no_implicit_float)
//TODO        I->addFnAttr(Attribute::NoImplicitFloat);
//TODO    }
//TODO}
//TODO
//TODOvoid llvm_lang_dependent_init(const char *Name) {
//TODO  if (TheDebugInfo)
//TODO    TheDebugInfo->Initialize();
//TODO  if (Name)
//TODO    TheModule->setModuleIdentifier(Name);
//TODO}
  Initialized = true;
}

void llvm_lang_dependent_init(const char *Name) {
  if (TheDebugInfo)
    TheDebugInfo->Initialize();
  if (Name)
    TheModule->setModuleIdentifier(Name);
}

//TODOoFILEstream *AsmOutStream = 0;
//TODOstatic formatted_raw_ostream *AsmOutRawStream = 0;
//TODOoFILEstream *AsmIntermediateOutStream = 0;
//TODO
//TODO/// Read bytecode from PCH file. Initialize TheModule and setup
//TODO/// LTypes vector.
//TODOvoid llvm_pch_read(const unsigned char *Buffer, unsigned Size) {
//TODO  std::string ModuleName = TheModule->getModuleIdentifier();
//TODO
//TODO  delete TheModule;
//TODO  delete TheDebugInfo;
//TODO
//TODO  clearTargetBuiltinCache();
//TODO
//TODO  MemoryBuffer *MB = MemoryBuffer::getNewMemBuffer(Size, ModuleName.c_str());
//TODO  memcpy((char*)MB->getBufferStart(), Buffer, Size);
//TODO
//TODO  std::string ErrMsg;
//TODO  TheModule = ParseBitcodeFile(MB, getGlobalContext(), &ErrMsg);
//TODO  delete MB;
//TODO
//TODO  // FIXME - Do not disable debug info while writing pch.
//TODO  if (!flag_pch_file && debug_info_level > DINFO_LEVEL_NONE) {
//TODO    TheDebugInfo = new DebugInfo(TheModule);
//TODO    TheDebugInfo->Initialize();
//TODO  }
//TODO
//TODO  if (!TheModule) {
//TODO    cerr << "Error reading bytecodes from PCH file\n";
//TODO    cerr << ErrMsg << "\n";
//TODO    exit(1);
//TODO  }
//TODO
//TODO  if (PerFunctionPasses || PerModulePasses) {
//TODO    destroyOptimizationPasses();
//TODO
//TODO    // Don't run codegen, when we should output PCH
//TODO    if (flag_pch_file)
//TODO      llvm_pch_write_init();
//TODO  }
//TODO
//TODO  // Read LLVM Types string table
//TODO  readLLVMTypesStringTable();
//TODO  readLLVMValues();
//TODO
//TODO  flag_llvm_pch_read = 1;
//TODO}
//TODO
//TODO// Initialize PCH writing. 
//TODOvoid llvm_pch_write_init(void) {
//TODO  timevar_push(TV_LLVM_INIT);
//TODO  AsmOutStream = new oFILEstream(asm_out_file);
//TODO  // FIXME: disentangle ostream madness here.  Kill off ostream and FILE.
//TODO  AsmOutRawStream =
//TODO    new formatted_raw_ostream(*new raw_os_ostream(*AsmOutStream),
//TODO                              formatted_raw_ostream::DELETE_STREAM);
//TODO  AsmOutFile = new OStream(*AsmOutStream);
//TODO
//TODO  PerModulePasses = new PassManager();
//TODO  PerModulePasses->add(new TargetData(*TheTarget->getTargetData()));
//TODO
//TODO  // If writing to stdout, set binary mode.
//TODO  if (asm_out_file == stdout)
//TODO    sys::Program::ChangeStdoutToBinary();
//TODO
//TODO  // Emit an LLVM .bc file to the output.  This is used when passed
//TODO  // -emit-llvm -c to the GCC driver.
//TODO  PerModulePasses->add(CreateBitcodeWriterPass(*AsmOutStream));
//TODO  
//TODO  // Disable emission of .ident into the output file... which is completely
//TODO  // wrong for llvm/.bc emission cases.
//TODO  flag_no_ident = 1;
//TODO
//TODO  flag_llvm_pch_read = 0;
//TODO
//TODO  timevar_pop(TV_LLVM_INIT);
//TODO}

//TODOstatic void destroyOptimizationPasses() {
//TODO  delete PerFunctionPasses;
//TODO  delete PerModulePasses;
//TODO  delete CodeGenPasses;
//TODO
//TODO  PerFunctionPasses = 0;
//TODO  PerModulePasses   = 0;
//TODO  CodeGenPasses     = 0;
//TODO}
//TODO
//TODOstatic void createPerFunctionOptimizationPasses() {
//TODO  if (PerFunctionPasses) 
//TODO    return;
//TODO
//TODO  // Create and set up the per-function pass manager.
//TODO  // FIXME: Move the code generator to be function-at-a-time.
//TODO  PerFunctionPasses =
//TODO    new FunctionPassManager(new ExistingModuleProvider(TheModule));
//TODO  PerFunctionPasses->add(new TargetData(*TheTarget->getTargetData()));
//TODO
//TODO  // In -O0 if checking is disabled, we don't even have per-function passes.
//TODO  bool HasPerFunctionPasses = false;
//TODO#ifdef ENABLE_CHECKING
//TODO  PerFunctionPasses->add(createVerifierPass());
//TODO  HasPerFunctionPasses = true;
//TODO#endif
//TODO
//TODO  if (optimize > 0 && !DisableLLVMOptimizations) {
//TODO    HasPerFunctionPasses = true;
//TODO    PerFunctionPasses->add(createCFGSimplificationPass());
//TODO    if (optimize == 1)
//TODO      PerFunctionPasses->add(createPromoteMemoryToRegisterPass());
//TODO    else
//TODO      PerFunctionPasses->add(createScalarReplAggregatesPass());
//TODO    PerFunctionPasses->add(createInstructionCombiningPass());
//TODO  }
//TODO
//TODO  // If there are no module-level passes that have to be run, we codegen as
//TODO  // each function is parsed.
//TODO  // FIXME: We can't figure this out until we know there are no always-inline
//TODO  // functions.
//TODO  // FIXME: This is disabled right now until bugs can be worked out.  Reenable
//TODO  // this for fast -O0 compiles!
//TODO  if (!emit_llvm_bc && !emit_llvm && 0) {
//TODO    FunctionPassManager *PM = PerFunctionPasses;    
//TODO    HasPerFunctionPasses = true;
//TODO
//TODO    CodeGenOpt::Level OptLevel = CodeGenOpt::Default;
//TODO
//TODO    switch (optimize) {
//TODO    default: break;
//TODO    case 0: OptLevel = CodeGenOpt::None; break;
//TODO    case 3: OptLevel = CodeGenOpt::Aggressive; break;
//TODO    }
//TODO
//TODO    // Normal mode, emit a .s file by running the code generator.
//TODO    // Note, this also adds codegenerator level optimization passes.
//TODO    switch (TheTarget->addPassesToEmitFile(*PM, *AsmOutRawStream,
//TODO                                           TargetMachine::AssemblyFile,
//TODO                                           OptLevel)) {
//TODO    default:
//TODO    case FileModel::Error:
//TODO      cerr << "Error interfacing to target machine!\n";
//TODO      exit(1);
//TODO    case FileModel::AsmFile:
//TODO      break;
//TODO    }
//TODO
//TODO    if (TheTarget->addPassesToEmitFileFinish(*PM, (MachineCodeEmitter *)0,
//TODO                                             OptLevel)) {
//TODO      cerr << "Error interfacing to target machine!\n";
//TODO      exit(1);
//TODO    }
//TODO  }
//TODO  
//TODO  if (HasPerFunctionPasses) {
//TODO    PerFunctionPasses->doInitialization();
//TODO  } else {
//TODO    delete PerFunctionPasses;
//TODO    PerFunctionPasses = 0;
//TODO  }
//TODO}
//TODO
//TODOstatic void createPerModuleOptimizationPasses() {
//TODO  if (PerModulePasses)
//TODO    // llvm_pch_write_init has already created the per module passes.
//TODO    return;
//TODO
//TODO  // FIXME: AT -O0/O1, we should stream out functions at a time.
//TODO  PerModulePasses = new PassManager();
//TODO  PerModulePasses->add(new TargetData(*TheTarget->getTargetData()));
//TODO  bool HasPerModulePasses = false;
//TODO
//TODO  if (!DisableLLVMOptimizations) {
//TODO    bool NeedAlwaysInliner = false;
//TODO    llvm::Pass *InliningPass = 0;
//TODO    if (flag_inline_trees > 1) {                // respect -fno-inline-functions
//TODO      InliningPass = createFunctionInliningPass();    // Inline small functions
//TODO    } else {
//TODO      // If full inliner is not run, check if always-inline is needed to handle
//TODO      // functions that are  marked as always_inline.
//TODO      for (Module::iterator I = TheModule->begin(), E = TheModule->end();
//TODO           I != E; ++I)
//TODO        if (I->hasFnAttr(Attribute::AlwaysInline)) {
//TODO          NeedAlwaysInliner = true;
//TODO          break;
//TODO        }
//TODO
//TODO      if (NeedAlwaysInliner)
//TODO        InliningPass = createAlwaysInlinerPass();  // Inline always_inline funcs
//TODO    }
//TODO
//TODO    HasPerModulePasses = true;
//TODO    createStandardModulePasses(PerModulePasses, optimize,
//TODO                               optimize_size || optimize < 3,
//TODO                               flag_unit_at_a_time, flag_unroll_loops,
//TODO                               !flag_no_simplify_libcalls, flag_exceptions,
//TODO                               InliningPass);
//TODO  }
//TODO
//TODO  if (emit_llvm_bc) {
//TODO    // Emit an LLVM .bc file to the output.  This is used when passed
//TODO    // -emit-llvm -c to the GCC driver.
//TODO    PerModulePasses->add(CreateBitcodeWriterPass(*AsmOutStream));
//TODO    HasPerModulePasses = true;
//TODO  } else if (emit_llvm) {
//TODO    // Emit an LLVM .ll file to the output.  This is used when passed 
//TODO    // -emit-llvm -S to the GCC driver.
//TODO    PerModulePasses->add(createPrintModulePass(AsmOutRawStream));
//TODO    HasPerModulePasses = true;
//TODO  } else {
//TODO    // If there are passes we have to run on the entire module, we do codegen
//TODO    // as a separate "pass" after that happens.
//TODO    // However if there are no module-level passes that have to be run, we
//TODO    // codegen as each function is parsed.
//TODO    // FIXME: This is disabled right now until bugs can be worked out.  Reenable
//TODO    // this for fast -O0 compiles!
//TODO    if (PerModulePasses || 1) {
//TODO      FunctionPassManager *PM = CodeGenPasses =
//TODO        new FunctionPassManager(new ExistingModuleProvider(TheModule));
//TODO      PM->add(new TargetData(*TheTarget->getTargetData()));
//TODO
//TODO      CodeGenOpt::Level OptLevel = CodeGenOpt::Default;
//TODO
//TODO      switch (optimize) {
//TODO      default: break;
//TODO      case 0: OptLevel = CodeGenOpt::None; break;
//TODO      case 3: OptLevel = CodeGenOpt::Aggressive; break;
//TODO      }
//TODO
//TODO      // Normal mode, emit a .s file by running the code generator.
//TODO      // Note, this also adds codegenerator level optimization passes.
//TODO      switch (TheTarget->addPassesToEmitFile(*PM, *AsmOutRawStream,
//TODO                                             TargetMachine::AssemblyFile,
//TODO                                             OptLevel)) {
//TODO      default:
//TODO      case FileModel::Error:
//TODO        cerr << "Error interfacing to target machine!\n";
//TODO        exit(1);
//TODO      case FileModel::AsmFile:
//TODO        break;
//TODO      }
//TODO
//TODO      if (TheTarget->addPassesToEmitFileFinish(*PM, (MachineCodeEmitter *)0,
//TODO                                               OptLevel)) {
//TODO        cerr << "Error interfacing to target machine!\n";
//TODO        exit(1);
//TODO      }
//TODO    }
//TODO  }
//TODO
//TODO  if (!HasPerModulePasses) {
//TODO    delete PerModulePasses;
//TODO    PerModulePasses = 0;
//TODO  }
//TODO}
//TODO
//TODO// llvm_asm_file_start - Start the .s file.
//TODOvoid llvm_asm_file_start(void) {
//TODO  timevar_push(TV_LLVM_INIT);
//TODO  AsmOutStream = new oFILEstream(asm_out_file);
//TODO  // FIXME: disentangle ostream madness here.  Kill off ostream and FILE.
//TODO  AsmOutRawStream =
//TODO    new formatted_raw_ostream(*new raw_os_ostream(*AsmOutStream),
//TODO                              formatted_raw_ostream::DELETE_STREAM);
//TODO  AsmOutFile = new OStream(*AsmOutStream);
//TODO
//TODO  flag_llvm_pch_read = 0;
//TODO
//TODO  if (emit_llvm_bc || emit_llvm)
//TODO    // Disable emission of .ident into the output file... which is completely
//TODO    // wrong for llvm/.bc emission cases.
//TODO    flag_no_ident = 1;
//TODO
//TODO  // If writing to stdout, set binary mode.
//TODO  if (asm_out_file == stdout)
//TODO    sys::Program::ChangeStdoutToBinary();
//TODO
//TODO  AttributeUsedGlobals.clear();
//TODO  timevar_pop(TV_LLVM_INIT);
//TODO}

/// ConvertStructorsList - Convert a list of static ctors/dtors to an
/// initializer suitable for the llvm.global_[cd]tors globals.
static void CreateStructorsList(std::vector<std::pair<Constant*, int> > &Tors,
                                const char *Name) {
  LLVMContext &Context = getGlobalContext();
  
  std::vector<Constant*> InitList;
  std::vector<Constant*> StructInit;
  StructInit.resize(2);
  
  const Type *FPTy =
    Context.getFunctionType(Type::VoidTy, std::vector<const Type*>(), false);
  FPTy = Context.getPointerTypeUnqual(FPTy);
  
  for (unsigned i = 0, e = Tors.size(); i != e; ++i) {
    StructInit[0] = Context.getConstantInt(Type::Int32Ty, Tors[i].second);
    
    // __attribute__(constructor) can be on a function with any type.  Make sure
    // the pointer is void()*.
    StructInit[1] = TheFolder->CreateBitCast(Tors[i].first, FPTy);
    InitList.push_back(Context.getConstantStruct(StructInit, false));
  }
  Constant *Array = Context.getConstantArray(
    Context.getArrayType(InitList[0]->getType(), InitList.size()), InitList);
  new GlobalVariable(*TheModule, Array->getType(), false,
                     GlobalValue::AppendingLinkage,
                     Array, Name);
}

//TODO// llvm_asm_file_end - Finish the .s file.
//TODOvoid llvm_asm_file_end(void) {
//TODO  timevar_push(TV_LLVM_PERFILE);
//TODO  LLVMContext &Context = getGlobalContext();
//TODO
//TODO  performLateBackendInitialization();
//TODO  createPerFunctionOptimizationPasses();
//TODO
//TODO  if (flag_pch_file) {
//TODO    writeLLVMTypesStringTable();
//TODO    writeLLVMValues();
//TODO  }
//TODO
//TODO  // Add an llvm.global_ctors global if needed.
//TODO  if (!StaticCtors.empty())
//TODO    CreateStructorsList(StaticCtors, "llvm.global_ctors");
//TODO  // Add an llvm.global_dtors global if needed.
//TODO  if (!StaticDtors.empty())
//TODO    CreateStructorsList(StaticDtors, "llvm.global_dtors");
//TODO
//TODO  if (!AttributeUsedGlobals.empty()) {
//TODO    std::vector<Constant *> AUGs;
//TODO    const Type *SBP= Context.getPointerTypeUnqual(Type::Int8Ty);
//TODO    for (SmallSetVector<Constant *,32>::iterator AI = AttributeUsedGlobals.begin(),
//TODO           AE = AttributeUsedGlobals.end(); AI != AE; ++AI) {
//TODO      Constant *C = *AI;
//TODO      AUGs.push_back(TheFolder->CreateBitCast(C, SBP));
//TODO    }
//TODO
//TODO    ArrayType *AT = Context.getArrayType(SBP, AUGs.size());
//TODO    Constant *Init = Context.getConstantArray(AT, AUGs);
//TODO    GlobalValue *gv = new GlobalVariable(*TheModule, AT, false,
//TODO                       GlobalValue::AppendingLinkage, Init,
//TODO                       "llvm.used");
//TODO    gv->setSection("llvm.metadata");
//TODO    AttributeUsedGlobals.clear();
//TODO  }
//TODO
//TODO  // Add llvm.global.annotations
//TODO  if (!AttributeAnnotateGlobals.empty()) {
//TODO    Constant *Array = Context.getConstantArray(
//TODO      Context.getArrayType(AttributeAnnotateGlobals[0]->getType(),
//TODO                                      AttributeAnnotateGlobals.size()),
//TODO                       AttributeAnnotateGlobals);
//TODO    GlobalValue *gv = new GlobalVariable(*TheModule, Array->getType(), false,
//TODO                                         GlobalValue::AppendingLinkage, Array,
//TODO                                         "llvm.global.annotations");
//TODO    gv->setSection("llvm.metadata");
//TODO    AttributeAnnotateGlobals.clear();
//TODO  }
//TODO
//TODO  // Finish off the per-function pass.
//TODO  if (PerFunctionPasses)
//TODO    PerFunctionPasses->doFinalization();
//TODO
//TODO  // Emit intermediate file before module level optimization passes are run.
//TODO  if (flag_debug_llvm_module_opt) {
//TODO    
//TODO    static PassManager *IntermediatePM = new PassManager();
//TODO    IntermediatePM->add(new TargetData(*TheTarget->getTargetData()));
//TODO
//TODO    char asm_intermediate_out_filename[MAXPATHLEN];
//TODO    strcpy(&asm_intermediate_out_filename[0], asm_file_name);
//TODO    strcat(&asm_intermediate_out_filename[0],".0");
//TODO    FILE *asm_intermediate_out_file = fopen(asm_intermediate_out_filename, "w+b");
//TODO    AsmIntermediateOutStream = new oFILEstream(asm_intermediate_out_file);
//TODO    AsmIntermediateOutFile = new OStream(*AsmIntermediateOutStream);
//TODO    raw_ostream *AsmIntermediateRawOutStream = 
//TODO      new raw_os_ostream(*AsmIntermediateOutStream);
//TODO    if (emit_llvm_bc)
//TODO      IntermediatePM->add(CreateBitcodeWriterPass(*AsmIntermediateOutStream));
//TODO    if (emit_llvm)
//TODO      IntermediatePM->add(createPrintModulePass(AsmIntermediateRawOutStream));
//TODO    IntermediatePM->run(*TheModule);
//TODO    AsmIntermediateRawOutStream->flush();
//TODO    delete AsmIntermediateRawOutStream;
//TODO    AsmIntermediateRawOutStream = 0;
//TODO    AsmIntermediateOutStream->flush();
//TODO    fflush(asm_intermediate_out_file);
//TODO    delete AsmIntermediateOutStream;
//TODO    AsmIntermediateOutStream = 0;
//TODO    delete AsmIntermediateOutFile;
//TODO    AsmIntermediateOutFile = 0;
//TODO  }
//TODO
//TODO  // Run module-level optimizers, if any are present.
//TODO  createPerModuleOptimizationPasses();
//TODO  if (PerModulePasses)
//TODO    PerModulePasses->run(*TheModule);
//TODO  
//TODO  // Run the code generator, if present.
//TODO  if (CodeGenPasses) {
//TODO    CodeGenPasses->doInitialization();
//TODO    for (Module::iterator I = TheModule->begin(), E = TheModule->end();
//TODO         I != E; ++I)
//TODO      if (!I->isDeclaration())
//TODO        CodeGenPasses->run(*I);
//TODO    CodeGenPasses->doFinalization();
//TODO  }
//TODO
//TODO  AsmOutRawStream->flush();
//TODO  AsmOutStream->flush();
//TODO  fflush(asm_out_file);
//TODO  delete AsmOutRawStream;
//TODO  AsmOutRawStream = 0;
//TODO  delete AsmOutStream;
//TODO  AsmOutStream = 0;
//TODO  delete AsmOutFile;
//TODO  AsmOutFile = 0;
//TODO  timevar_pop(TV_LLVM_PERFILE);
//TODO}
//TODO
//TODO// llvm_call_llvm_shutdown - Release LLVM global state.
//TODOvoid llvm_call_llvm_shutdown(void) {
//TODO  llvm_shutdown();
//TODO}
//TODO
//TODO// llvm_emit_code_for_current_function - Top level interface for emitting a
//TODO// function to the .s file.
//TODOvoid llvm_emit_code_for_current_function(tree fndecl) {
//TODO  if (cfun->nonlocal_goto_save_area)
//TODO    sorry("%Jnon-local gotos not supported by LLVM", fndecl);
//TODO
//TODO  if (errorcount || sorrycount) {
//TODO    TREE_ASM_WRITTEN(fndecl) = 1;
//TODO    return;  // Do not process broken code.
//TODO  }
//TODO  timevar_push(TV_LLVM_FUNCS);
//TODO
//TODO  // Convert the AST to raw/ugly LLVM code.
//TODO  Function *Fn;
//TODO  {
//TODO    TreeToLLVM Emitter(fndecl);
//TODO    enum symbol_visibility vis = DECL_VISIBILITY (fndecl);
//TODO
//TODO    if (vis != VISIBILITY_DEFAULT)
//TODO      // "asm_out.visibility" emits an important warning if we're using a
//TODO      // visibility that's not supported by the target.
//TODO      targetm.asm_out.visibility(fndecl, vis);
//TODO
//TODO    Fn = Emitter.EmitFunction();
//TODO  }
//TODO
//TODO#if 0
//TODO  if (dump_file) {
//TODO    fprintf (dump_file,
//TODO             "\n\n;;\n;; Full LLVM generated for this function:\n;;\n");
//TODO    Fn->dump();
//TODO  }
//TODO#endif
//TODO
//TODO  performLateBackendInitialization();
//TODO  createPerFunctionOptimizationPasses();
//TODO
//TODO  if (PerFunctionPasses)
//TODO    PerFunctionPasses->run(*Fn);
//TODO  
//TODO  // TODO: Nuke the .ll code for the function at -O[01] if we don't want to
//TODO  // inline it or something else.
//TODO  
//TODO  // There's no need to defer outputting this function any more; we
//TODO  // know we want to output it.
//TODO  DECL_DEFER_OUTPUT(fndecl) = 0;
//TODO  
//TODO  // Finally, we have written out this function!
//TODO  TREE_ASM_WRITTEN(fndecl) = 1;
//TODO  timevar_pop(TV_LLVM_FUNCS);
//TODO}

// emit_alias_to_llvm - Given decl and target emit alias to target.
void emit_alias_to_llvm(tree decl, tree target, tree target_decl) {
  if (errorcount || sorrycount) {
    TREE_ASM_WRITTEN(decl) = 1;
    return;  // Do not process broken code.
  }
  
  LLVMContext &Context = getGlobalContext();

//TODO  timevar_push(TV_LLVM_GLOBALS);

  // Get or create LLVM global for our alias.
  GlobalValue *V = cast<GlobalValue>(DECL_LLVM(decl));
  
  GlobalValue *Aliasee = NULL;
  
  if (target_decl)
    Aliasee = cast<GlobalValue>(DECL_LLVM(target_decl));
  else {
    // This is something insane. Probably only LTHUNKs can be here
    // Try to grab decl from IDENTIFIER_NODE

    // Query SymTab for aliasee
    const char* AliaseeName = IDENTIFIER_POINTER(target);
    Aliasee =
      dyn_cast_or_null<GlobalValue>(TheModule->
                                    getValueSymbolTable().lookup(AliaseeName));

    // Last resort. Query for name set via __asm__
    if (!Aliasee) {
      std::string starred = std::string("\001") + AliaseeName;
      Aliasee =
        dyn_cast_or_null<GlobalValue>(TheModule->
                                      getValueSymbolTable().lookup(starred));
    }
    
    if (!Aliasee) {
      if (lookup_attribute ("weakref", DECL_ATTRIBUTES (decl))) {
        if (GlobalVariable *GV = dyn_cast<GlobalVariable>(V))
          Aliasee = new GlobalVariable(*TheModule, GV->getType(),
                                       GV->isConstant(),
                                       GlobalVariable::ExternalWeakLinkage,
                                       NULL, AliaseeName);
        else if (Function *F = dyn_cast<Function>(V))
          Aliasee = Function::Create(F->getFunctionType(),
                                     Function::ExternalWeakLinkage,
                                     AliaseeName, TheModule);
        else
          assert(0 && "Unsuported global value");
      } else {
        error ("%J%qD aliased to undefined symbol %qs", decl, decl, AliaseeName);
//TODO        timevar_pop(TV_LLVM_GLOBALS);
        return;
      }
    }
  }

  GlobalValue::LinkageTypes Linkage;

  // A weak alias has TREE_PUBLIC set but not the other bits.
  if (false)//FIXME DECL_LLVM_PRIVATE(decl))
    Linkage = GlobalValue::PrivateLinkage;
  else if (DECL_WEAK(decl))
    // The user may have explicitly asked for weak linkage - ignore flag_odr.
    Linkage = GlobalValue::WeakAnyLinkage;
  else if (!TREE_PUBLIC(decl))
    Linkage = GlobalValue::InternalLinkage;
  else
    Linkage = GlobalValue::ExternalLinkage;

  GlobalAlias* GA = new GlobalAlias(Aliasee->getType(), Linkage, "",
                                    Aliasee, TheModule);

  handleVisibility(decl, GA);

  if (GA->getType()->canLosslesslyBitCastTo(V->getType()))
    V->replaceAllUsesWith(Context.getConstantExprBitCast(GA, V->getType()));
  else if (!V->use_empty()) {
    error ("%J Alias %qD used with invalid type!", decl, decl);
//TODO    timevar_pop(TV_LLVM_GLOBALS);
    return;
  }
    
  changeLLVMConstant(V, GA);
  GA->takeName(V);
  if (GlobalVariable *GV = dyn_cast<GlobalVariable>(V))
    GV->eraseFromParent();
  else if (GlobalAlias *GA = dyn_cast<GlobalAlias>(V))
    GA->eraseFromParent();
  else if (Function *F = dyn_cast<Function>(V))
    F->eraseFromParent();
  else
    assert(0 && "Unsuported global value");

  TREE_ASM_WRITTEN(decl) = 1;
  
//TODO  timevar_pop(TV_LLVM_GLOBALS);
  return;
}

// Convert string to global value. Use existing global if possible.
Constant* ConvertMetadataStringToGV(const char *str) {
  
  Constant *Init = getGlobalContext().getConstantArray(std::string(str));

  // Use cached string if it exists.
  static std::map<Constant*, GlobalVariable*> StringCSTCache;
  GlobalVariable *&Slot = StringCSTCache[Init];
  if (Slot) return Slot;
  
  // Create a new string global.
  GlobalVariable *GV = new GlobalVariable(*TheModule, Init->getType(), true,
                                          GlobalVariable::PrivateLinkage,
                                          Init, ".str");
  GV->setSection("llvm.metadata");
  Slot = GV;
  return GV;
  
}

/// AddAnnotateAttrsToGlobal - Adds decls that have a
/// annotate attribute to a vector to be emitted later.
void AddAnnotateAttrsToGlobal(GlobalValue *GV, tree decl) {
  LLVMContext &Context = getGlobalContext();
  
  // Handle annotate attribute on global.
  tree annotateAttr = lookup_attribute("annotate", DECL_ATTRIBUTES (decl));
  if (annotateAttr == 0)
    return;
  
  // Get file and line number
  Constant *lineNo =
    Context.getConstantInt(Type::Int32Ty, DECL_SOURCE_LINE(decl));
  Constant *file = ConvertMetadataStringToGV(DECL_SOURCE_FILE(decl));
  const Type *SBP= Context.getPointerTypeUnqual(Type::Int8Ty);
  file = TheFolder->CreateBitCast(file, SBP);
 
  // There may be multiple annotate attributes. Pass return of lookup_attr 
  //  to successive lookups.
  while (annotateAttr) {
    
    // Each annotate attribute is a tree list.
    // Get value of list which is our linked list of args.
    tree args = TREE_VALUE(annotateAttr);
    
    // Each annotate attribute may have multiple args.
    // Treat each arg as if it were a separate annotate attribute.
    for (tree a = args; a; a = TREE_CHAIN(a)) {
      // Each element of the arg list is a tree list, so get value
      tree val = TREE_VALUE(a);
      
      // Assert its a string, and then get that string.
      assert(TREE_CODE(val) == STRING_CST && 
             "Annotate attribute arg should always be a string");
      Constant *strGV = TreeConstantToLLVM::EmitLV_STRING_CST(val);
      Constant *Element[4] = {
        TheFolder->CreateBitCast(GV,SBP),
        TheFolder->CreateBitCast(strGV,SBP),
        file,
        lineNo
      };
 
      AttributeAnnotateGlobals.push_back(
        Context.getConstantStruct(Element, 4, false));
    }
      
    // Get next annotate attribute.
    annotateAttr = TREE_CHAIN(annotateAttr);
    if (annotateAttr)
      annotateAttr = lookup_attribute("annotate", annotateAttr);
  }
}

/// reset_initializer_llvm - Change the initializer for a global variable.
void reset_initializer_llvm(tree decl) {
  // If there were earlier errors we can get here when DECL_LLVM has not
  // been set.  Don't crash.
  // We can also get here when DECL_LLVM has not been set for some object
  // referenced in the initializer.  Don't crash then either.
  if (errorcount || sorrycount)
    return;

  // Get or create the global variable now.
  GlobalVariable *GV = cast<GlobalVariable>(DECL_LLVM(decl));
  
  // Visibility may also have changed.
  handleVisibility(decl, GV);

  // Convert the initializer over.
  Constant *Init = TreeConstantToLLVM::Convert(DECL_INITIAL(decl));

  // Set the initializer.
  GV->setInitializer(Init);
}
  
/// reset_type_and_initializer_llvm - Change the type and initializer for 
/// a global variable.
void reset_type_and_initializer_llvm(tree decl) {
  // If there were earlier errors we can get here when DECL_LLVM has not
  // been set.  Don't crash.
  // We can also get here when DECL_LLVM has not been set for some object
  // referenced in the initializer.  Don't crash then either.
  LLVMContext &Context = getGlobalContext();
  
  if (errorcount || sorrycount)
    return;

  // Get or create the global variable now.
  GlobalVariable *GV = cast<GlobalVariable>(DECL_LLVM(decl));
  
  // Visibility may also have changed.
  handleVisibility(decl, GV);

  // Temporary to avoid infinite recursion (see comments emit_global_to_llvm)
  GV->setInitializer(Context.getUndef(GV->getType()->getElementType()));

  // Convert the initializer over.
  Constant *Init = TreeConstantToLLVM::Convert(DECL_INITIAL(decl));

  // If we had a forward definition that has a type that disagrees with our
  // initializer, insert a cast now.  This sort of thing occurs when we have a
  // global union, and the LLVM type followed a union initializer that is
  // different from the union element used for the type.
  if (GV->getType()->getElementType() != Init->getType()) {
    GV->removeFromParent();
    GlobalVariable *NGV = new GlobalVariable(*TheModule, Init->getType(), 
                                             GV->isConstant(),
                                             GV->getLinkage(), 0,
                                             GV->getName());
    NGV->setVisibility(GV->getVisibility());
    NGV->setSection(GV->getSection());
    NGV->setAlignment(GV->getAlignment());
    NGV->setLinkage(GV->getLinkage());
    GV->replaceAllUsesWith(TheFolder->CreateBitCast(NGV, GV->getType()));
    changeLLVMConstant(GV, NGV);
    delete GV;
    SET_DECL_LLVM(decl, NGV);
    GV = NGV;
  }

  // Set the initializer.
  GV->setInitializer(Init);
}
  
/// emit_global_to_llvm - Emit the specified VAR_DECL or aggregate CONST_DECL to
/// LLVM as a global variable.  This function implements the end of
/// assemble_variable.
void emit_global_to_llvm(tree decl) {
  if (errorcount || sorrycount) {
    TREE_ASM_WRITTEN(decl) = 1;
    return;  // Do not process broken code.
  }

  // FIXME: Support alignment on globals: DECL_ALIGN.
  // FIXME: DECL_PRESERVE_P indicates the var is marked with attribute 'used'.

  // Global register variables don't turn into LLVM GlobalVariables.
  if (TREE_CODE(decl) == VAR_DECL && DECL_REGISTER(decl))
    return;

  // If tree nodes says defer output then do not emit global yet.
  if (CODE_CONTAINS_STRUCT (TREE_CODE (decl), TS_DECL_WITH_VIS) 
      && (DECL_DEFER_OUTPUT(decl)))
      return;

  // If we encounter a forward declaration then do not emit the global yet.
  if (!TYPE_SIZE(TREE_TYPE(decl)))
    return;

  LLVMContext &Context = getGlobalContext();

//TODO  timevar_push(TV_LLVM_GLOBALS);

  // Get or create the global variable now.
  GlobalVariable *GV = cast<GlobalVariable>(DECL_LLVM(decl));
  
  // Convert the initializer over.
  Constant *Init;
  if (DECL_INITIAL(decl) == 0 || DECL_INITIAL(decl) == error_mark_node) {
    // This global should be zero initialized.  Reconvert the type in case the
    // forward def of the global and the real def differ in type (e.g. declared
    // as 'int A[]', and defined as 'int A[100]').
    Init = getGlobalContext().getNullValue(ConvertType(TREE_TYPE(decl)));
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
    GV->setInitializer(Context.getUndef(GV->getType()->getElementType()));
    Init = TreeConstantToLLVM::Convert(DECL_INITIAL(decl));
  }

  // If we had a forward definition that has a type that disagrees with our
  // initializer, insert a cast now.  This sort of thing occurs when we have a
  // global union, and the LLVM type followed a union initializer that is
  // different from the union element used for the type.
  if (GV->getType()->getElementType() != Init->getType()) {
    GV->removeFromParent();
    GlobalVariable *NGV = new GlobalVariable(*TheModule, Init->getType(),
                                             GV->isConstant(),
                                             GlobalValue::ExternalLinkage, 0,
                                             GV->getName());
    GV->replaceAllUsesWith(TheFolder->CreateBitCast(NGV, GV->getType()));
    changeLLVMConstant(GV, NGV);
    delete GV;
    SET_DECL_LLVM(decl, NGV);
    GV = NGV;
  }
 
  // Set the initializer.
  GV->setInitializer(Init);

  // Set thread local (TLS)
  if (TREE_CODE(decl) == VAR_DECL && DECL_THREAD_LOCAL_P(decl))
    GV->setThreadLocal(true);

  // Set the linkage.
  GlobalValue::LinkageTypes Linkage = GV->getLinkage();
  if (CODE_CONTAINS_STRUCT (TREE_CODE (decl), TS_DECL_WITH_VIS)
      && false) {// FIXME DECL_LLVM_PRIVATE(decl)) {
    Linkage = GlobalValue::PrivateLinkage;
  } else if (!TREE_PUBLIC(decl)) {
    Linkage = GlobalValue::InternalLinkage;
  } else if (DECL_WEAK(decl)) {
    // The user may have explicitly asked for weak linkage - ignore flag_odr.
    Linkage = GlobalValue::WeakAnyLinkage;
  } else if (DECL_ONE_ONLY(decl)) {
    Linkage = GlobalValue::getWeakLinkage(flag_odr);
  } else if (DECL_COMMON(decl) &&  // DECL_COMMON is only meaningful if no init
             (!DECL_INITIAL(decl) || DECL_INITIAL(decl) == error_mark_node)) {
    // llvm-gcc also includes DECL_VIRTUAL_P here.
    Linkage = GlobalValue::CommonLinkage;
  } else if (DECL_COMDAT(decl)) {
    Linkage = GlobalValue::getLinkOnceLinkage(flag_odr);
  }

  // Allow loads from constants to be folded even if the constant has weak
  // linkage.  Do this by giving the constant weak_odr linkage rather than
  // weak linkage.  It is not clear whether this optimization is valid (see
  // gcc bug 36685), but mainline gcc chooses to do it, and fold may already
  // have done it, so we might as well join in with gusto.
  if (GV->isConstant()) {
    if (Linkage == GlobalValue::WeakAnyLinkage)
      Linkage = GlobalValue::WeakODRLinkage;
    else if (Linkage == GlobalValue::LinkOnceAnyLinkage)
      Linkage = GlobalValue::LinkOnceODRLinkage;
  }
  GV->setLinkage(Linkage);

#ifdef TARGET_ADJUST_LLVM_LINKAGE
  TARGET_ADJUST_LLVM_LINKAGE(GV, decl);
#endif /* TARGET_ADJUST_LLVM_LINKAGE */

  handleVisibility(decl, GV);

  // Set the section for the global.
  if (TREE_CODE(decl) == VAR_DECL) {
    if (DECL_SECTION_NAME(decl)) {
      GV->setSection(TREE_STRING_POINTER(DECL_SECTION_NAME(decl)));
#ifdef LLVM_IMPLICIT_TARGET_GLOBAL_VAR_SECTION
    } else if (const char *Section = 
                LLVM_IMPLICIT_TARGET_GLOBAL_VAR_SECTION(decl)) {
      GV->setSection(Section);
#endif
    }
    
    // Set the alignment for the global if one of the following condition is met
    // 1) DECL_ALIGN is better than the alignment as per ABI specification
    // 2) DECL_ALIGN is set by user.
    if (DECL_ALIGN(decl)) {
      unsigned TargetAlign =
        getTargetData().getABITypeAlignment(GV->getType()->getElementType());
      if (DECL_USER_ALIGN(decl) ||
          8 * TargetAlign < (unsigned)DECL_ALIGN(decl))
        GV->setAlignment(DECL_ALIGN(decl) / 8);
    }

    // Handle used decls
    if (DECL_PRESERVE_P (decl))
      AttributeUsedGlobals.insert(GV);
  
    // Add annotate attributes for globals
    if (DECL_ATTRIBUTES(decl))
      AddAnnotateAttrsToGlobal(GV, decl);
  
#ifdef LLVM_IMPLICIT_TARGET_GLOBAL_VAR_SECTION
  } else if (TREE_CODE(decl) == CONST_DECL) {
    if (const char *Section = 
        LLVM_IMPLICIT_TARGET_GLOBAL_VAR_SECTION(decl)) {
      GV->setSection(Section);

      /* LLVM LOCAL - begin radar 6389998 */
#ifdef TARGET_ADJUST_CFSTRING_NAME
      TARGET_ADJUST_CFSTRING_NAME(GV, Section);
#endif
      /* LLVM LOCAL - end radar 6389998 */
    }
#endif
  }

  // No debug info for globals when optimization is on.  While this is
  // something that would be accurate and useful to a user, it currently
  // affects some optimizations that, e.g., count uses.
  if (TheDebugInfo && !optimize)
    TheDebugInfo->EmitGlobalVariable(GV, decl);

  TREE_ASM_WRITTEN(decl) = 1;
//TODO  timevar_pop(TV_LLVM_GLOBALS);
}


/// ValidateRegisterVariable - Check that a static "asm" variable is
/// well-formed.  If not, emit error messages and return true.  If so, return
/// false.
bool ValidateRegisterVariable(tree decl) {
  LLVMContext &Context = getGlobalContext();
  int RegNumber = decode_reg_name(extractRegisterName(decl));
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
  else if (!Ty->isSingleValueType())
    sorry("%JLLVM cannot handle register variable %qD, report a bug",
          decl, decl);
  else {
    if (TREE_THIS_VOLATILE(decl))
      warning(0, "volatile register variables don%'t work as you might wish");
    
    SET_DECL_LLVM(decl, Context.getConstantIntFalse());
    return false;  // Everything ok.
  }
  SET_DECL_LLVM(decl, Context.getConstantIntTrue());
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

  LLVMContext &Context = getGlobalContext();
  
  // For a duplicate declaration, we can be called twice on the
  // same DECL node.  Don't discard the LLVM already made.
  if (DECL_LLVM_SET_P(decl)) return;

  if (errorcount || sorrycount)
    return;  // Do not process broken code.
  
  
  // Global register variable with asm name, e.g.:
  // register unsigned long esp __asm__("ebp");
  if (TREE_CODE(decl) != FUNCTION_DECL && DECL_REGISTER(decl)) {
    // This  just verifies that the variable is ok.  The actual "load/store"
    // code paths handle accesses to the variable.
    ValidateRegisterVariable(decl);
    return;
  }
  
//TODO  timevar_push(TV_LLVM_GLOBALS);

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
    Function *FnEntry = TheModule->getFunction(Name);
    if (FnEntry == 0) {
      unsigned CC;
      AttrListPtr PAL;
      const FunctionType *Ty = 
        TheTypeConverter->ConvertFunctionType(TREE_TYPE(decl), decl, NULL,
                                              CC, PAL);
      FnEntry = Function::Create(Ty, Function::ExternalLinkage, Name, TheModule);
      FnEntry->setCallingConv(CC);
      FnEntry->setAttributes(PAL);

      // Check for external weak linkage.
      if (DECL_EXTERNAL(decl) && DECL_WEAK(decl))
        FnEntry->setLinkage(Function::ExternalWeakLinkage);

#ifdef TARGET_ADJUST_LLVM_LINKAGE
      TARGET_ADJUST_LLVM_LINKAGE(FnEntry,decl);
#endif /* TARGET_ADJUST_LLVM_LINKAGE */

      handleVisibility(decl, FnEntry);

      // If FnEntry got renamed, then there is already an object with this name
      // in the symbol table.  If this happens, the old one must be a forward
      // decl, just replace it with a cast of the new one.
      if (FnEntry->getName() != Name) {
        GlobalVariable *G = TheModule->getGlobalVariable(Name, true);
        assert(G && G->isDeclaration() && "A global turned into a function?");

        // Replace any uses of "G" with uses of FnEntry.
        Constant *GInNewType = TheFolder->CreateBitCast(FnEntry, G->getType());
        G->replaceAllUsesWith(GInNewType);

        // Update the decl that points to G.
        changeLLVMConstant(G, GInNewType);

        // Now we can give GV the proper name.
        FnEntry->takeName(G);
        
        // G is now dead, nuke it.
        G->eraseFromParent();
      }
    }
    SET_DECL_LLVM(decl, FnEntry);
  } else {
    assert((TREE_CODE(decl) == VAR_DECL ||
            TREE_CODE(decl) == CONST_DECL) && "Not a function or var decl?");
    const Type *Ty = ConvertType(TREE_TYPE(decl));
    GlobalVariable *GV ;

    // If we have "extern void foo", make the global have type {} instead of
    // type void.
    if (Ty == Type::VoidTy) Ty = Context.getStructType(NULL, NULL);

    if (Name[0] == 0) {   // Global has no name.
      GV = new GlobalVariable(*TheModule, Ty, false, 
                              GlobalValue::ExternalLinkage, 0, "");

      // Check for external weak linkage.
      if (DECL_EXTERNAL(decl) && DECL_WEAK(decl))
        GV->setLinkage(GlobalValue::ExternalWeakLinkage);

#ifdef TARGET_ADJUST_LLVM_LINKAGE
      TARGET_ADJUST_LLVM_LINKAGE(GV,decl);
#endif /* TARGET_ADJUST_LLVM_LINKAGE */

      handleVisibility(decl, GV);
    } else {
      // If the global has a name, prevent multiple vars with the same name from
      // being created.
      GlobalVariable *GVE = TheModule->getGlobalVariable(Name, true);
    
      if (GVE == 0) {
        GV = new GlobalVariable(*TheModule, Ty, false,
                                GlobalValue::ExternalLinkage, 0, Name);

        // Check for external weak linkage.
        if (DECL_EXTERNAL(decl) && DECL_WEAK(decl))
          GV->setLinkage(GlobalValue::ExternalWeakLinkage);

#ifdef TARGET_ADJUST_LLVM_LINKAGE
        TARGET_ADJUST_LLVM_LINKAGE(GV,decl);
#endif /* TARGET_ADJUST_LLVM_LINKAGE */

	handleVisibility(decl, GV);

        // If GV got renamed, then there is already an object with this name in
        // the symbol table.  If this happens, the old one must be a forward
        // decl, just replace it with a cast of the new one.
        if (GV->getName() != Name) {
          Function *F = TheModule->getFunction(Name);
          assert(F && F->isDeclaration() && "A function turned into a global?");
          
          // Replace any uses of "F" with uses of GV.
          Constant *FInNewType = TheFolder->CreateBitCast(GV, F->getType());
          F->replaceAllUsesWith(FInNewType);

          // Update the decl that points to F.
          changeLLVMConstant(F, FInNewType);

          // Now we can give GV the proper name.
          GV->takeName(F);
          
          // F is now dead, nuke it.
          F->eraseFromParent();
        }
        
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

    // Set thread local (TLS)
    if (TREE_CODE(decl) == VAR_DECL && DECL_THREAD_LOCAL_P(decl))
      GV->setThreadLocal(true);

    SET_DECL_LLVM(decl, GV);
  }
//TODO  timevar_pop(TV_LLVM_GLOBALS);
}

/// llvm_get_decl_name - Used by varasm.c, returns the specified declaration's
/// name.
const char *llvm_get_decl_name(void *LLVM) {
  if (LLVM)
    if (const ValueName *VN = ((Value*)LLVM)->getValueName())
      return VN->getKeyData();
  return "";
}

// llvm_mark_decl_weak - Used by varasm.c, called when a decl is found to be
// weak, but it already had an llvm object created for it. This marks the LLVM
// object weak as well.
void llvm_mark_decl_weak(tree decl) {
  assert(DECL_LLVM_SET_P(decl) && DECL_WEAK(decl) &&
         isa<GlobalValue>(DECL_LLVM(decl)) && "Decl isn't marked weak!");
  GlobalValue *GV = cast<GlobalValue>(DECL_LLVM(decl));

  // Do not mark something that is already known to be linkonce or internal.
  // The user may have explicitly asked for weak linkage - ignore flag_odr.
  if (GV->hasExternalLinkage()) {
    GlobalValue::LinkageTypes Linkage;
    if (GV->isDeclaration()) {
      Linkage = GlobalValue::ExternalWeakLinkage;
    } else {
      Linkage = GlobalValue::WeakAnyLinkage;
      // Allow loads from constants to be folded even if the constant has weak
      // linkage.  Do this by giving the constant weak_odr linkage rather than
      // weak linkage.  It is not clear whether this optimization is valid (see
      // gcc bug 36685), but mainline gcc chooses to do it, and fold may already
      // have done it, so we might as well join in with gusto.
      if (GlobalVariable *GVar = dyn_cast<GlobalVariable>(GV))
        if (GVar->isConstant())
          Linkage = GlobalValue::WeakODRLinkage;
    }
    GV->setLinkage(Linkage);
  }
}

// llvm_emit_ctor_dtor - Called to emit static ctors/dtors to LLVM code.  fndecl
// is a 'void()' FUNCTION_DECL for the code, initprio is the init priority, and
// isCtor indicates whether this is a ctor or dtor.
//
void llvm_emit_ctor_dtor(tree FnDecl, int InitPrio, int isCtor) {
  mark_decl_referenced(FnDecl);  // Inform cgraph that we used the global.

  if (errorcount || sorrycount) return;

  Constant *C = cast<Constant>(DECL_LLVM(FnDecl));
  (isCtor ? &StaticCtors:&StaticDtors)->push_back(std::make_pair(C, InitPrio));
}

void llvm_emit_typedef(tree decl) {
  // Need hooks for debug info?
  return;
}

// llvm_emit_file_scope_asm - Emit the specified string as a file-scope inline
// asm block.
//
void llvm_emit_file_scope_asm(const char *string) {
  if (TheModule->getModuleInlineAsm().empty())
    TheModule->setModuleInlineAsm(string);
  else
    TheModule->setModuleInlineAsm(TheModule->getModuleInlineAsm() + "\n" +
                                  string);
}

//FIXME// print_llvm - Print the specified LLVM chunk like an operand, called by
//FIXME// print-tree.c for tree dumps.
//FIXME//
//FIXMEvoid print_llvm(FILE *file, void *LLVM) {
//FIXME  oFILEstream FS(file);
//FIXME  FS << "LLVM: ";
//FIXME  WriteAsOperand(FS, (Value*)LLVM, true, TheModule);
//FIXME}
//FIXME
//FIXME// print_llvm_type - Print the specified LLVM type symbolically, called by
//FIXME// print-tree.c for tree dumps.
//FIXME//
//FIXMEvoid print_llvm_type(FILE *file, void *LLVM) {
//FIXME  oFILEstream FS(file);
//FIXME  FS << "LLVM: ";
//FIXME  
//FIXME  // FIXME: oFILEstream can probably be removed in favor of a new raw_ostream
//FIXME  // adaptor which would be simpler and more efficient.  In the meantime, just
//FIXME  // adapt the adaptor.
//FIXME  raw_os_ostream RO(FS);
//FIXME  WriteTypeSymbolic(RO, (const Type*)LLVM, TheModule);
//FIXME}

// Get a register name given its decl.  In 4.2 unlike 4.0 these names
// have been run through set_user_assembler_name which means they may
// have a leading \1 at this point; compensate.

const char* extractRegisterName(tree decl) {
  const char* Name = IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(decl));
  return (*Name==1) ? Name+1 : Name;
}
