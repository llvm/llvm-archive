//===-- insert.cpp - Insert SAFECode Run-time Checks ----------------------===//
//
//                     SAFECode
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the insertion of SAFECode's run-time checks.
//
//===----------------------------------------------------------------------===//

#include "safecode/Config/config.h"
#include "InsertPoolChecks.h"
#include "SCUtils.h"
#include "llvm/Instruction.h"
#include "llvm/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/ADT/VectorExtras.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ConstantRange.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/LoopInfo.h" 
#include <iostream>
#include <vector>
#include <set>

using namespace llvm;

extern Value *getRepresentativeMetaPD(Value *);

RegisterPass<PreInsertPoolChecks> pipc ("presafecode", "prepare for SAFECode");
RegisterPass<InsertPoolChecks>     ipc ("safecode",    "insert runtime checks");

cl::opt<bool> EnableSplitChecks  ("enable-splitchecks", cl::Hidden,
                                  cl::init(false),
                                  cl::desc("Split lookup and checks"));

cl::opt<bool> InsertPoolChecksForArrays("boundschecks-usepoolchecks",
                cl::Hidden, cl::init(false),
                cl::desc("Insert pool checks instead of exact bounds checks"));
  
// Options for Enabling/Disabling the Insertion of Various Checks
cl::opt<bool> EnableUnknownChecks ("enable-unknownchecks", cl::Hidden,
                                   cl::init(false),
                                   cl::desc("Enable Checks on Incomplete/Unknown Nodes"));

cl::opt<bool> EnableNullChecks  ("enable-nullchecks", cl::Hidden,
                                cl::init(false),
                                cl::desc("Enable Checks on NULL Pools"));


cl::opt<bool> DisableRegisterGlobals ("disable-regglobals", cl::Hidden,
                                cl::init(false),
                                cl::desc("Do not register globals"));

cl::opt<bool> DisableLSChecks  ("disable-lschecks", cl::Hidden,
                                cl::init(false),
                                cl::desc("Disable Load/Store Checks"));

cl::opt<bool> DisableGEPChecks ("disable-gepchecks", cl::Hidden,
                                cl::init(false),
                                cl::desc("Disable GetElementPtr(GEP) Checks"));

cl::opt<bool> DisableStackChecks ("disable-stackchecks", cl::Hidden,
                                  cl::init(false),
                                  cl::desc("Disable Stack Checks"));

cl::opt<bool> DisableFuncChecks ("disable-funcchecks", cl::Hidden,
                                  cl::init(false),
                                  cl::desc("Disable Function Call Checks"));

cl::opt<bool> DisableIntrinsicChecks ("disable-intrinchecks", cl::Hidden,
                                      cl::init(false),
                                      cl::desc("Disable Intrinsic Checks"));

cl::opt<bool> EnableMonotonicOptimisation("enable-monotonic", cl::Hidden,
                                          cl::init(false),
                   cl::desc("Enable LICM for bounds checks on monotonic loops"));

// Options for where to insert various initialization code
cl::opt<string> InitFunctionName ("initfunc",
                                  cl::desc("Specify name of initialization "
                                           "function"),
                                  cl::value_desc("function name"));

// Pass Statistics
static Statistic<> NullChecks ("safecode",
                               "Poolchecks with NULL pool descriptor");
static Statistic<> FullChecks ("safecode",
                               "Poolchecks with non-NULL pool descriptor");
static Statistic<> MissChecks ("safecode",
                               "Poolchecks omitted due to bad pool descriptor");
static Statistic<> PoolChecks ("safecode", "Poolchecks Added");

static Statistic<> FuncChecks ("safecode", "Indirect Call Checks Added");
static Statistic<> MissedFuncChecks ("safecode", "Indirect Call Checks Missed");
static Statistic<> SavedPoolChecks ("safecode", "Pool Checks Performed on Checked Pointers");
static Statistic<> AlignChecks ("safecode", "Number of alignment checks required");
static Statistic<> AlignLSChecks ("safecode", "Number of alignment on load/store checks required");

// Bounds Check Statistics
static Statistic<> BoundsChecks     ("safecode",
                                     "Total bounds checks inserted");
static Statistic<> IBoundsChecks    ("safecode",
                                     "Bounds checks with incomplete DSNode");
static Statistic<> UBoundsChecks    ("safecode",
                                     "Bounds checks with unknown DSNode");
static Statistic<> ABoundsChecks    ("safecode",
                                     "Bounds checks with stack DSNode");
static Statistic<> NullBoundsChecks ("safecode",
                                     "Missed bounds checks - NULL pool handle");
static Statistic<> NoSHGBoundsChecks ("safecode",
                                      "Missed bounds checks - no SHG DSNode");


static Statistic<> MissedIncompleteChecks ("safecode",
                               "Poolchecks missed because of incompleteness");
static Statistic<> MissedMultDimArrayChecks ("safecode",
                                             "Multi-dimensional array checks");

static Statistic<> MissedGlobalChecks ("safecode", "Missed global checks");
static Statistic<> MissedNullChecks   ("safecode", "Missed PD checks");

// Exact Check Statistics
static Statistic<> ExactChecks        ("safecode", "Exactchecks inserted");
static Statistic<> ConstExactChecks   ("safecode", "Omitted Exactchecks with constant arguments");
static Statistic<> ZeroFuncChecks     ("safecode", "Indirect Call Checks with Zero Targets");
 
// Object registration statistics
static Statistic<> StackRegisters     ("safecode", "Stack registrations");
static Statistic<> SavedRegAllocs     ("safecode", "Stack registrations avoided");

// Other statistics
static Statistic<> StructGEPsRemoved  ("safecode", "Structure GEP Checks Removed");

//MonotonicOpts
static Statistic<> MonotonicOpts  ("safecode", "Number of monotonic LICM bounds check optimisations");

// The set of values that already have run-time checks
static std::set<Value *> CheckedValues;

////////////////////////////////////////////////////////////////////////////
// Static Functions
////////////////////////////////////////////////////////////////////////////

static inline bool
isNodeRegistered (DSNode * Node) {
  // Do not perform checks on the pointer if its DSNode does not have a known
  // allocation site.
  if (!((Node->isAllocaNode()) ||
        (Node->isHeapNode())   ||
        (Node->isGlobalNode()))) {
    ++NoSHGBoundsChecks;
    return false;
  }

  return true;
}

//Do not replace these with check results
static std::set<Value*> AddedValues;

static GlobalVariable*
makeMetaPool(Module* M, DSNode* N) {
  //Here we insert a global meta pool
  //Now create a meta pool for this value, DSN Node
  const Type * VoidPtrType = PointerType::get(Type::SByteTy);
  std::vector<const Type*> MPTV;
  for (int x = 0; x < 9; ++x)
    MPTV.push_back(VoidPtrType);
  // Add the types for the hit cache
  MPTV.push_back(Type::UIntTy);
  MPTV.push_back(ArrayType::get (Type::UIntTy, 4));
  MPTV.push_back(ArrayType::get (Type::UIntTy, 4));
  MPTV.push_back(ArrayType::get (VoidPtrType, 4));

  const StructType* MPT = StructType::get(MPTV);

  static int x = 0;
  std::string Name = "_metaPool_";
 
  unsigned Flags = N ? N->getMP()->getFlags() : 0;
  if(Flags & DSNode::Incomplete)
    Name += "I";
  if(Flags & DSNode::UnknownNode)
    Name += "U";
  if(Flags & DSNode::AllocaNode)
    Name += "A";
  if(Flags & DSNode::GlobalNode)
    Name += "G";
  if(Flags & DSNode::HeapNode)
    Name += "H";
  if( N && N->isNodeCompletelyFolded())
    Name += "F";
  Name += "_";
  char c[100];
  snprintf(c, sizeof(c), "%d", x);
  Name += c;
  Name += "_";
  ++x;

  return new GlobalVariable(
                            /*type=*/ MPT,
                            /*isConstant=*/ false,
                            /*Linkage=*/ GlobalValue::InternalLinkage,
                            /*initializer=*/ Constant::getNullValue(MPT),
                            /*name=*/ Name,
                            /*parent=*/ M );
}

#ifndef LLVA_KERNEL
Value *
PreInsertPoolChecks::getPoolHandle(const Value *V, Function *F, PA::FuncInfo &FI, bool collapsed) {
  DSGraph &TDG = getDSGraph(*F);
  const DSNode *Node  = TDG.getNodeForValue((Value *)V).getNode();
  // Get the pool handle for this DSNode...
  //  assert(!Node->isUnknownNode() && "Unknown node \n");
  Type *VoidPtrType = PointerType::get(Type::SByteTy); 
  Type *PoolDescType = ArrayType::get(VoidPtrType, 50);
  Type *PoolDescPtrTy = PointerType::get(PoolDescType);
  if (!Node) {
    return 0; //0 means there is no isse with the value, otherwise there will be a callnode
  }
  if (Node->isUnknownNode()) {
    //FIXME this should be in a top down pass or propagated like collapsed pools below 
    if (!collapsed) {
      unsigned offset = TDG.getNodeForValue((Value *)V).getOffset();
      assert((!offset) && " we don't handle middle of structs yet\n");
      return Constant::getNullValue(PoolDescPtrTy);
    }
  }
  std::map<const DSNode*, Value*>::iterator I = FI.PoolDescriptors.find(Node);
  map <Function *, set<Value *> > &
    CollapsedPoolPtrs = efPass->CollapsedPoolPtrs;
  
  if (I != FI.PoolDescriptors.end()) {
    // Check that the node pointed to by V in the TD DS graph is not
    // collapsed
    
    if (!collapsed && CollapsedPoolPtrs.count(F)) {
      Value *v = I->second;
      if (CollapsedPoolPtrs[F].find(I->second) != CollapsedPoolPtrs[F].end()) {
#ifdef DEBUG
        std::cerr << "Collapsed pools \n";
#endif
        return Constant::getNullValue(PoolDescPtrTy);
      } else {
        return v;
      }
    } else {
      return I->second;
    } 
  }
  return 0;
}
#endif

Value *
PreInsertPoolChecks::createPoolHandle (Module & M, DSNode * Node) {
  // If there is no node, return NULL.
  if (!Node) return 0;

  // Get the DSNode's MetaPool.  If it doesn't have one, return NULL.
  MetaPool * MP = Node->getMP();
  if (!MP) return 0;

  // If the MetaPool global variable has already been created, then simply
  // return it.  Otherwise, create one.
  if (!(MP->getMetaPoolValue()))
    MP->setMetaPoolValue (makeMetaPool (&M, Node));  
  return (MP->getMetaPoolValue());
}


Value *
PreInsertPoolChecks::createPoolHandle (const Value * V, Function * F) {
  //
  // Get the DSNode from the Top Down DSGraph.
  //
  DSGraph &TDG =  getDSGraph(*F);
  DSNode *Node = TDG.getNodeForValue((Value *)V).getNode();

  // If there is no node, return NULL.
  if (!Node) return 0;

  // Get the DSNode's MetaPool.  If it doesn't have one, return NULL.
  MetaPool * MP = Node->getMP();
  if (!MP) return 0;

  // If the MetaPool global variable has already been created, then simply
  // return it.  Otherwise, create one.
  if (!(MP->getMetaPoolValue()))
    MP->setMetaPoolValue (makeMetaPool (F->getParent(), Node));  
  return (MP->getMetaPoolValue());
}

static void
AddCallToRegFunc (Function* F, GlobalVariable* GV, Function* PR, Value* PH,
                  Value* AllocSize) {
  //
  // First, make sure that we're not registering an external zero-sized global.
  // These are caused by the following C construct and should never be checked:
  //  extern variable[];
  //
  ConstantInt * C;
  if (GV->isExternal())
    if ((C = dyn_cast<ConstantInt>(AllocSize)) && (!(C->getZExtValue())))
      return;

  const Type *VoidPtrType = PointerType::get(Type::SByteTy); 

  assert(PH && "No PoolHandle for Global!");

  BasicBlock::iterator InsertPt = F->getEntryBlock().begin();
  Instruction *GVCasted = new CastInst(GV, VoidPtrType, GV->getName()+"casted",InsertPt);
  Value *PHCasted = new CastInst(PH, VoidPtrType, PH->getName()+"casted",InsertPt);
  AllocSize = castTo (AllocSize, Type::UIntTy, InsertPt);
  std::vector<Value *> args (1, PHCasted);
  args.push_back (GVCasted);
  args.push_back (AllocSize);
  new CallInst(PR, args, "", InsertPt);
}

////////////////////////////////////////////////////////////////////////////
// Class: PreInsertPoolChecks
////////////////////////////////////////////////////////////////////////////

//
// Method: runOnModule()
//
// Description:
//  This method is called by the pass manager.  The job of this class is to
//  create any global variables and inter-procedural changes that cannot be
//  done in the doInitialization() method of the InsertPoolChecks class.
//
bool
PreInsertPoolChecks::runOnModule (Module & M) {
  // Retrieve the analysis results from other passes
  cuaPass = &getAnalysis<ConvertUnsafeAllocas>();
  TDPass  = &getAnalysis<TDDataStructures>();
  TD      = &getAnalysis<TargetData>();

  // Add prototypes for the run-time checks to the module
  addPoolCheckProto (M);

  // Register global arrays and collapsed nodes with global pools
  if (!DisableRegisterGlobals) registerGlobalArraysWithGlobalPools(M);

  //
  // Create a MetaPool global variable for every DSNode that we might
  // encounter.
  //
  Module::iterator mI = M.begin(), mE = M.end();
  for ( ; mI != mE; ++mI) {
    Function *F = mI;

    // Skip functions that are external
    if (F->isExternal()) continue;

    // Skip the poolcheckglobals() function because it won't have a DSGraph
    if (F->getName() == "poolcheckglobals") continue;

    // Create a MetaPool variable for each DSNode in the DSGraph.
    DSGraph & TDG = getDSGraph(*F);
    DSGraph::node_iterator NI = TDG.node_begin(), NE = TDG.node_end();
    while (NI != NE) {
      addLinksNeedingAlignment (NI);
      createPoolHandle (M, NI);
      ++NI;
    }
  }
}

//
// Method: getDSGraph()
//
// Description:
//  Return the DSGraph for the given function.  This method automatically
//  selects the correct pass to query for the graph based upon whether we're
//  doing user-space or kernel analysis.
//
DSGraph &
PreInsertPoolChecks::getDSGraph(Function & F) {
#ifndef LLVA_KERNEL
  return equivPass->getDSGraph(F);
#else  
  return TDPass->getDSGraph(F);
#endif  
}

//
// Method: addLinksNeedingAlignment()
//
// Description:
//  Determine if this DSNode has any pointers to DSNodes which will require
//  alignment checks.  If so, add those DSNodes to the set of DSNodes needing
//  alignment checks.  Note that we do not determine if the *given* node needs
//  alignment checks.
//
void
PreInsertPoolChecks::addLinksNeedingAlignment (DSNode * Node) {
  //
  // Determine whether an alignment check is needed.  This occurs when a DSNode
  // is type unknown (collapsed) but has pointers to type known (uncollapsed)
  // DSNodes.
  //
  if ((Node) && (Node->isNodeCompletelyFolded())) {
    for (unsigned i = 0 ; i < Node->getNumLinks(); ++i) {
      DSNode * LinkNode = Node->getLink(i).getNode();
      if (LinkNode && (!(LinkNode->isNodeCompletelyFolded()))) {
        AlignmentNodes.insert (LinkNode);
      }
    }
  }
}

void
PreInsertPoolChecks::addPoolCheckProto(Module &M) {
  const Type * VoidPtrType = PointerType::get(Type::SByteTy);
#if 0
  const Type *PoolDescType = ArrayType::get(VoidPtrType, 50);
  //	StructType::get(make_vector<const Type*>(VoidPtrType, VoidPtrType,
  //                                               Type::UIntTy, Type::UIntTy, 0));
  const Type * PoolDescTypePtr = PointerType::get(PoolDescType);
#endif

  std::vector<const Type *> Arg(1, VoidPtrType);
  Arg.push_back(VoidPtrType);
  FunctionType *PoolCheckTy =
    FunctionType::get(Type::VoidTy,Arg, false);
  PoolCheck = M.getOrInsertFunction("poolcheck", PoolCheckTy);

  std::vector<const Type *> Arg2(1, VoidPtrType);
  Arg2.push_back(VoidPtrType);
  Arg2.push_back(VoidPtrType);
  FunctionType *PoolCheckArrayTy =
    FunctionType::get(Type::VoidTy,Arg2, false);
  PoolCheckArray = M.getOrInsertFunction("poolcheckarray", PoolCheckArrayTy);
  PoolCheckIArray = M.getOrInsertFunction("poolcheckarray_i", PoolCheckArrayTy);


  std::vector<const Type *> Arg3(1, VoidPtrType);
  Arg3.push_back(VoidPtrType); //for output
  Arg3.push_back(VoidPtrType); //for referent 
  FunctionType *BoundsCheckTy = FunctionType::get(VoidPtrType,Arg3, false);
  BoundsCheck   = M.getOrInsertFunction("pchk_bounds", BoundsCheckTy);
  UIBoundsCheck = M.getOrInsertFunction("pchk_bounds_i", BoundsCheckTy);

  std::vector<const Type *> Arg4(1, VoidPtrType);
  Arg4.push_back(VoidPtrType);
  FunctionType *getBoundsTy = FunctionType::get(VoidPtrType,Arg4, false);
  getBounds   = M.getOrInsertFunction("getBounds",   getBoundsTy);
  UIgetBounds = M.getOrInsertFunction("getBounds_i", getBoundsTy);

  //Get the poolregister function
  PoolRegister = M.getOrInsertFunction("pchk_reg_obj", Type::VoidTy, VoidPtrType,
                                       VoidPtrType, Type::UIntTy, NULL);
  StackRegister = M.getOrInsertFunction("pchk_reg_stack", Type::VoidTy, VoidPtrType,
                                       VoidPtrType, Type::UIntTy, NULL);
  ObjFree = M.getOrInsertFunction("pchk_drop_obj", Type::VoidTy, VoidPtrType,
                                  VoidPtrType, NULL);
  StackFree = M.getOrInsertFunction("pchk_drop_stack", Type::VoidTy, VoidPtrType,
                                  VoidPtrType, NULL);
  
  FuncRegister = M.getOrInsertFunction("pchk_reg_func", Type::VoidTy, VoidPtrType, Type::UIntTy, PointerType::get(VoidPtrType), NULL);

  PoolFindMP = M.getOrInsertFunction("pchk_getLoc", VoidPtrType, VoidPtrType, NULL);
  PoolRegMP = M.getOrInsertFunction("pchk_reg_pool", Type::VoidTy, VoidPtrType, VoidPtrType, VoidPtrType, NULL);

  std::vector<const Type *> FArg2(1, Type::IntTy);
  FArg2.push_back(Type::IntTy);
  FArg2.push_back(VoidPtrType);
  FunctionType *ExactCheckTy = FunctionType::get(VoidPtrType, FArg2, false);
  ExactCheck = M.getOrInsertFunction("exactcheck", ExactCheckTy);

  std::vector<const Type *> FArg3(1, Type::UIntTy);
  FArg3.push_back(VoidPtrType);
  FArg3.push_back(VoidPtrType);
  FArg3.push_back(VoidPtrType);
  FArg3.push_back(VoidPtrType);
  FArg3.push_back(VoidPtrType);
  FArg3.push_back(VoidPtrType);
  FArg3.push_back(VoidPtrType);
  FunctionType *FunctionCheckTy = FunctionType::get(Type::VoidTy, FArg3, true);
  FunctionCheck = M.getOrInsertFunction("funccheck", FunctionCheckTy);

  std::vector<const Type *> FArg6(1, VoidPtrType);
  FArg6.push_back(VoidPtrType);
  FunctionType *FunctionCheckGTy = FunctionType::get(Type::VoidTy, FArg6, true);
  FunctionCheckG = M.getOrInsertFunction("funccheck_g", FunctionCheckGTy);

  std::vector<const Type *> FArg9(1, Type::UIntTy);
  FArg9.push_back(VoidPtrType);
  FArg9.push_back(PointerType::get(VoidPtrType));
  FunctionType *FunctionCheckTTy = FunctionType::get(Type::VoidTy, FArg9, true);
  FunctionCheckT = M.getOrInsertFunction("funccheck_t", FunctionCheckTTy);

  FArg9.clear();
  FArg9.push_back(VoidPtrType);
  FunctionType *ICCheckTy = FunctionType::get(Type::VoidTy, FArg9, true);
  ICCheck = M.getOrInsertFunction("pchk_iccheck", ICCheckTy);

  std::vector<const Type*> FArg5(1, VoidPtrType);
  FArg5.push_back(VoidPtrType);
  FunctionType *GetActualValueTy = FunctionType::get(VoidPtrType, FArg5, false);
  GetActualValue = M.getOrInsertFunction("pchk_getActualValue", GetActualValueTy);

  std::vector<const Type*> FArg4(1, VoidPtrType); //base
  FArg4.push_back(VoidPtrType); //result
  FArg4.push_back(Type::UIntTy); //size
  FunctionType *ExactCheck2Ty = FunctionType::get(VoidPtrType, FArg4, false);
  ExactCheck2  = M.getOrInsertFunction("exactcheck2",  ExactCheck2Ty);

  std::vector<const Type*> FArg7(1, VoidPtrType); //base
  FArg7.push_back(VoidPtrType); //result
  FArg7.push_back(VoidPtrType); //end
  FunctionType *ExactCheck3Ty = FunctionType::get(VoidPtrType, FArg7, false);
  ExactCheck3 = M.getOrInsertFunction("exactcheck3", ExactCheck3Ty);

  std::vector<const Type*> FArg8(1, VoidPtrType); //base
  FunctionType *getBeginEndTy = FunctionType::get(VoidPtrType, FArg8, false);
  getBegin = M.getOrInsertFunction("getBegin", getBeginEndTy);
  getEnd   = M.getOrInsertFunction("getEnd",   getBeginEndTy);
}

void
PreInsertPoolChecks::registerGlobalArraysWithGlobalPools(Module &M) {
#ifdef LLVA_KERNEL

  const Type *VoidPtrType = PointerType::get(Type::SByteTy); 
  const Type *PoolDescType = ArrayType::get(VoidPtrType, 50);
  const Type *PoolDescPtrTy = PointerType::get(PoolDescType);
  const Type* csiType = Type::getPrimitiveType(Type::UIntTyID);

  //Get the registration function
  std::vector<const Type*> Vt;
  const FunctionType* RFT = FunctionType::get(Type::VoidTy, Vt, false);
  Function* RegFunc = M.getOrInsertFunction("poolcheckglobals", RFT);
  if (RegFunc->empty()) {
    BasicBlock* BB = new BasicBlock("reg", RegFunc);
    new ReturnInst(BB);
  }

  //Now iterate over globals and register all the arrays
  DSGraph &G = TDPass->getGlobalsGraph();
  Module::global_iterator GI = M.global_begin(), GE = M.global_end();
  for ( ; GI != GE; ++GI) {
    if (GlobalVariable *GV = dyn_cast<GlobalVariable>(GI)) {
      if (GV->getType() != PoolDescPtrTy) {
        GlobalValue * GVLeader = G.getScalarMap().getLeaderForGlobal(GV);
        DSNode *DSN  = G.getNodeForValue(GVLeader).getNode();
        if (((isa<ArrayType>(GV->getType()->getElementType())) ||
            (DSN && DSN->isNodeCompletelyFolded()))
            && !isa<OpaqueType>(GV->getType())
            && !(isa<PointerType>(GV->getType()) && 
                 isa<OpaqueType>(cast<PointerType>(GV->getType())->getElementType()))) {
          Value * AllocSize;
          if (const ArrayType *AT = dyn_cast<ArrayType>(GV->getType()->getElementType())) {
            //std::cerr << "found global" << *GI << std::endl;
            AllocSize = ConstantInt::get(csiType,
                  (AT->getNumElements() * TD->getTypeSize(AT->getElementType())));
          } else {
            AllocSize = ConstantInt::get(csiType, TD->getTypeSize(GV->getType()->getElementType()));
          }
          Value* PH = getPD(DSN, M);
          if (PH)
            AddCallToRegFunc(RegFunc, GV, PoolRegister, PH, AllocSize);
        }
      }
    }
  }
#else
  Function *MainFunc = M.getMainFunction();
  if (MainFunc == 0 || MainFunc->isExternal()) {
    std::cerr << "Cannot do array bounds check for this program"
	      << "no 'main' function yet!\n";
    abort();
  }
  //First register, argc and argv
  Function::arg_iterator AI = MainFunc->arg_begin(), AE = MainFunc->arg_end();
  if (AI != AE) {
    //There is argc and argv
    Value *Argc = AI;
    AI++;
    Value *Argv = AI;
    PA::FuncInfo *FI = paPass->getFuncInfoOrClone(*MainFunc);
    Value *PH= getPoolHandle(Argv, MainFunc, *FI);
    Function *PoolRegister = paPass->PoolRegister;
    BasicBlock::iterator InsertPt = MainFunc->getEntryBlock().begin();
    while ((isa<CallInst>(InsertPt)) || isa<CastInst>(InsertPt) || isa<AllocaInst>(InsertPt) || isa<BinaryOperator>(InsertPt)) ++InsertPt;
    if (PH) {
      Type *VoidPtrType = PointerType::get(Type::SByteTy); 
      Instruction *GVCasted = new CastInst(Argv,
					   VoidPtrType, Argv->getName()+"casted",InsertPt);
      const Type* csiType = Type::getPrimitiveType(Type::UIntTyID);
      Value *AllocSize = new CastInst(Argc,
				      csiType, Argc->getName()+"casted",InsertPt);
      AllocSize = BinaryOperator::create(Instruction::Mul, AllocSize,
					 ConstantInt::get(csiType, 4), "sizetmp", InsertPt);
      new CallInst(PoolRegister,
				  make_vector(PH, AllocSize, GVCasted, 0),
				  "", InsertPt); 
      
    } else {
      std::cerr << "argv's pool descriptor is not present. \n";
      //	abort();
    }
    
  }

  //Now iterate over globals and register all the arrays
  Module::global_iterator GI = M.global_begin(), GE = M.global_end();
  for ( ; GI != GE; ++GI) {
    if (GlobalVariable *GV = dyn_cast<GlobalVariable>(GI)) {
      Type *VoidPtrType = PointerType::get(Type::SByteTy); 
      Type *PoolDescType = ArrayType::get(VoidPtrType, 50);
      Type *PoolDescPtrTy = PointerType::get(PoolDescType);
      if (GV->getType() != PoolDescPtrTy) {
        DSGraph &G = equivPass->getGlobalsGraph();
        DSNode *DSN  = G.getNodeForValue(GV).getNode();
        if ((isa<ArrayType>(GV->getType()->getElementType())) ||
            DSN->isNodeCompletelyFolded()) {
          Value * AllocSize;
          const Type* csiType = Type::getPrimitiveType(Type::UIntTyID);
          if (const ArrayType *AT = dyn_cast<ArrayType>(GV->getType()->getElementType())) {
            //std::cerr << "found global" << *GI << std::endl;
            AllocSize = ConstantInt::get(csiType,
                  (AT->getNumElements() * TD->getTypeSize(AT->getElementType())));
          } else {
            AllocSize = ConstantInt::get(csiType, TD->getTypeSize(GV->getType()));
          }
          Function *PoolRegister = paPass->PoolRegister;
          BasicBlock::iterator InsertPt = MainFunc->getEntryBlock().begin();
          //skip the calls to poolinit
          while ((isa<CallInst>(InsertPt))  ||
                  isa<CastInst>(InsertPt)   ||
                  isa<AllocaInst>(InsertPt) ||
                  isa<BinaryOperator>(InsertPt)) ++InsertPt;
      
          std::map<const DSNode *, Value *>::iterator I = paPass->GlobalNodes.find(DSN);
          if (I != paPass->GlobalNodes.end()) {
            Value *PH = I->second;
            Instruction *GVCasted = new CastInst(GV,
                   VoidPtrType, GV->getName()+"casted",InsertPt);
            new CallInst(PoolRegister,
                make_vector(PH, AllocSize, GVCasted, 0),
                "", InsertPt); 
          } else {
            std::cerr << "pool descriptor not present \n ";
            abort();
          }
        }
      }
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////////////
// Class: InsertPoolChecks
////////////////////////////////////////////////////////////////////////////

void
InsertPoolChecks::addPoolCheckProto(Module &M) {
  const Type * VoidPtrType = PointerType::get(Type::SByteTy);
#if 0
  const Type *PoolDescType = ArrayType::get(VoidPtrType, 50);
  //	StructType::get(make_vector<const Type*>(VoidPtrType, VoidPtrType,
  //                                               Type::UIntTy, Type::UIntTy, 0));
  const Type * PoolDescTypePtr = PointerType::get(PoolDescType);
#endif

  std::vector<const Type *> Arg(1, VoidPtrType);
  Arg.push_back(VoidPtrType);
  FunctionType *PoolCheckTy =
    FunctionType::get(Type::VoidTy,Arg, false);
  PoolCheck   = M.getOrInsertFunction("poolcheck", PoolCheckTy);
  PoolCheckUI = M.getOrInsertFunction("poolcheck_i", PoolCheckTy);

  Arg.clear();
  Arg.push_back(VoidPtrType);
  Arg.push_back(VoidPtrType);
  Arg.push_back(Type::UIntTy);
  PoolCheckTy = FunctionType::get(Type::VoidTy,Arg, false);
  PoolCheckAlign   = M.getOrInsertFunction("poolcheckalign", PoolCheckTy);
  PoolCheckAlignUI = M.getOrInsertFunction("poolcheckalign_i", PoolCheckTy);

  std::vector<const Type *> Arg2(1, VoidPtrType);
  Arg2.push_back(VoidPtrType);
  Arg2.push_back(VoidPtrType);
  FunctionType *PoolCheckArrayTy =
    FunctionType::get(Type::VoidTy,Arg2, false);
  PoolCheckArray = M.getOrInsertFunction("poolcheckarray", PoolCheckArrayTy);
  PoolCheckIArray = M.getOrInsertFunction("poolcheckarray_i", PoolCheckArrayTy);


  std::vector<const Type *> Arg3(1, VoidPtrType);
  Arg3.push_back(VoidPtrType); //for output
  Arg3.push_back(VoidPtrType); //for referent 
  FunctionType *BoundsCheckTy = FunctionType::get(VoidPtrType,Arg3, false);
  BoundsCheck   = M.getOrInsertFunction("pchk_bounds", BoundsCheckTy);
  UIBoundsCheck = M.getOrInsertFunction("pchk_bounds_i", BoundsCheckTy);

  std::vector<const Type *> Arg4(1, VoidPtrType);
  Arg4.push_back(VoidPtrType);
  FunctionType *getBoundsTy = FunctionType::get(VoidPtrType,Arg4, false);
  getBounds   = M.getOrInsertFunction("getBounds",   getBoundsTy);
  UIgetBounds = M.getOrInsertFunction("getBounds_i", getBoundsTy);

  //Get the poolregister function
  PoolRegister = M.getOrInsertFunction("pchk_reg_obj", Type::VoidTy, VoidPtrType,
                                       VoidPtrType, Type::UIntTy, NULL);
  StackRegister = M.getOrInsertFunction("pchk_reg_stack", Type::VoidTy, VoidPtrType,
                                       VoidPtrType, Type::UIntTy, NULL);
  ObjFree = M.getOrInsertFunction("pchk_drop_obj", Type::VoidTy, VoidPtrType,
                                  VoidPtrType, NULL);
  StackFree = M.getOrInsertFunction("pchk_drop_stack", Type::VoidTy, VoidPtrType,
                                  VoidPtrType, NULL);
  
  FuncRegister = M.getOrInsertFunction("pchk_reg_func", Type::VoidTy, VoidPtrType, Type::UIntTy, PointerType::get(VoidPtrType), NULL);

  PoolFindMP = M.getOrInsertFunction("pchk_getLoc", VoidPtrType, VoidPtrType, NULL);
  PoolRegMP = M.getOrInsertFunction("pchk_reg_pool", Type::VoidTy, VoidPtrType, VoidPtrType, VoidPtrType, NULL);

  //  Function *KmemCacheAlloc = M.getNamedFunction("kmem_cache_alloc");
  //  assert(KmemCacheAlloc->arg_size() == 2 &&"Kmem_cache_alloc number of arguments != 2");
  //  Function::arg_iterator kcai = KmemCacheAlloc->arg_begin();
  //  Type *kmem_cache_t = kcai->getType();
  //  kcai++;
  //  Type *kem_cache_size_t = kcai->getType();
  //  We'll use Void * instead of the above
  KmemCachegetSize = M.getOrInsertFunction("kmem_cache_size", Type::UIntTy, VoidPtrType, NULL);

  std::vector<const Type *> FArg2(1, Type::IntTy);
  FArg2.push_back(Type::IntTy);
  FArg2.push_back(VoidPtrType);
  FunctionType *ExactCheckTy = FunctionType::get(VoidPtrType, FArg2, false);
  ExactCheck = M.getOrInsertFunction("exactcheck", ExactCheckTy);

  std::vector<const Type *> FArg3(1, Type::UIntTy);
  FArg3.push_back(VoidPtrType);
  FArg3.push_back(VoidPtrType);
  FunctionType *FunctionCheckTy = FunctionType::get(Type::VoidTy, FArg3, true);
  FunctionCheck = M.getOrInsertFunction("funccheck", FunctionCheckTy);

  std::vector<const Type *> FArg6(1, VoidPtrType);
  FArg6.push_back(VoidPtrType);
  FunctionType *FunctionCheckGTy = FunctionType::get(Type::VoidTy, FArg6, true);
  FunctionCheckG = M.getOrInsertFunction("funccheck_g", FunctionCheckGTy);

  std::vector<const Type *> FArg9(1, Type::UIntTy);
  FArg9.push_back(VoidPtrType);
  FArg9.push_back(PointerType::get(VoidPtrType));
  FunctionType *FunctionCheckTTy = FunctionType::get(Type::VoidTy, FArg9, true);
  FunctionCheckT = M.getOrInsertFunction("funccheck_t", FunctionCheckTTy);

  FArg9.clear();
  FArg9.push_back(VoidPtrType);
  FunctionType *ICCheckTy = FunctionType::get(Type::VoidTy, FArg9, true);
  ICCheck = M.getOrInsertFunction("pchk_iccheck", ICCheckTy);

  std::vector<const Type*> FArg5(1, VoidPtrType);
  FArg5.push_back(VoidPtrType);
  FunctionType *GetActualValueTy = FunctionType::get(VoidPtrType, FArg5, false);
  GetActualValue = M.getOrInsertFunction("pchk_getActualValue", GetActualValueTy);

  std::vector<const Type*> FArg4(1, VoidPtrType); //base
  FArg4.push_back(VoidPtrType); //result
  FArg4.push_back(Type::UIntTy); //size
  FunctionType *ExactCheck2Ty = FunctionType::get(VoidPtrType, FArg4, false);
  ExactCheck2  = M.getOrInsertFunction("exactcheck2",  ExactCheck2Ty);

  std::vector<const Type*> FArg7(1, VoidPtrType); //base
  FArg7.push_back(VoidPtrType); //result
  FArg7.push_back(VoidPtrType); //end
  FunctionType *ExactCheck3Ty = FunctionType::get(VoidPtrType, FArg7, false);
  ExactCheck3 = M.getOrInsertFunction("exactcheck3", ExactCheck3Ty);

  std::vector<const Type*> FArg8(1, VoidPtrType); //base
  FunctionType *getBeginEndTy = FunctionType::get(VoidPtrType, FArg8, false);
  getBegin = M.getOrInsertFunction("getBegin", getBeginEndTy);
  getEnd   = M.getOrInsertFunction("getEnd",   getBeginEndTy);
}


void
InsertPoolChecks::addHeapRegs (Module & M) {
  DSNode * Node;
  MetaPool * MP;
  const Type * VoidPtrType = PointerType::get(Type::SByteTy);
  std::set<Value *> ProcessedMetaPools;
  std::vector<Value *> args;

  while (PHNeeded.size()) {
    Node = *(PHNeeded.begin());
    PHNeeded.erase(Node);
    MP = Node->getMP();
    Value* MPV = MP->getMetaPoolValue();

    //
    // Determine if we have already processed this MetaPool.  If we have, then
    // just go on to the next DSNode.
    if (ProcessedMetaPools.find (MPV) == ProcessedMetaPools.end())
      ProcessedMetaPools.insert (MPV);
    else
      continue;

    // Add registers in front of every allocation
    for (std::list<CallSite>::iterator i = MP->allocs.begin(),
          e = MP->allocs.end(); i != e; ++i) {
      args.clear();
      std::string name = i->getCalledFunction()->getName();
      if (name == "kmem_cache_alloc") {
        Instruction * InsertPt = i->getInstruction();
        //insert a register before
        Value* VP  = castTo (i->getArgument(0), VoidPtrType, InsertPt);
        Value* VMP = castTo (MPV, VoidPtrType, InsertPt);
        Value* VMPP = new CallInst(PoolFindMP, VP, "", InsertPt);

        args.push_back (VMP);
        args.push_back (VP);
        args.push_back (VMPP);
        new CallInst (PoolRegMP, args, "", i->getInstruction());

        //Insert Obj reg using kmem_cache_size after
        Instruction* IP = i->getInstruction()->getNext();
        Value* VRP = castTo (i->getInstruction(), VoidPtrType, IP);
        CallInst *len = new CallInst(KmemCachegetSize, VP, "", IP);

        args.clear();
        args.push_back (VMP); //MetaPool
        args.push_back (VRP); //object 
        args.push_back (len); //len
        new CallInst(PoolRegister, args, "", IP);

#if 0
      } else if ((name == "kmalloc") ||
#else
      } else if (
#endif
                 (name == "__vmalloc") ||
                 (name == "__alloc_bootmem")) {
        //inser obj register after
        Instruction* IP = i->getInstruction()->getNext();
        Value* VP = castTo (i->getInstruction(), VoidPtrType, IP);
        Value* VMP = castTo (MPV, VoidPtrType, IP);
        Value* len = castTo (i->getArgument(0), Type::UIntTy, IP);

        args.push_back (VMP);
        args.push_back (VP);
        args.push_back (len);
        new CallInst(PoolRegister, args, "", IP);
      } else
        assert(0 && "unknown alloc");
    }
  }
}

void InsertPoolChecks::addMetaPools(Module& M, MetaPool* MP, DSNode* N) {
  if (!MP || MP->getMetaPoolValue()) return;
  MP->setMetaPoolValue(makeMetaPool(&M, N));  
  Value* MPV = MP->getMetaPoolValue();
  //added registers in front of every allocation
  for(std::list<CallSite>::iterator i = MP->allocs.begin(),
        e = MP->allocs.end(); i != e; ++i) {
    std::string name = i->getCalledFunction()->getName();
    if (name == "kmem_cache_alloc") {
      //insert a register before
      Value* VP = new CastInst(i->getArgument(0), PointerType::get(Type::SByteTy), "", i->getInstruction());
      Value* VMP = new CastInst(MPV, PointerType::get(Type::SByteTy), "MP", i->getInstruction());
      Value* VMPP = new CallInst(PoolFindMP, make_vector(VP, 0), "", i->getInstruction());
      new CallInst(PoolRegMP, make_vector(VMP, VP, VMPP, 0), "", i->getInstruction());
#if 0
    } else if ((name == "kmalloc") ||
#else
    } else if (
#endif
               (name == "__vmalloc") ||
               (name == "__alloc_bootmem")) {
      //inser obj register after
      Instruction* IP = i->getInstruction()->getNext();
      Value* VP = new CastInst(i->getInstruction(), PointerType::get(Type::SByteTy), "", IP);
      Value* VMP = new CastInst(MPV, PointerType::get(Type::SByteTy), "MP", IP);
      Value* len = new CastInst(i->getArgument(0), Type::UIntTy, "len", IP);
      new CallInst(PoolRegister, make_vector(VMP, VP, len, 0), "", IP);
    } else
      assert(0 && "unknown alloc");
  }
}

void InsertPoolChecks::addObjFrees(Module& M) {
#ifdef LLVA_KERNEL
#if 0
  Function* KMF = M.getNamedFunction("kfree");
#endif
  Function* VMF = M.getNamedFunction("vfree");
  std::list<Function*> L;
#if 0
  if (KMF) L.push_back(KMF);
#endif
  if (VMF) L.push_back(VMF);
  for (std::list<Function*>::iterator ii = L.begin(), ee = L.end();
       ii != ee; ++ii) {
    Function* F = *ii;
    for (Value::use_iterator ii = F->use_begin(), ee = F->use_end();
         ii != ee; ++ii) {
      if (CallInst* CI = dyn_cast<CallInst>(*ii)) {
        if (CI->getCalledFunction() == F) {
          Value* Ptr = CI->getOperand(1);
          Value* MP = getPD(getDSNode(Ptr, CI->getParent()->getParent()), M);
          if (MP) {
            MP = new CastInst(MP, PointerType::get(Type::SByteTy), "MP", CI);
            Ptr = new CastInst(Ptr, PointerType::get(Type::SByteTy), "ADDR", CI);
            new CallInst(ObjFree, make_vector(MP, Ptr, 0), "", CI);
          }
        }
      }
    }
  }
#endif
}

//
// Method: insertICCheck()
//
// Description:
//  Perform a run-time check to ensure that this pointer points to the
//  beginning of an interrupt context.
//
void
InsertPoolChecks::insertICCheck (Value * IC, Instruction * InsertPt) {
  const Type * VoidPtrType = PointerType::get(Type::SByteTy);
  Value * CastPointer = castTo (IC, VoidPtrType, InsertPt);
  new CallInst (ICCheck, CastPointer, "", InsertPt);
}

//
// Return value:
//  Returns the inserted boundscheck call instruction.  This can be used to
//  replace the destination instruction if desired.
//  If no instruction was inserted, it returns Dest.
//
Value *
InsertPoolChecks::insertBoundsCheck (Instruction * I,
                                     Value * Src,
                                     Value * Dest,
                                     Instruction * InsertPt) {
  // Enclosing function
  Function * F   = I->getParent()->getParent();
  Function *Fnew = F;

  // Source node on which we will look up the pool handle
  Value *newSrc = Src;

#ifndef LLVA_KERNEL    
#ifdef NOEQUIV
  // Some times the ECGraphs doesnt contain F for newly created cloned
  // functions
  if (!equivPass->ContainsDSGraphFor(*F)) {
    PA::FuncInfo *FI = paPass->getFuncInfoOrClone(*F);
    newSrc = FI->MapValueToOriginal(Src);
    Instruction * Inew = FI->MapValueToOriginal(I);
    assert(newSrc && " Instruction not in value map (clone)\n");
  }
  Fnew = Inew->getParent()->getParent();
#endif
#endif          

  //
  // Get the pool handle for the source pointer.
  //
  Value *PH = getPoolHandle(newSrc, Fnew); 
  if (!PH) {
    // Update statistics
    ++NullBoundsChecks;
    return Dest;
  }

  //
  // Get the DSGraph of the enclosing function and get the corresponding
  // DSNode.
  //
  DSGraph & TDG = getDSGraph(*F);
  DSNode * Node = TDG.getNodeForValue(I).getNode();

  //
  // Determine whether an alignment check is needed.  This occurs when a DSNode
  // is type unknown (collapsed) but has pointers to type known (uncollapsed)
  // DSNodes.
  //
  if (preSCPass->nodeNeedsAlignment (Node)) {
    ++AlignChecks;
  }

  // If there is no DSNode, do not perform a check
  if (!Node) return Dest;

  // If we do not know the allocation site, don't bother checking it
  if (!(isNodeRegistered (Node)))
    return Dest;

  // Record statistics on the incomplete checks we do.  Note that a node may
  // be counted more than once.
  if (Node->isIncomplete())
    ++IBoundsChecks;
  if (Node->isUnknownNode())
    ++UBoundsChecks;
  if (Node->isAllocaNode())
    ++ABoundsChecks;

  //
  // Cast the pool handle, source and destination pointers into the correct
  // type for the call instruction.
  //
  Value * SrcCast = Src;
  Value * DestCast = Dest;
  if (Src->getType() != PointerType::get(Type::SByteTy))
    SrcCast = new CastInst (Src, PointerType::get(Type::SByteTy),
                            "castsrc", InsertPt);
  if (Dest->getType() != PointerType::get(Type::SByteTy))
    DestCast = new CastInst (Dest, PointerType::get(Type::SByteTy),
                            "castdst", InsertPt);
  if (PH->getType() != PointerType::get(Type::SByteTy))
    PH = new CastInst (PH, PointerType::get(Type::SByteTy),
                            "castph", InsertPt);

  AddedValues.insert(DestCast);

  //
  // Insert the bounds check.
  //
  Value * CI;
  if (EnableSplitChecks) {
    std::vector<Value *> args(1, PH);
    args.push_back (SrcCast);
    if ((Node == 0) ||
        (Node->isAllocaNode()) ||
        (Node->isIncomplete()) ||
        (Node->isUnknownNode()))
      CI = new CallInst(UIgetBounds, args, "uibc",InsertPt);
    else
      CI = new CallInst(getBounds, args, "bc", InsertPt);
    std::vector<Value *> boundsargs(1, CI);
    Instruction * LowerBound = new CallInst(getBegin, boundsargs, "gb", InsertPt);
    Instruction * UpperBound = new CallInst(getEnd,   boundsargs, "ge", InsertPt);
    Value * EC3 = addExactCheck3 (LowerBound, DestCast, UpperBound, InsertPt);
    AddedValues.insert(EC3);
    if (EC3->getType() != Dest->getType())
      EC3 = new CastInst(EC3, Dest->getType(), EC3->getName(), InsertPt);
    AddedValues.insert(EC3);
    // Replace all uses of the original pointer with the result of the exactcheck.
    // This ensures that the check will not get dead code eliminated.
    Value::use_iterator UI = Dest->use_begin();
    for (; UI != Dest->use_end(); ++UI) {
      if (AddedValues.find(*UI) == AddedValues.end())
        UI->replaceUsesOfWith (Dest,EC3);
    }
    CI = EC3;
  } else {
    std::vector<Value *> args(1, PH);
    args.push_back (SrcCast);
    args.push_back (DestCast);
    if ((Node == 0) ||
        (Node->isAllocaNode()) ||
        (Node->isIncomplete()) ||
        (Node->isUnknownNode()))
      CI = new CallInst(UIBoundsCheck, args, "uibc",InsertPt);
    else
      CI = new CallInst(BoundsCheck, args, "bc", InsertPt);
  }

  //
  // Record that this value was checked.
  //
  CheckedValues.insert (Dest);
  CheckedValues.insert (CI);

  // Update statistics
  ++BoundsChecks;
  return CI;
}

//
// Method: getOrCreateFunctionTable()
//
// Description:
//  Given a DSNode, return a global variable containing an array of the
//  function pointers within that DSNode.  If this global variable does not
//  exist, create it and modify poolcheckglobals() to register it with its
//  MetaPool.
//
GlobalVariable *
InsertPoolChecks::getOrCreateFunctionTable (DSNode * Node, Value * PH,
                                            Module * M) {
  // Determine if we have already created such a global.
  if (FuncListMap.count (PH)) {
    return FuncListMap[PH];
  }

	// Get the globals list corresponding to the node
	std::vector<Function *> FuncList;
	Node->addFullFunctionList(FuncList);
	unsigned num = FuncList.size();

  // Create a global array of void pointers containing the list of possible
  // function targets.
  std::vector<Function *>::iterator flI= FuncList.begin(), flE = FuncList.end();
  const Type* csiType = Type::getPrimitiveType(Type::UIntTyID);
  Value *NumArg = ConstantInt::get(csiType, num);	
  Type *VoidPtrType = PointerType::get(Type::SByteTy); 
  std::vector<Constant *> Targets;

  for (; flI != flE ; ++flI) {
    Function *func = *flI;
    Constant *CastfuncI = ConstantExpr::getCast (func, VoidPtrType);
    Targets.push_back(CastfuncI);
  }

  ArrayType * TableType = ArrayType::get (VoidPtrType, num);
  Constant * TableInit = ConstantArray::get (TableType, Targets);
  GlobalVariable * Table = new GlobalVariable (TableType, true, GlobalValue::InternalLinkage, TableInit, "functargets", M);
  Value * TableCast = ConstantExpr::getCast (Table, PointerType::get(VoidPtrType));

  //
  // Add an instruction to poolcheckglobals() to register this table of
  // functions with the MetaPool.
  //
  std::vector<const Type*> Vt;
  const FunctionType* RFT = FunctionType::get(Type::VoidTy, Vt, false);
  Function * RegFunc = M->getOrInsertFunction("poolcheckglobals", RFT);
  BasicBlock* BB;
  if (RegFunc->empty()) {
    BasicBlock* BB = new BasicBlock("reg", RegFunc);
    new ReturnInst(BB);
  } else {
    BB = &(RegFunc->getEntryBlock());
  }

  Instruction * InsertPt = BB->getTerminator();
  Value * CastPH = new CastInst(PH, VoidPtrType, "casted", InsertPt);
  std::vector<Value *> args;
  args.push_back (CastPH);
  args.push_back (NumArg);
  args.push_back (TableCast);
  new CallInst(FuncRegister, args, "", InsertPt); 

  //
  // Insert the DSNode and Table into the map.
  //
  FuncListMap.insert (make_pair(PH,Table));
  return Table;
}

//
// Method: insertFunctionCheck()
//
// Description:
//  Insert a run-time check on the argument to an indirect call instruction.
//
void
InsertPoolChecks::insertFunctionCheck (CallInst * CI) {
  if (DisableFuncChecks)
    return;

  // Get the containing function and the function pointer.
  Function * F = CI->getParent()->getParent();
  Value * FuncPointer = CI->getCalledValue();

  // Try to remove any encapsulating casts to get to the original instruction
  ConstantExpr *cExpr;
  while ((cExpr = dyn_cast<ConstantExpr>(FuncPointer)) &&
         (cExpr->getOpcode() == Instruction::Cast))
      FuncPointer = cExpr->getOperand(0);

  //
  // Determine if the function pointer came from a load instruction that loaded
  // its value from a type known pool.  If it is, it does not need a check.
  //
  if (LoadInst * LI = dyn_cast<LoadInst>(FuncPointer)) {
    DSNode * LoadNode = getDSNode (LI->getPointerOperand(), F);
    if ((LoadNode) && (!(LoadNode->isNodeCompletelyFolded())))
      return;
  }

  //
  // Get the DSNode and Pool handle for the call instruction.
  //
  Value *PH = getPoolHandle (FuncPointer, F);
  if (!PH) {
    // Update statistics
    ++NullBoundsChecks;
    return;
  }
  DSNode* Node = getDSNode(FuncPointer, F);

  //
  // If the node is incomplete, then the call targets associated with the
  // DSNode may be missing valid function targets.  In that case, do not
  // insert a check.
  //
  if ((!EnableUnknownChecks) && (Node->isIncomplete())) {
    ++MissedFuncChecks;
    return;
  }
 
	// Get the globals list corresponding to the node
	std::vector<Function *> FuncList;
	Node->addFullFunctionList(FuncList);
	unsigned num = FuncList.size();

  //
  // Insert a call to one of the function check functions, depending upon
  // the number of functions we must check against.
  //
  if (num == 0) {
    ++ZeroFuncChecks;
    return;
  }

  // Update statistics on the number of indirect call checks.
  ++FuncChecks;

  std::vector<Function *>::iterator flI= FuncList.begin(), flE = FuncList.end();
  const Type* csiType = Type::getPrimitiveType(Type::UIntTyID);
  Value *NumArg = ConstantInt::get(csiType, num);	
  Type *VoidPtrType = PointerType::get(Type::SByteTy); 

  if (num < 7) {
    const Type* csiType = Type::getPrimitiveType(Type::UIntTyID);
    Value *NumArg = ConstantInt::get(csiType, num);	
           
    CastInst *CastVI = 
      new CastInst(FuncPointer, VoidPtrType, "casted", (Instruction *)(CI));
  
    std::vector<Value *> args(1, NumArg);
    args.push_back(CastVI);
    for (; flI != flE ; ++flI) {
      Function *func = *flI;
      CastInst *CastfuncI = 
        new CastInst(func, VoidPtrType, "casted", (Instruction *)(CI));
      args.push_back(CastfuncI);
    }
    for (unsigned index = num; index < 7; ++index)
      args.push_back (ConstantPointerNull::get (PointerType::get (Type::SByteTy)));
    new CallInst(FunctionCheck, args,"", (Instruction *)(CI));
  } else if (num < 21) {
    // Create the first two arguments for the function check call
    std::vector<Value *> args(1, NumArg);
    CastInst *CastVI = 
      new CastInst(FuncPointer, VoidPtrType, "casted", (Instruction *)(CI));
    args.push_back(CastVI);

    // Create a global array of void pointers containing the list of possible
    // function targets.
    GlobalVariable * Table = getOrCreateFunctionTable (Node, PH, F->getParent());

    CastInst *CastTable = 
      new CastInst(Table, 
       PointerType::get(VoidPtrType), "casted", (Instruction *)(CI));
  
    args.push_back(CastTable);
    new CallInst(FunctionCheckT, args, "", (Instruction *)(CI));
  } else {
    // Create a global array of void pointers containing the list of possible
    // function targets.
    getOrCreateFunctionTable (Node, PH, F->getParent());

    // Create the first two arguments for the function check call
    PH = new CastInst (PH, VoidPtrType, "casted", (Instruction *)(CI));
    CastInst *CastVI = 
      new CastInst(FuncPointer, VoidPtrType, "casted", (Instruction *)(CI));
    std::vector<Value *> args(1, PH);
    args.push_back(CastVI);

    new CallInst(FunctionCheckG, args,"", (Instruction *)(CI));
  }
}

//
// Function: addExactCheck2()
//
// Description:
//  Utility routine that inserts a call to exactcheck2().
//
// Inputs:
//  BasePointer   - An LLVM Value representing the base of the object to check.
//  Result        - An LLVM Value representing the pointer to check.
//  Bounds        - An LLVM Value representing the bounds of the check.
//  InsertPt      - The instruction before which to insert the check.
//
void
InsertPoolChecks::addExactCheck2 (Value * BasePointer,
                                  Value * Result,
                                  Value * Bounds,
                                  Instruction * InsertPt) {
  Value * ResultPointer = Result;

  // The LLVM type for a void *
  Type *VoidPtrType = PointerType::get(Type::SByteTy); 

  //
  // Cast the operands to the correct type.
  //
  if (BasePointer->getType() != VoidPtrType)
    BasePointer = new CastInst(BasePointer, VoidPtrType,
                               BasePointer->getName()+".ec2.casted",
                               InsertPt);

  if (ResultPointer->getType() != VoidPtrType)
    ResultPointer = new CastInst(ResultPointer, VoidPtrType,
                                 ResultPointer->getName()+".ec2.casted",
                                 InsertPt);

  Value * CastBounds = Bounds;
  if (Bounds->getType() != Type::UIntTy)
    CastBounds = new CastInst(Bounds, Type::UIntTy,
                              Bounds->getName()+".ec.casted", InsertPt);

  //
  // Create the call to exactcheck2().
  //
  std::vector<Value *> args(1, BasePointer);
  args.push_back(ResultPointer);
  args.push_back(CastBounds);
  Instruction * CI;
  CI = new CallInst(ExactCheck2, args, "", InsertPt);

  //
  // Record that this value was checked.
  //
  CheckedValues.insert (Result);

#if 0
  //
  // Replace the old pointer with the return value of exactcheck2(); this
  // prevents GCC from removing it completely.
  //
  if (CI->getType() != GEP->getType())
    CI = new CastInst (CI, GEP->getType(), GEP->getName(), InsertPt);

  Value::use_iterator UI = GEP->use_begin();
  for (; UI != GEP->use_end(); ++UI) {
    if (((*UI) != CI) && ((*UI) != ResultPointer))
      UI->replaceUsesOfWith (GEP, CI);
  }
#endif

  // Update statistics
  ++ExactChecks;
  return;
}

//
// Function: addExactCheck3()
//
// Description:
//  Utility routine that inserts a call to exactcheck3().
//
// Inputs:
//  Source   - The pointer that is the lower bound of the array.
//  Result   - The pointer that we wish to check.
//  Bounds   - The length of the array in bytes.
//  InsertPt - The instruction before which to insert the check.
//
Value *
InsertPoolChecks::addExactCheck3 (Value * Source,
                                  Value * Result,
                                  Value * Bounds,
                                  Instruction * InsertPt) {
  // The LLVM type for a void *
  Type *VoidPtrType = PointerType::get(Type::SByteTy); 

  // Update statistics
  ++ExactChecks;

  //
  // Cast the operands to the correct type.
  //
  Value * BasePointer = Source;
  if (BasePointer->getType() != VoidPtrType)
    BasePointer = new CastInst(BasePointer, VoidPtrType,
                               BasePointer->getName()+".ec3.base.casted",
                               InsertPt);

  Value * ResultPointer = Result;
  if (ResultPointer->getType() != VoidPtrType)
    ResultPointer = new CastInst(Result, VoidPtrType,
                                 Result->getName()+".ec3.result.casted",
                                 InsertPt);

  Value * CastBounds = Bounds;
  if (Bounds->getType() != VoidPtrType)
    CastBounds = new CastInst(Bounds, VoidPtrType,
                              Bounds->getName()+".ec3.end.casted", InsertPt);

  std::vector<Value *> args(1, BasePointer);
  args.push_back(ResultPointer);
  args.push_back(CastBounds);

  //
  // Record that this value was checked.
  //
  CheckedValues.insert (Result);
  return new CallInst(ExactCheck3, args, "ec3", InsertPt);
}

//
// Function: addExactCheck()
//
// Description:
//  Utility routine that inserts a call to exactcheck().  This function can
//  perform some optimization be determining if the arguments are constant.
//  If they are, we can forego inserting the call.
//
// Inputs:
//  Index - An LLVM Value representing the index of the access.
//  Bounds - An LLVM Value representing the bounds of the check.
//
void
InsertPoolChecks::addExactCheck (Value * Pointer,
                                 Value * Index, Value * Bounds,
                                 Instruction * InsertPt) {
  //
  // Record that this value was checked.
  //
  CheckedValues.insert (Pointer);

  //
  // Attempt to determine statically if this check will always pass; if so,
  // then don't bother doing it at run-time.
  //
  ConstantInt * CIndex  = dyn_cast<ConstantInt>(Index);
  ConstantInt * CBounds = dyn_cast<ConstantInt>(Bounds);
  if (CIndex && CBounds) {
    int index  = CIndex->getSExtValue();
    int bounds = CBounds->getSExtValue();
    assert ((index >= 0) && "exactcheck: const negative index");
    assert ((index < bounds) && "exactcheck: const out of range");

    // Update stats and return
    ++ConstExactChecks;
    return;
  }

  //
  // Second, cast the operands to the correct type.
  //
  Value * CastIndex = Index;
  if (Index->getType() != Type::IntTy)
    CastIndex = new CastInst(Index, Type::IntTy,
                             Index->getName()+".ec.casted", InsertPt);

  Value * CastBounds = Bounds;
  if (Bounds->getType() != Type::IntTy)
    CastBounds = new CastInst(Bounds, Type::IntTy,
                              Bounds->getName()+".ec.casted", InsertPt);

  const Type *VoidPtrType = PointerType::get(Type::SByteTy); 
  Value * CastResult = Pointer;
  if (CastResult->getType() != VoidPtrType)
    CastResult = new CastInst(CastResult, VoidPtrType,
                              CastResult->getName()+".ec.casted", InsertPt);

  std::vector<Value *> args(1, CastIndex);
  args.push_back(CastBounds);
  args.push_back(CastResult);
  Instruction * CI = new CallInst(ExactCheck, args, "ec", InsertPt);

#if 0
  //
  // Replace the old index with the return value of exactcheck(); this
  // prevents GCC from removing it completely.
  //
  Value * CastCI = CI;
  if (CI->getType() != GEP->getType())
    CastCI = new CastInst (CI, GEP->getType(), GEP->getName(), InsertPt);

  Value::use_iterator UI = GEP->use_begin();
  for (; UI != GEP->use_end(); ++UI) {
    if (((*UI) != CI) && ((*UI) != CastResult))
      UI->replaceUsesOfWith (GEP, CastCI);
  }
#endif

  // Update statistics
  ++ExactChecks;
  return;
}

//
// Function: addExactCheck()
//
// Description:
//  Utility routine that inserts a call to exactcheck().  This function can
//  perform some optimization be determining if the arguments are constant.
//  If they are, we can forego inserting the call.
//
// Inputs:
//  GEP - The GEP for which the check will be done.
//  Index - An LLVM Value representing the index of the access.
//  Bounds - An LLVM Value representing the bounds of the check.
//
void
InsertPoolChecks::addExactCheck (Instruction * GEP,
                                 Value * Index, Value * Bounds) {
  //
  // Record that this value was checked.
  //
  CheckedValues.insert (GEP);

  // Upper and lower values on the index and bounds
  int index_lower;
  int index_upper;
  int bounds_lower;
  int bounds_upper;

  //
  // First, attempt to determine statically whether the check will always pass.
  // If so, then don't bother doing it at run-time.
  //
  ConstantInt * CIndex  = dyn_cast<ConstantInt>(Index);
  ConstantInt * CBounds = dyn_cast<ConstantInt>(Bounds);
  if (CIndex && CBounds) {
    int index  = CIndex->getSExtValue();
    int bounds = CBounds->getSExtValue();
    assert ((index >= 0) && "exactcheck: const negative index");
    assert ((index < bounds) && "exactcheck: const out of range");

    // Update stats and return
    ++ConstExactChecks;
    return;
  } else if (CBounds) {
    int bounds = CBounds->getSExtValue();
    SCEVHandle SCEVIndex  = scevPass->getSCEV (Index);
    ConstantRange IndexRange  = SCEVIndex->getValueRange();
    if (!(IndexRange.isWrappedSet())) {
      int index_lower = IndexRange.getLower()->getSExtValue();
      int index_upper = IndexRange.getUpper()->getSExtValue();
      int bounds_lower = bounds;
      int bounds_upper = bounds;

      if ((index_lower >= 0) && (index_upper < bounds_lower)) {
        ++ConstExactChecks;
        return;
      }
    }
  }

  SCEVHandle SCEVIndex  = scevPass->getSCEV (Index);
  SCEVHandle SCEVBounds = scevPass->getSCEV (Bounds);
  ConstantRange BoundsRange = SCEVBounds->getValueRange();
  ConstantRange IndexRange  = SCEVIndex->getValueRange();
  if ((!(IndexRange.isWrappedSet())) && (!(BoundsRange.isWrappedSet()))) {
    int index_lower = IndexRange.getLower()->getSExtValue();
    int index_upper = IndexRange.getUpper()->getSExtValue();
    int bounds_lower = BoundsRange.getLower()->getSExtValue();
    int bounds_upper = BoundsRange.getUpper()->getSExtValue();

    if ((index_lower >= 0) && (index_upper < bounds_lower)) {
      ++ConstExactChecks;
      return;
    }
  }

  //
  // Cast the operands to the correct type.
  //
  Instruction * InsertPt = GEP->getNext();

  Value * CastIndex = Index;
  if (Index->getType() != Type::IntTy)
    CastIndex = new CastInst(Index, Type::IntTy,
                             Index->getName()+".ec.casted", InsertPt);

  Value * CastBounds = Bounds;
  if (Bounds->getType() != Type::IntTy)
    CastBounds = new CastInst(Bounds, Type::IntTy,
                              Bounds->getName()+".ec.casted", InsertPt);

  const Type *VoidPtrType = PointerType::get(Type::SByteTy); 
  Value * CastResult = GEP;
  if (CastResult->getType() != VoidPtrType)
    CastResult = new CastInst(CastResult, VoidPtrType,
                              CastResult->getName()+".ec.casted", InsertPt);

  std::vector<Value *> args(1, CastIndex);
  args.push_back(CastBounds);
  args.push_back(CastResult);
  Instruction * CI = new CallInst(ExactCheck, args, "ec", InsertPt);

  //
  // Replace the old index with the return value of exactcheck(); this
  // prevents GCC from removing it completely.
  //
#if 0
  Value * CastCI = CI;
  if (CI->getType() != GEP->getType())
    CastCI = new CastInst (CI, GEP->getType(), GEP->getName(), InsertPt);

  Value::use_iterator UI = GEP->use_begin();
  for (; UI != GEP->use_end(); ++UI) {
    if (((*UI) != CI) && ((*UI) != CastResult))
      UI->replaceUsesOfWith (GEP, CastCI);
  }
#endif

  // Update statistics
  ++ExactChecks;
  return;
}

//
// Function: isEligableForExactCheck()
//
// Return value:
//  true  - This value is eligable for an exactcheck.
//  false - This value is not eligable for an exactcheck.
//
static inline bool
isEligableForExactCheck (Value * Pointer) {
  if ((isa<AllocaInst>(Pointer)) || (isa<GlobalVariable>(Pointer)))
    return true;

  if (CallInst* CI = dyn_cast<CallInst>(Pointer)) {
    if (CI->getCalledFunction() &&
        (CI->getCalledFunction()->getName() == "__vmalloc" || 
         CI->getCalledFunction()->getName() == "kmalloc" || 
         CI->getCalledFunction()->getName() == "kmem_cache_alloc" || 
         CI->getCalledFunction()->getName() == "__alloc_bootmem")) {
      return true;
    }
  }

  return false;
}

//
// Function: findCheckedPointer()
//
// Description:
//  Given a pointer value, attempt to determine whether the pointer or all of
//  the instructions that created it have been checked.
//
bool
InsertPoolChecks::findCheckedPointer (Value * PointerOperand) {
  Value * SourcePointer = PointerOperand;

  while (CheckedValues.find (SourcePointer) == CheckedValues.end()) {
    // Check for cast constant expressions and instructions
    if (ConstantExpr * cExpr = dyn_cast<ConstantExpr>(SourcePointer)) {
      if (cExpr->getOpcode() == Instruction::Cast) {
        if (isa<PointerType>(cExpr->getOperand(0)->getType())) {
          SourcePointer = cExpr->getOperand(0);
          continue;
        }
      }
      // We cannot handle this expression; break out of the loop
      break;
    }

    if (CastInst * CastI = dyn_cast<CastInst>(SourcePointer)) {
      if (isa<PointerType>(CastI->getOperand(0)->getType())) {
        SourcePointer = CastI->getOperand(0);
        continue;
      }
      break;
    }

    // We can't scan through any more instructions; give up
    break;
  }

  //
  // If the pointer is a GEP, then as long as it has a DSNode, it has been
  // checked or proven safe.
  //
  Instruction * I;
  if ((I = dyn_cast<GetElementPtrInst>(SourcePointer)) &&
     (getDSNode (I, I->getParent()->getParent())) &&
     (getPoolHandle(I, I->getParent()->getParent()))) {
    return true;
  }
#if 0
  return (CheckedValues.find (SourcePointer) != CheckedValues.end());
#else
  // Return false; check must dominate load/store.
  return false;
#endif
}

//
// Function: findSourcePointer()
//
// Description:
//  Given a pointer value, attempt to find a source of the pointer that can
//  be used in an exactcheck().
//
// Outputs:
//  indexed - Flags whether the data flow went through a indexing operation
//            (i.e. a GEP).  This value is always written.
//
static Value *
findSourcePointer (Value * PointerOperand, bool & indexed) {
  //
  // Attempt to look for the originally allocated object by scanning the data
  // flow up.
  //
  indexed = false;
  Value * SourcePointer = PointerOperand;
  Value * OldSourcePointer = 0;
  while (!isEligableForExactCheck (SourcePointer)) {
    assert (OldSourcePointer != SourcePointer);
    OldSourcePointer = SourcePointer;
    // Check for GEP and cast constant expressions
    if (ConstantExpr * cExpr = dyn_cast<ConstantExpr>(SourcePointer)) {
      if ((cExpr->getOpcode() == Instruction::Cast) ||
          (cExpr->getOpcode() == Instruction::GetElementPtr)) {
        if (isa<PointerType>(cExpr->getOperand(0)->getType())) {
          SourcePointer = cExpr->getOperand(0);
          continue;
        }
      }
      // We cannot handle this expression; break out of the loop
      break;
    }

    // Check for GEP and cast instructions
    if (GetElementPtrInst * G = dyn_cast<GetElementPtrInst>(SourcePointer)) {
      SourcePointer = G->getPointerOperand();
      indexed = true;
      continue;
    }

    if (CastInst * CastI = dyn_cast<CastInst>(SourcePointer)) {
      if (isa<PointerType>(CastI->getOperand(0)->getType())) {
        SourcePointer = CastI->getOperand(0);
        continue;
      }
      break;
    }

    // Check for call instructions to exact checks.
    CallInst * CI1;
    if ((CI1 = dyn_cast<CallInst>(SourcePointer)) &&
        (CI1->getCalledFunction()) &&
        (CI1->getCalledFunction()->getName() == "exactcheck3")) {
      SourcePointer = CI1->getOperand (2);
      continue;
    }

    // We can't scan through any more instructions; give up
    break;
  }

  if (isEligableForExactCheck (SourcePointer))
    PointerOperand = SourcePointer;

  return PointerOperand;
}

//
// Function: insertExactCheck()
//
// Description:
//  Attepts to insert an efficient, accurate array bounds check for the given
//  GEP instruction; this check will not use Pools are MetaPools.
//
// Return value:
//  true  - An exactcheck() was successfully added.
//  false - An exactcheck() could not be added; a more extensive check will be
//          needed.
//
bool
InsertPoolChecks::insertExactCheck (GetElementPtrInst * GEP) {
  // The GEP instruction casted to the correct type
  Instruction *Casted = GEP;

  // The pointer operand of the GEP expression
  Value * PointerOperand = GEP->getPointerOperand();

  //
  // Get the DSNode for the instruction
  //
  Function *F   = GEP->getParent()->getParent();
  DSGraph & TDG = getDSGraph(*F);
  DSNode * Node = TDG.getNodeForValue(GEP).getNode();
  assert (Node && "boundscheck: DSNode is NULL!");

  //
  // Determine whether an alignment check is needed.  This occurs when a DSNode
  // is type unknown (collapsed) but has pointers to type known (uncollapsed)
  // DSNodes.
  //
  if (preSCPass->nodeNeedsAlignment (Node)) {
    ++AlignChecks;
  }

#if 0
  // Debugging: See if we're missing exactcheck opportunities
  if (isa<SelectInst>(PointerOperand))
    std::cerr << "LLVA: EC: Select Inst\n";

  if (isa<GetElementPtrInst>(PointerOperand))
    std::cerr << "LLVA: EC: GEP: In " << F->getName() << "\n";

  CallInst * CI1;
  if ((CI1 = dyn_cast<CallInst>(PointerOperand)) &&
      (CI1->getCalledFunction()) &&
      (CI1->getCalledFunction()->getName() == "exactcheck3"))
    std::cerr << "LLVA: EC: GEP: Hidden by ec3: In " << F->getName() << "\n";

  PHINode * PN = 0;
  if (PN = dyn_cast<PHINode>(PointerOperand)) {
    std::cerr << "LLVA: EC: PHI\n";
    for (unsigned index = 0; index < PN->getNumIncomingValues(); ++index) {
      std::cerr << "LLVA: " << *(PN->getIncomingValue(index)) << std::endl;
    }
  }
#endif

  //
  // Attempt to find the object which we need to check.
  //
  bool WasIndexed = true;
  PointerOperand = findSourcePointer (PointerOperand, WasIndexed);

  //
  // Ensure the pointer operand really is a pointer.
  //
  if (!isa<PointerType>(PointerOperand->getType()))
    return false;

  if (GlobalVariable *GV = dyn_cast<GlobalVariable>(PointerOperand)) {
    //
    // Attempt to remove checks on GEPs that only index into structures.
    // These criteria must be met:
    //  1) The pool must be Type-Homogoneous.
    //
#if 0
    if ((!(Node->isNodeCompletelyFolded())) &&
        (indexesStructsOnly (GEP))) {
      ++StructGEPsRemoved;
      return true;
    }
#endif

    //
    // Attempt to use a call to exactcheck() to check this value if it is a
    // global array with a non-zero size.  We do not check zero length arrays
    // because in C they are often used to declare an external array of unknown
    // size as follows:
    //        extern struct foo the_array[];
    //
    const ArrayType *AT = dyn_cast<ArrayType>(GV->getType()->getElementType());
    if ((!WasIndexed) && AT && (AT->getNumElements())) {
      // we need to insert an actual check
      // It could be a select instruction
      // First get the size
      // This only works for one or two dimensional arrays
      if (GEP->getNumOperands() == 2) {
        Value *secOp = GEP->getOperand(1);

        const Type* csiType = Type::getPrimitiveType(Type::IntTyID);
        ConstantInt * Bounds = ConstantInt::get(csiType,AT->getNumElements());
        addExactCheck (GEP, secOp, Bounds);
        return true;
      } else if (GEP->getNumOperands() == 3) {
        if (ConstantInt *COP = dyn_cast<ConstantInt>(GEP->getOperand(1))) {
          //FIXME assuming that the first array index is 0
          assert((COP->getZExtValue() == 0) && "non zero array index\n");

          Value * secOp = GEP->getOperand(2);
          const Type* csiType = Type::getPrimitiveType(Type::IntTyID);
          ConstantInt * Bounds = ConstantInt::get(csiType,AT->getNumElements());
          addExactCheck (GEP, secOp, Bounds);
          return true;
        } else {
          // TODO:
          //  Handle non constant index two dimensional arrays later
          Value* Size=ConstantInt::get(Type::IntTy, TD->getTypeSize(GV->getType()));
          addExactCheck2 (PointerOperand, GEP, Size, GEP->getNext());
          return true;
        }
      } else {
        // Handle Multi dimensional cases later
        Value* AllocSize=ConstantInt::get(Type::IntTy, TD->getTypeSize(GV->getType()->getElementType()));
        addExactCheck2 (PointerOperand, GEP, AllocSize, GEP->getNext());
        return true;
        GEP->dump();
        ++MissedMultDimArrayChecks;
      }
      DEBUG(std::cerr << " Global variable ok \n");
    } else {
      Value* Size=ConstantInt::get(Type::IntTy, TD->getTypeSize(GV->getType()));
      addExactCheck2 (PointerOperand, GEP, Size, GEP->getNext());
      return true;
    }
  }

  //
  // If the pointer was generated by a dominating alloca instruction, we can
  // do an exactcheck on it, too.
  //
  if (AllocaInst *AI = dyn_cast<AllocaInst>(PointerOperand)) {
    //
    // Attempt to remove checks on GEPs that only index into structures.
    // These criteria must be met:
    //  1) The pool must be Type-Homogoneous.
    //
#if 0
    if ((!(Node->isNodeCompletelyFolded())) &&
        (indexesStructsOnly (GEP))) {
      ++StructGEPsRemoved;
      return true;
    }
#endif

    const Type * AllocaType = AI->getAllocatedType();
    Value *AllocSize=ConstantInt::get(Type::IntTy, TD->getTypeSize(AllocaType));

    if (AI->isArrayAllocation())
      AllocSize = BinaryOperator::create(Instruction::Mul,
                                         AllocSize,
                                         AI->getOperand(0), "sizetmp", GEP);

    addExactCheck2 (PointerOperand, GEP, AllocSize, GEP->getNext());
    return true;
  }

  //
  // If the pointer was an allocation, we should be able to do exact checks
  //
  CallInst* CI = dyn_cast<CallInst>(PointerOperand);
  if (CI && (CI->getCalledFunction())) {
    if ((CI->getCalledFunction()->getName() == "__vmalloc") || 
        (CI->getCalledFunction()->getName() == "kmalloc") || 
        (CI->getCalledFunction()->getName() == "__alloc_bootmem")) {
      //
      // Attempt to remove checks on GEPs that only index into structures.
      // These criteria must be met:
      //  1) The pool must be Type-Homogoneous.
      //
#if 0
      if ((!(Node->isNodeCompletelyFolded())) &&
          (indexesStructsOnly (GEP))) {
        ++StructGEPsRemoved;
        return true;
      }
#endif

      Value* Cast = new CastInst(CI->getOperand(1), Type::IntTy, "", GEP);
      addExactCheck2(PointerOperand, GEP, Cast, GEP->getNext());
      return true;
    } else if (CI->getCalledFunction()->getName() == "kmem_cache_alloc") {
      // Insert a call to kmem_get_cachesize() to determine the size of the
      // allocated object
      const Type * VoidPtrType = PointerType::get(Type::SByteTy);
      Value* VP  = castTo (CI->getOperand(1), VoidPtrType, GEP);
      CallInst *len = new CallInst(KmemCachegetSize, VP, "", GEP);

      // Add the exactcheck
      Value* CastLen = castTo (len, Type::IntTy, GEP);
      addExactCheck2(PointerOperand, GEP, CastLen, GEP->getNext());
    }
  }

  //
  // If the pointer is to a structure, we may be able to perform a simple
  // exactcheck on it, too, unless the array is at the end of the structure.
  // Then, we assume it's a variable length array and must be full checked.
  //
#if 0
  if (const PointerType * PT = dyn_cast<PointerType>(PointerOperand->getType()))
    if (const StructType *ST = dyn_cast<StructType>(PT->getElementType())) {
      const Type * CurrentType = ST;
      ConstantInt * C;
      for (unsigned index = 2; index < GEP->getNumOperands() - 1; ++index) {
        //
        // If this GEP operand is a constant, index down into the next type.
        //
        if (C = dyn_cast<ConstantInt>(GEP->getOperand(index))) {
          if (const StructType * ST2 = dyn_cast<StructType>(CurrentType)) {
            CurrentType = ST2->getElementType(C->getZExtValue());
            continue;
          }

          if (const ArrayType * AT = dyn_cast<ArrayType>(CurrentType)) {
            CurrentType = AT->getElementType();
            continue;
          }

          // We don't know how to handle this type of element
          break;
        }

        //
        // If the GEP operand is not constant and points to an array type,
        // then try to insert an exactcheck().
        //
        const ArrayType * AT;
        if ((AT = dyn_cast<ArrayType>(CurrentType)) && (AT->getNumElements())) {
          const Type* csiType = Type::getPrimitiveType(Type::IntTyID);
          ConstantInt * Bounds = ConstantInt::get(csiType,AT->getNumElements());
          addExactCheck (GEP, GEP->getOperand (index), Bounds);
          return true;
        }
      }
    }
#endif

  /*
   * We were not able to insert a call to exactcheck().
   */
  return false;
}

//
// Function: insertExactCheck()
//
// Description:
//  Attepts to insert an efficient, accurate array bounds check for the given
//  GEP instruction; this check will not use Pools are MetaPools.
//
// Inputs:
//  I        - The instruction for which we are adding the check.
//  Src      - The pointer that needs to be checked.
//  Size     - The size, in bytes, that will be read/written by instruction I.
//  InsertPt - The instruction before which the check should be inserted.
//
// Return value:
//  true  - An exactcheck() was successfully added.
//  false - An exactcheck() could not be added; a more extensive check will be
//          needed.
//
bool
InsertPoolChecks::insertExactCheck (Instruction * I,
                                    Value * Src,
                                    Value * Size,
                                    Instruction * InsertPt) {
  // The pointer operand of the GEP expression
  Value * PointerOperand = Src;

  //
  // Get the DSNode for the instruction
  //
#if 1
  Function *F   = I->getParent()->getParent();
  DSGraph & TDG = getDSGraph(*F);
  DSNode * Node = TDG.getNodeForValue(I).getNode();
  if (!Node)
    return false;
#endif

  //
  // Determine whether an alignment check is needed.  This occurs when a DSNode
  // is type unknown (collapsed) but has pointers to type known (uncollapsed)
  // DSNodes.
  //
  if (preSCPass->nodeNeedsAlignment (Node)) {
    ++AlignChecks;
  }

  //
  // Attempt to find the original object for which this check applies.
  // This involves unpeeling casts, GEPs, etc.
  //
  bool WasIndexed = true;
  PointerOperand = findSourcePointer (PointerOperand, WasIndexed);

  //
  // Ensure the pointer operand really is a pointer.
  //
  if (!isa<PointerType>(PointerOperand->getType()))
  {
    return false;
  }

  //
  // Attempt to use a call to exactcheck() to check this value if it is a
  // global array with a non-zero size.  We do not check zero length arrays
  // because in C they are often used to declare an external array of unknown
  // size as follows:
  //        extern struct foo the_array[];
  //
  if (GlobalVariable *GV = dyn_cast<GlobalVariable>(PointerOperand)) {
    const Type* csiType = Type::getPrimitiveType(Type::IntTyID);
    unsigned int arraysize = TD->getTypeSize(GV->getType()->getElementType());
    ConstantInt * Bounds = ConstantInt::get(csiType, arraysize);
    if (WasIndexed)
      addExactCheck2 (PointerOperand, Src, Bounds, InsertPt);
    else
      addExactCheck (Src, Size, Bounds, InsertPt);
    return true;
  }

  //
  // If the pointer was generated by a dominating alloca instruction, we can
  // do an exactcheck on it, too.
  //
  if (AllocaInst *AI = dyn_cast<AllocaInst>(PointerOperand)) {
    const Type * AllocaType = AI->getAllocatedType();
    Value *AllocSize=ConstantInt::get(Type::IntTy, TD->getTypeSize(AllocaType));

    if (AI->isArrayAllocation())
      AllocSize = BinaryOperator::create(Instruction::Mul,
                                         AllocSize,
                                         AI->getOperand(0), "allocsize", InsertPt);

    if (WasIndexed)
      addExactCheck2 (PointerOperand, Src, AllocSize, InsertPt);
    else
      addExactCheck (Src, Size, AllocSize, InsertPt);
    return true;
  }

  //
  // If the pointer was an allocation, we should be able to do exact checks
  //
  if(CallInst* CI = dyn_cast<CallInst>(PointerOperand)) {
    if (CI->getCalledFunction() && (
                              CI->getCalledFunction()->getName() == "__vmalloc" || 
                              CI->getCalledFunction()->getName() == "kmalloc")) {
      Value* Cast = new CastInst(CI->getOperand(1), Type::IntTy, "allocsize", InsertPt);
      if (WasIndexed)
        addExactCheck2 (PointerOperand, Src, Cast, InsertPt);
      else
        addExactCheck (Src, Size, Cast, InsertPt);
      return true;
    }
  }

  //
  // We were not able to insert a call to exactcheck().
  //
  return false;
}

bool
InsertPoolChecks::doInitialization (Module & M) {
  // Add the new poolcheck prototype 
  addPoolCheckProto (M);

  return true;
}

bool
InsertPoolChecks::doFinalization (Module & M) {
  // Insert code to register heap allocations with the correct MetaPool
  // and to register kernel pools with the correct MetaPool
  addHeapRegs (M);

  // Insert code to drop objects from MetaPools when they are freed by the
  // kernel allocators
  addObjFrees (M);

  return true;
}

bool
InsertPoolChecks::runOnFunction (Function & F) {
  //
  // Retrieve references to all of the passes from which we will gather
  // information.
  //
  preSCPass = &getAnalysis<PreInsertPoolChecks>();
  cuaPass   = &getAnalysis<ConvertUnsafeAllocas>();
  TD        = &getAnalysis<TargetData>();
  scevPass  = &getAnalysis<ScalarEvolution>();
  LI        = &getAnalysis<LoopInfo>();
#ifdef LLVA_KERNEL  
  TDPass  = &getAnalysis<TDDataStructures>();
#else
  paPass    = &getAnalysis<PoolAllocate>();
  equivPass = &(paPass->getECGraphs());
  efPass    = &getAnalysis<EmbeCFreeRemoval>();
  TD        = &getAnalysis<TargetData>();
#endif

#if 1
  // Transform the function
  if (!(F.isExternal())) TransformFunction (F);
  if (!DisableLSChecks)  addLoadStoreChecks(F);
#endif

  //
  // Update the statistics.
  //
  PoolChecks = NullChecks + FullChecks;
  
  return true;
}

void InsertPoolChecks::handleCallInst(CallInst *CI) {
  //
  // Determine if this is an indirect call.  If so, insert a run-time check for
  // it.
  //
  if (!(CI->getCalledFunction()))
    insertFunctionCheck (CI);

  //
  // Check for intrinsic functions and add checks as necessary.
  //
  if (CI && (!DisableIntrinsicChecks)) {
    Value *Fop = CI->getOperand(0);
    Function *F = CI->getParent()->getParent();
#ifdef LLVA_KERNEL    
    std::string FuncName = Fop->getName();
    if ((FuncName == "llvm.memcpy.i32")    || 
        (FuncName == "llvm.memcpy.i64")    ||
        (FuncName == "llvm.memset.i32")    ||
        (FuncName == "llvm.memset.i64")    ||
        (FuncName == "llvm.memmove.i32")   ||
        (FuncName == "llvm.memmove.i64")   ||
        (FuncName == "llva_memcpy")        ||
        (FuncName == "llva_memset")        ||
        (FuncName == "llva_strncpy")       ||
        (FuncName == "llva_invokememcpy")  ||
        (FuncName == "llva_invokestrncpy") ||
        (FuncName == "llva_invokememset")  ||
        (FuncName == "memcmp")) {
      //
      // Create a call to an accurate bounds check for each string parameter.
      //
      Instruction *InsertPt = CI;
      Value * CastPointer1 = castTo (CI->getOperand(1), Type::UIntTy, InsertPt);
      Value * CastPointer2 = castTo (CI->getOperand(2), Type::UIntTy, InsertPt);
      Value * CastCIOp3    = castTo (CI->getOperand(3), Type::UIntTy, InsertPt);

      Instruction *Bop1 = BinaryOperator::create(Instruction::Add, CastPointer1,
                                                 CastCIOp3, "mcadd",InsertPt);
      Instruction *Bop2 = BinaryOperator::create(Instruction::Add, CastPointer2,
                                                 CastCIOp3, "mcadd",InsertPt);
      Instruction *Finalp1 = BinaryOperator::create(Instruction::Sub, Bop1,
                                                 ConstantInt::get (Type::UIntTy, 1), "mcsub",InsertPt);
      Instruction *Finalp2 = BinaryOperator::create(Instruction::Sub, Bop2,
                                                 ConstantInt::get (Type::UIntTy, 1), "mcsub",InsertPt);

      Instruction *Length = BinaryOperator::create(Instruction::Sub, CastCIOp3,
                                                ConstantInt::get (Type::UIntTy, 1), "mclen",InsertPt);
      AddedValues.insert(CastPointer1);
      AddedValues.insert(CastPointer2);
      AddedValues.insert(CastCIOp3);
      AddedValues.insert(Bop1);
      AddedValues.insert(Bop2);
      AddedValues.insert(Finalp1);
      AddedValues.insert(Finalp2);
      AddedValues.insert(Length);

      // Create the call to do an accurate bounds check
      if (!insertExactCheck(CI, CI->getOperand(1), Length, InsertPt))
        insertBoundsCheck (CI, CI->getOperand(1), Bop1, InsertPt);
      if (!insertExactCheck(CI, CI->getOperand(2), Length, InsertPt))
        insertBoundsCheck (CI, CI->getOperand(2), Bop2, InsertPt);
#if 0
    } else if ((FuncName == "llva_load_integer") ||
               (FuncName == "llva_save_integer") ||
               (FuncName == "llva_load_integer_stackp") ||
               (FuncName == "llva_save_integer_stackp") ||
               (FuncName == "llva_push_function1")) {
      //
      // Create a call to an accurate bounds check for the integer state
      // pointer.
      //
      Instruction *InsertPt = CI;
      Value * CastPointer1 = castTo (CI->getOperand(1), Type::UIntTy, InsertPt);
      Value * CastLength   = ConstantInt::get (Type::UIntTy, LLVA_INTEGERSTATE_SIZE);

      Instruction *Bop1 = BinaryOperator::create(Instruction::Add, CastPointer1,
                                                 CastLength, "mcadd",InsertPt);
      Value *Length = ConstantInt::get (Type::UIntTy, LLVA_INTEGERSTATE_SIZE - 1);

      // Create the call to do an accurate bounds check
      if (!insertExactCheck(CI, CI->getOperand(1), Length, InsertPt))
        insertBoundsCheck (CI, CI->getOperand(1), Bop1, InsertPt);
    } else if ((FuncName == "llva_load_icontext") ||
               (FuncName == "llva_save_icontext")) {
      //
      // Perform a check on the interrupt context.
      //
      Instruction *InsertPt = CI;
      insertICCheck (CI->getOperand(1), InsertPt);

      //
      // Create a call to an accurate bounds check for the integer state.
      //
      Value * CastPointer1 = castTo (CI->getOperand(2), Type::UIntTy, InsertPt);
      Value * CastLength   = ConstantInt::get (Type::UIntTy, LLVA_ICONTEXT_SIZE);

      Instruction *Bop1 = BinaryOperator::create(Instruction::Add, CastPointer1,
                                                 CastLength, "mcadd",InsertPt);
      Value *Length = ConstantInt::get (Type::UIntTy, LLVA_ICONTEXT_SIZE - 1);

      // Create the call to do an accurate bounds check
      if (!insertExactCheck(CI, CI->getOperand(2), Length, InsertPt))
        insertBoundsCheck (CI, CI->getOperand(1), Bop1, InsertPt);
    } else if ((FuncName == "llva_load_fp") || (FuncName == "llva_save_fp")) {
      //
      // Create a call to an accurate bounds check for the FP state
      // pointer.
      //
      Instruction *InsertPt = CI;
      Value * CastPointer1 = castTo (CI->getOperand(1), Type::UIntTy, InsertPt);
      Value * CastLength   = ConstantInt::get (Type::UIntTy, LLVA_FPSTATE_SIZE);

      Instruction *Bop1 = BinaryOperator::create(Instruction::Add, CastPointer1,
                                                 CastLength, "mcadd",InsertPt);
      Value *Length = ConstantInt::get (Type::UIntTy, LLVA_FPSTATE_SIZE - 1);

      // Create the call to do an accurate bounds check
      if (!insertExactCheck(CI, CI->getOperand(1), Length, InsertPt))
        insertBoundsCheck (CI, CI->getOperand(1), Bop1, InsertPt);
    } else if (FuncName == "llva_push_syscall") {
      //
      // Create a call to an accurate bounds check for the interrupt context
      // pointer.
      //
      Instruction *InsertPt = CI;
      Value * CastPointer1 = castTo (CI->getOperand(2), Type::UIntTy, InsertPt);
      Value * CastLength   = ConstantInt::get (Type::UIntTy, LLVA_ICONTEXT_SIZE);

      Instruction *Bop1 = BinaryOperator::create(Instruction::Add, CastPointer1,
                                                 CastLength, "mcadd",InsertPt);
      Value *Length = ConstantInt::get (Type::UIntTy, LLVA_ICONTEXT_SIZE - 1);

      // Create the call to do an accurate bounds check
      if (!insertExactCheck(CI, CI->getOperand(1), Length, InsertPt))
        insertBoundsCheck (CI, CI->getOperand(1), Bop1, InsertPt);
    } else if ((FuncName == "llva_init_icontext") ||
               (FuncName == "llva_clear_icontext") ||
               (FuncName == "llva_was_privileged") ||
               (FuncName == "llva_icontext_lif") ||
               (FuncName == "llva_ipop_function0") ||
               (FuncName == "llva_ipush_function0") ||
               (FuncName == "llva_ipush_function1") ||
               (FuncName == "llva_ipush_function3") ||
               (FuncName == "llva_ialloca") ||
               (FuncName == "llva_unwind") ||
               (FuncName == "llva_icontext_load_retvalue") ||
               (FuncName == "llva_icontext_save_retvalue") ||
               (FuncName == "llva_get_icontext_stackp") ||
               (FuncName == "llva_set_icontext_stackp") ||
               (FuncName == "llva_iset_privileged")) {
      //
      // Create a call to an accurate bounds check for the interrupt context
      // pointer.
      //
      Instruction *InsertPt = CI;
      insertICCheck (CI->getOperand(1), InsertPt);
#endif
    }
#endif    
  }
}

void InsertPoolChecks::simplifyGEPList() {
#if 0
  std::set<Instruction *> & UnsafeGetElemPtrs = cuaPass->getUnsafeGetElementPtrsFromABC();
  std::map< std::pair<Value*, BasicBlock*>, std::set<Instruction*> > m;
  for (std::set<Instruction *>::iterator ii = UnsafeGetElemPtrs.begin(), ee = UnsafeGetElemPtrs.end();
       ii != ee; ++ii) {
    GetElementPtrInst* GEP = cast<GetElementPtrInst>(*ii);
    m[std::make_pair(GEP->getOperand(0), GEP->getParent())].insert(GEP);
  }

  unsigned singletons;
  unsigned multi;
  for (std::map< std::pair<Value*,BasicBlock*>, std::set<Instruction*> >::iterator ii = m.begin(), ee = m.end();
       ii != ee; ++ii) {
    if (ii->second.size() > 1) {
      std::cerr << "##############\n";
      for (std::set<Instruction *>::iterator i = ii->second.begin(), e = ii->second.end();
       i != e; ++i) {
        (*i)->dump();
      }
      std::cerr << "##############\n";
      ++multi;
    } else
      ++singletons;
  }

  std::cerr << "Singletons: " << singletons << " Multitons: " << multi << "\n";

#endif
}

//
// Function: compatibleGEPs()
//
// Description:
//  Determine whether two GEPs address the same memory and have the (save the
//  for last) same arguments.
//
// Return value:
//  true  - All arguments (except possibly the last) are identical.
//  false - One or more arguments besides the last argument are different.
//
static inline bool
identicalGEPs (GetElementPtrInst * GEP1, GetElementPtrInst * GEP2) {
  if (GEP1->getPointerOperand() != GEP2->getPointerOperand()) return false;
  if (GEP1->getNumOperands()    != GEP2->getNumOperands())    return false;
  for (unsigned index = 0; index < GEP1->getNumOperands() - 1; ++index) {
    if (GEP1->getOperand(index) != GEP2->getOperand(index)) {
      return false;
      break;
    }
  }

  return true;
}

bool
InsertPoolChecks::AggregateGEPs (GetElementPtrInst * MAI,
                                 std::set<Instruction *> & RelatedGEPs) {
  // Get the set of unsafe GEPs for this instruction's basic block
  std::set<Instruction *> * UnsafeGetElemPtrs = cuaPass->getUnsafeGetElementPtrsFromABC(MAI->getParent());
  if (!UnsafeGetElemPtrs) {
    RelatedGEPs.insert (MAI);
    return true;
  }

  // If this GEP has already been deemed safe, then return;
  std::set<Instruction *>::const_iterator iCurrent = UnsafeGetElemPtrs->find(MAI);
  if (iCurrent == UnsafeGetElemPtrs->end()) {
    return true;
  }

  // Determine whether the GEP has a constant as its last index.
  // If so, search for all other GEPs that have, save for the last index,
  // identical operands.  Then select the two with the higest and lowest
  // indices; we will perform our run-time checks on these.
  std::set<Instruction *>::iterator UGI = UnsafeGetElemPtrs->begin(),
                                    UGE = UnsafeGetElemPtrs->end();
  Instruction * MinGEP = MAI;
  Instruction * MaxGEP = MAI;
  if (isa<ConstantInt>(MAI->getOperand((MAI->getNumOperands() - 1)))) {

    for (;UGI != UGE; ++UGI) {
      Instruction * I = *UGI;
      GetElementPtrInst * GEP = dyn_cast<GetElementPtrInst>(I);
      if (!GEP) continue;
      if (!(isa<ConstantInt>(GEP->getOperand((GEP->getNumOperands() - 1))))) continue;
      if (identicalGEPs (GEP, MAI)) {
        ConstantInt * GEPLastIndex;
        ConstantInt * MinLastIndex;
        ConstantInt * MaxLastIndex;
        GEPLastIndex = dyn_cast<ConstantInt>(GEP->getOperand(GEP->getNumOperands() - 1));
        MinLastIndex = dyn_cast<ConstantInt>(MinGEP->getOperand(MinGEP->getNumOperands() - 1));
        MaxLastIndex = dyn_cast<ConstantInt>(MaxGEP->getOperand(MaxGEP->getNumOperands() - 1));
        if (GEPLastIndex->getSExtValue() < MinLastIndex->getSExtValue()) {
          UnsafeGetElemPtrs->erase (MinGEP);
          MinGEP = GEP;
        }
        if (GEPLastIndex->getSExtValue() > MaxLastIndex->getSExtValue()) {
          UnsafeGetElemPtrs->erase (MaxGEP);
          MaxGEP = GEP;
        }
      }
    }

    //
    // Find the first compatible GEP in the basic block.
    //
    Instruction * Ins = MAI->getParent()->getFirstNonPHI();
    GetElementPtrInst * FirstGEP = 0;
    while (!(Ins->isTerminator())) {
      if (FirstGEP = dyn_cast<GetElementPtrInst>(Ins)) {
        if (identicalGEPs (MAI, FirstGEP))
          break;
      }
      Ins = Ins->getNext();
    }

    //
    // Move the minimum and maximum GEPs to before the first identical GEP.
    //
    if (FirstGEP != MinGEP) {
      MinGEP->moveBefore (FirstGEP);
    }
    if (FirstGEP != MaxGEP) {
      MaxGEP->moveBefore (FirstGEP);
    }
  }

  RelatedGEPs.insert (MinGEP);
  if (MinGEP != MaxGEP) RelatedGEPs.insert (MaxGEP);

  return true;
}

void InsertPoolChecks::handleGetElementPtr(GetElementPtrInst *MAI) {
  // Get the set of unsafe GEP instructions from the array bounds check pass
  // If this instruction is not within that set, then the result of the GEP
  // instruction has been proven safe, and there is no need to insert a check.
  std::set<Instruction *> * UnsafeGetElemPtrs = cuaPass->getUnsafeGetElementPtrsFromABC(MAI->getParent());
  if (!UnsafeGetElemPtrs) return;
  std::set<Instruction *>::const_iterator iCurrent = UnsafeGetElemPtrs->find(MAI);
  if (iCurrent == UnsafeGetElemPtrs->end()) {
#if 0
    std::cerr << "statically proved safe : Not inserting checks " << *MAI << "\n";
#endif
    return;
  }

#if 0
  // Find all unsafe GEPs within the basic block that are identical save for
  // their last index
  std::set<Instruction *> RelatedGEPs;
  std::set<Instruction *>::iterator UGI = UnsafeGetElemPtrs->begin(),
                                    UGE = UnsafeGetElemPtrs->end();
  if (isa<ConstantInt>(MAI->getOperand((MAI->getNumOperands() - 1))))
    for (;UGI != UGE; ++UGI) {
      Instruction * I = *UGI;
      GetElementPtrInst * GEP = dyn_cast<GetElementPtrInst>(I);
      if (!GEP) continue;
      if (GEP->getPointerOperand() != MAI->getPointerOperand()) continue;
      if (GEP->getNumOperands() != MAI->getNumOperands()) continue;
      if (!(isa<ConstantInt>(GEP->getOperand((GEP->getNumOperands() - 1))))) continue;
      bool identical = true;
      for (unsigned index = 0; index < GEP->getNumOperands() - 1; ++index) {
        if (GEP->getOperand(index) != MAI->getOperand(index)) {
          identical = false;
          break;
        }
      }
      if (identical)
        RelatedGEPs.insert (GEP);
    }

  std::cerr << "LLVA: RelatedGEP: " << RelatedGEPs.size() << std::endl;
#endif

  if (InsertPoolChecksForArrays) {
    Function *F = MAI->getParent()->getParent();
    GetElementPtrInst *GEP = MAI;
    // Now we need to decide if we need to pass in the alignmnet
    //for the poolcheck
    //    if (getDSNodeOffset(GEP->getPointerOperand(), F)) {
    //          std::cerr << " we don't handle middle of structs yet\n";
    //assert(!getDSNodeOffset(GEP->getPointerOperand(), F) && " we don't handle middle of structs yet\n");
    //       ++MissChecks;
    //       continue;
    //     }

#ifndef LLVA_KERNEL
    PA::FuncInfo *FI = paPass->getFuncInfoOrClone(*F);
    Instruction *Casted = GEP;
    if (!FI->ValueMap.empty()) {
      assert(FI->ValueMap.count(GEP) && "Instruction not in the value map \n");
      Instruction *temp = dyn_cast<Instruction>(FI->ValueMap[GEP]);
      assert(temp && " Instruction  not there in the Value map");
      Casted  = temp;
    }
    if (GetElementPtrInst *GEPNew = dyn_cast<GetElementPtrInst>(Casted)) {
      Value *PH = getPoolHandle(GEP, F, *FI);
      if (PH && isa<ConstantPointerNull>(PH)) return;
      if (!PH) {
        Value *PointerOperand = GEPNew->getPointerOperand();
        if (ConstantExpr *cExpr = dyn_cast<ConstantExpr>(PointerOperand)) {
          if (cExpr->getOpcode() == Instruction::Cast)
            PointerOperand = cExpr->getOperand(0);
        }
        if (GlobalVariable *GV = dyn_cast<GlobalVariable>(PointerOperand)) {
          if (const ArrayType *AT = dyn_cast<ArrayType>(GV->getType()->getElementType())) {
            // We need to insert an actual check.  It could be a select
            // instruction.
            // First get the size.
            // This only works for one or two dimensional arrays.
            if (GEPNew->getNumOperands() == 2) {
              Value *secOp = GEPNew->getOperand(1);
              if (secOp->getType() != Type::IntTy) {
                secOp = new CastInst(secOp, Type::IntTy,
                                     secOp->getName()+".ec.casted", Casted);
              }

              const Type* csiType = Type::getPrimitiveType(Type::IntTyID);
              std::vector<Value *> args(1,secOp);
              args.push_back(ConstantInt::get(csiType,AT->getNumElements()));
              new CallInst(ExactCheck,args,"", Casted);
              DEBUG(std::cerr << "Inserted exact check call Instruction \n");
              return;
            } else if (GEPNew->getNumOperands() == 3) {
              if (ConstantInt *COP = dyn_cast<ConstantInt>(GEPNew->getOperand(1))) {
                // FIXME: assuming that the first array index is 0
                assert((COP->getZExtValue() == 0) && "non zero array index\n");
                Value * secOp = GEPNew->getOperand(2);
                if (secOp->getType() != Type::IntTy) {
                  secOp = new CastInst(secOp, Type::IntTy,
                                       secOp->getName()+".ec2.casted", Casted);
                }
                std::vector<Value *> args(1,secOp);
                const Type* csiType = Type::getPrimitiveType(Type::IntTyID);
                args.push_back(ConstantInt::get(csiType,AT->getNumElements()));
                new CallInst(ExactCheck, args, "", Casted->getNext());
                return;
              } else {
                // Handle non constant index two dimensional arrays later
                abort();
              }
            } else {
              // Handle Multi dimensional cases later
              DEBUG(std::cerr << "WARNING: Handle multi dimensional globals later\n");
              (*iCurrent)->dump();
            }
          }
          DEBUG(std::cerr << " Global variable ok \n");
        }

        //      These must be real unknowns and they will be handled anyway
        //      std::cerr << " WARNING, DID NOT HANDLE   \n";
        //      (*iCurrent)->dump();
        return;
      } else {
        if (Casted->getType() != PointerType::get(Type::SByteTy)) {
          Casted = new CastInst(Casted,PointerType::get(Type::SByteTy),
                                (Casted)->getName()+".pc.casted",
                                (Casted)->getNext());
        }
        std::vector<Value *> args(1, PH);
        args.push_back(Casted);
        // Insert it
        new CallInst(PoolCheck,args, "",Casted->getNext());
        DEBUG(std::cerr << "inserted instrcution \n");
      }
    }
#else
    //
    // Get the pool handle associated with the pointer operand.
    //
    Value *PH = getPoolHandle(GEP->getPointerOperand(), F);
    GetElementPtrInst *GEPNew = GEP;
    Instruction *Casted = GEP;

    DSGraph & TDG = getDSGraph(*F);
    DSNode * Node = TDG.getNodeForValue(GEP).getNode();

    DEBUG(std::cerr << "LLVA: addGEPChecks: Pool " << PH << " Node ");
    DEBUG(std::cerr << Node << std::endl);

    Value *PointerOperand = GEPNew->getPointerOperand();
    if (ConstantExpr *cExpr = dyn_cast<ConstantExpr>(PointerOperand)) {
      if (cExpr->getOpcode() == Instruction::Cast)
        PointerOperand = cExpr->getOperand(0);
    }
    if (GlobalVariable *GV = dyn_cast<GlobalVariable>(PointerOperand)) {
      if (const ArrayType *AT = dyn_cast<ArrayType>(GV->getType()->getElementType())) {
        // we need to insert an actual check
        // It could be a select instruction
        // First get the size
        // This only works for one or two dimensional arrays
        if (GEPNew->getNumOperands() == 2) {
          Value *secOp = GEPNew->getOperand(1);
#ifdef LLVA_KERNEL
          //
          // Determine whether the exactcheck() will have constant integer
          // arguments.  If so, then we can evaluate them statically and avoid
          // inserting the run-time check.
          //
          if (ConstantInt * Index = dyn_cast<ConstantInt>(secOp)) {
            int index = Index->getSExtValue();
            assert ((index < 0) && "exactcheck will fail at runtime");
            if (index < AT->getNumElements())
              return;
            assert (0 && "exactcheck out of range");
          }
#endif
          if (secOp->getType() != Type::IntTy) {
            secOp = new CastInst(secOp, Type::IntTy,
                                 secOp->getName()+".ec3.casted", Casted);
          }
          
          std::vector<Value *> args(1,secOp);
          const Type* csiType = Type::getPrimitiveType(Type::IntTyID);
          args.push_back(ConstantInt::get(csiType,AT->getNumElements()));
          new CallInst(ExactCheck,args,"", Casted);
          //	    DEBUG(std::cerr << "Inserted exact check call Instruction \n");
          return;
        } else if (GEPNew->getNumOperands() == 3) {
          if (ConstantInt *COP = dyn_cast<ConstantInt>(GEPNew->getOperand(1))) {
            //FIXME assuming that the first array index is 0
            assert((COP->getZExtValue() == 0) && "non zero array index\n");
            Value * secOp = GEPNew->getOperand(2);
#ifdef LLVA_KERNEL
            //
            // Determine whether the exactcheck() will have constant integer
            // arguments.  If so, then we can evaluate them statically and avoid
            // inserting the run-time check.
            //
            if (ConstantInt * Index = dyn_cast<ConstantInt>(secOp)) {
              int index = Index->getSExtValue();
              assert ((index < 0) && "exactcheck will fail at runtime");
              if (index < AT->getNumElements())
                return;
              assert (0 && "exactcheck out of range");
            }
#endif
            if (secOp->getType() != Type::IntTy) {
              secOp = new CastInst(secOp, Type::IntTy,
                                   secOp->getName()+".ec4.casted", Casted);
            }
            std::vector<Value *> args(1,secOp);
            const Type* csiType = Type::getPrimitiveType(Type::IntTyID);
            args.push_back(ConstantInt::get(csiType,AT->getNumElements()));
            new CallInst(ExactCheck,args,"", Casted->getNext());
            return;
          } else {
            //Handle non constant index two dimensional arrays later
            abort();
          }
        } else {
          //Handle Multi dimensional cases later
          std::cerr << "WARNING: Handle multi dimensional globals later\n";
          (*iCurrent)->dump();
          ++MissedMultDimArrayChecks;
        }
        DEBUG(std::cerr << " Global variable ok \n");
      }
    }

    //
    // We cannot insert an exactcheck().  Insert a pool check.
    //
    // 
    if (!PH) {
#if 0
      std::cerr << "missing a GEP check for" << *MAI << "alloca case?\n";
#endif
      ++NullChecks;
      if (!PH) ++MissedNullChecks;
      // Don't bother to insert the NULL check unless the user asked
      if (!EnableNullChecks)
        return;
      PH = Constant::getNullValue(PointerType::get(Type::SByteTy));
    } else {
      assert ((isa<GlobalValue>(PH)) && "MetaPool Handle is not a global!");
    }

    //
    // If this is a complete node, insert a poolcheck.
    // If this is an icomplete node, insert a poolcheckarray.
    //
    Instruction *InsertPt = Casted->getNext();
    if (Casted->getType() != PointerType::get(Type::SByteTy)) {
      Casted = new CastInst(Casted,PointerType::get(Type::SByteTy),
                            (Casted)->getName()+".pc2.casted",InsertPt);
    }
    Instruction *CastedPointerOperand = new CastInst(PointerOperand,
                                                     PointerType::get(Type::SByteTy),
                                                     PointerOperand->getName()+".casted",InsertPt);
    Instruction *CastedPH = new CastInst(PH,
                                         PointerType::get(Type::SByteTy),
                                         "ph",InsertPt);
    if (Node->isIncomplete()) {
      std::vector<Value *> args(1, CastedPH);
      args.push_back(CastedPointerOperand);
      args.push_back(Casted);
      CallInst(PoolCheckArray,args, "",InsertPt);
    } else {
      std::vector<Value *> args(1, CastedPH);
      args.push_back(Casted);
      new CallInst(PoolCheck,args, "",InsertPt);
    }
#endif
  } else {
    // Insert accurate bounds checks for arrays (as opposed to poolchecks)

    //
    // Attempt to insert a standard exactcheck() call for the GEP.
    //
    if (insertExactCheck (MAI))
      return;

    //Exact poolchecks
    if (const PointerType *PT = dyn_cast<PointerType>(MAI->getPointerOperand()->getType())) {
#if 0
      if (const StructType *ST = dyn_cast<StructType>(PT->getElementType())) {
#else
      const StructType *ST = dyn_cast<StructType>(PT->getElementType());
      if (0) {
#endif
        //It is a struct type with pointers
        //for each pointer with struct typ
        //we need to watchg out for arrays inside structs 
        if (ConstantInt *COP = dyn_cast<ConstantInt>(MAI->getOperand(1))) {
          //FIXME assuming that the first index is safe
          //assert((COP->getRawValue() == 0) && "non zero array index\n");
          bool allconstant = true;
          for (unsigned i = 2; i < MAI->getNumOperands(); ++i) {
            if (!isa<Constant>(MAI->getOperand(i))) {
              allconstant = false;
              break;
            }
          }
          if (!allconstant) {
            if (MAI->getNumOperands() == 4) {
              if (ConstantInt *COPi = dyn_cast<ConstantInt>(MAI->getOperand(2))) {
                const Type *stel = ST->getElementType(COPi->getZExtValue());
                if (const ArrayType *elAT = dyn_cast<ArrayType>(stel)) {
                  //Need to factor this in to separate method!!!
                  Value * secOp = MAI->getOperand(3);
                  Value *indexTypeSize = ConstantInt::get(Type::UIntTy, TD->getTypeSize(secOp->getType())); 
#ifdef LLVA_KERNEL
                  //
                  // Determine whether the exactcheck() will have constant integer
                  // arguments.  If so, then we can evaluate them statically and avoid
                  // inserting the run-time check.
                  //
                  if (ConstantInt * Index = dyn_cast<ConstantInt>(secOp)) {
                    int index = Index->getSExtValue();
                    assert ((index < 0) && "exactcheck will fail at runtime");
                    if (index < TD->getTypeSize(elAT))
                      return;
                    assert (0 && "exactcheck out of range");
                  }
#endif
                  if (secOp->getType() != Type::IntTy) {
                    secOp = new CastInst(secOp, Type::IntTy,
                                         secOp->getName()+".casted",  MAI);
                  }
                  //                      secOp = BinaryOperator::create(Instruction::Mul, indexTypeSize, secOp,"indextmp", MAI);
                  std::vector<Value *> args(1,secOp);
                  Value *AllocSize =
                    ConstantInt::get(Type::IntTy, TD->getTypeSize(elAT));
                  /*
                    if (AI->isArrayAllocation())
                    AllocSize = BinaryOperator::create(Instruction::Mul,
                    AllocSize,
                    AI->getOperand(0), "sizetmp", MAI);
                  */
                  //		      args.push_back(ConstantInt::get(csiType,elAT->getNumElements()));
                  args.push_back(AllocSize);
                  new CallInst(ExactCheck,args,"",MAI);
			
                } else {
                  //non array, non constant value
                  abort();
                }
              } else {
                // non-constant value, not possible
                abort();
              }
            } else {
#if 0
              //FIXME this is a less precise check than possible
              //		  std::cerr << "WARNING : did not handle array within a struct precisely, num operands != 4\n";
              Instruction *InsertPt = MAI->getNext();
              Type *VoidPtrType = PointerType::get(Type::SByteTy); 
              Value *MAIPSbyte =  new CastInst(MAI->getPointerOperand(),
                                               VoidPtrType, 
                                               MAI->getPointerOperand()->getName()+".casted",InsertPt);
                  
              Value *MAISbyte =  new CastInst(MAI,
                                              VoidPtrType, 
                                              MAI->getName()+".casted",InsertPt);
                  
              std::vector<Value *> args(1,MAIPSbyte);
              args.push_back(MAISbyte);
              Value *AllocSize =
                ConstantInt::get(Type::UIntTy, TD->getTypeSize(ST));
              args.push_back(AllocSize);
              new CallInst(ExactCheck2,args,"",InsertPt);
#endif
            }
          } else {
            std::cerr << "Not all args constant: " << *MAI << std::endl;
          }
        } else {
          std::cerr << "HandleLikeArray: " << *MAI << std::endl;
          goto HandleThisLikeArray;
        }
      } else {
      HandleThisLikeArray:	      
        if (AllocaInst *AI = dyn_cast<AllocaInst>(MAI->getPointerOperand())) {
          //we can put an exact check here
          if (1 || (MAI->getNumOperands() == 3)) {
            if (ConstantInt *COP = dyn_cast<ConstantInt>(MAI->getOperand(1))) {
              //FIXME assuming that the first array index is 0
#if 0
              assert((COP->getZExtValue() == 0) && "non zero array index\n");
#else
              if (COP->getZExtValue() == 0) {
#endif
              Value * secOp = MAI->getOperand(2);
              Value *indexTypeSize = ConstantInt::get(Type::UIntTy, TD->getTypeSize(secOp->getType())); 
#ifdef LLVA_KERNEL
              //
              // Determine whether the exactcheck() will have constant integer
              // arguments.  If so, then we can evaluate them statically and avoid
              // inserting the run-time check.
              //
              if (!(AI->isArrayAllocation()))
                if (ConstantInt * Index = dyn_cast<ConstantInt>(secOp)) {
                  int index = Index->getSExtValue();
                  if ((index > 0) && (index < TD->getTypeSize(AI->getAllocatedType())))
                    return;
                }
#endif
              //Convert everything to bytes
              if (secOp->getType() != Type::IntTy) {
                secOp = new CastInst(secOp, Type::IntTy, secOp->getName()+".casted",
                                     MAI);
              }
              //		  secOp = BinaryOperator::create(Instruction::Mul, indexTypeSize, secOp,"indextmp",MAI);
              std::vector<Value *> args(1,secOp);
              Value *AllocSize =
                ConstantInt::get(Type::IntTy, TD->getTypeSize(AI->getAllocatedType()));
		  
              if (AI->isArrayAllocation())
                AllocSize = BinaryOperator::create(Instruction::Mul,
                                                   AllocSize,
                                                   AI->getOperand(0), "sizetmp", MAI);
		  
              args.push_back(AllocSize);
              CallInst *newCI = new CallInst(ExactCheck,args,"", MAI);
              } else {
                std::cerr << "COP not zero: " << *MAI << std::endl;
              }
            } else {
              std::cerr << "Bad COP: " << *MAI << std::endl;
            }
          } else {
            std::cerr << " num operands != 3: " << *MAI << std::endl;
            abort();
          }
        } else {
          // Now check if the GEP is inside a loop with monotonically increasing
          //loop bounds
          //We use the LoopInfo Pass this
          Loop *L = LI->getLoopFor(MAI->getParent());
          bool monotonicOpt = false;
          if (L && (MAI->getNumOperands() == 2)) {
            bool HasConstantItCount = isa<SCEVConstant>(scevPass->getIterationCount(L));
            Value *vIndex = MAI->getOperand(1);
            if (Instruction *Index = dyn_cast<Instruction>(vIndex)) {
              //If it is not an instruction then it must already be loop invariant
              if (L->isLoopInvariant(MAI->getPointerOperand())) { 
                SCEVHandle SH = scevPass->getSCEV(Index);
                if (SH->hasComputableLoopEvolution(L) ||    // Varies predictably
                    HasConstantItCount) {
                  if (SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(SH))
                    if (AR->isAffine()) {
                      SCEVHandle EntryValue = AR->getStart();
                      //                      EntryValue->getValueRange().dump();
                      //                      Index->dump();
                      SCEVHandle ExitValue = scevPass->getSCEVAtScope(Index, L->getParentLoop());
                      BasicBlock *Preheader = L->getLoopPreheader();
                      if (!isa<SCEVCouldNotCompute>(ExitValue)) {
                        SCEVExpander Rewriter(*scevPass, *LI);
                        Instruction *ptIns = Preheader->getTerminator();
                        Value *NewVal = Rewriter.expandCodeFor(ExitValue, ptIns,
                                                               Index->getType());
                        //                        NewVal->dump();
                        if (!isa<SCEVCouldNotCompute>(EntryValue)) {
                          Value *NewVal2 = Rewriter.expandCodeFor(EntryValue, ptIns,
                                                                  Index->getType());
                          //                          NewVal2->dump();
                          //Inserted the values now insert GEPs and add checks
                          std::vector<Value *> gepargs1(1,NewVal);
                          GetElementPtrInst *GEPUpper =
                            new GetElementPtrInst(MAI->getPointerOperand(), gepargs1, MAI->getName()+"upbc", ptIns);
                          insertBoundsCheck (MAI, GEPUpper->getPointerOperand(), GEPUpper, ptIns);
                          std::vector<Value *> gepargs2(1,NewVal2);
                          GetElementPtrInst *GEPLower =
                            new GetElementPtrInst(MAI->getPointerOperand(), gepargs2, MAI->getName()+"lobc", ptIns);
                          insertBoundsCheck (MAI, GEPLower->getPointerOperand(), GEPLower, ptIns);
                          monotonicOpt = true;
                          ++MonotonicOpts;
                        }
                      }
                    }
                }
              }
            }
          }
          if (!monotonicOpt) {
            //
            // Insert a bounds check and use its return value in all subsequent
            // uses.
            //
            Instruction *nextIns = MAI->getNext();
            insertBoundsCheck (MAI, MAI->getPointerOperand(), MAI, nextIns);
          }
        }
      }
    } else {
      std::cerr << "GEP does not have pointer type arg" << *MAI << std::endl;
      abort();
    }
  }
}

  void InsertPoolChecks::addGetActualValue(SetCondInst *SCI, unsigned operand) {
    //we know that the operand is a pointer type 
    Value *op = SCI->getOperand(operand);
    Function *F = SCI->getParent()->getParent();
#ifndef LLVA_KERNEL    
#ifdef NOEQUIV
    if (!equivPass->ContainsDSGraphFor(*F)) {
      //some times the ECGraphs doesnt contain F
      //for newly created cloned functions
      PA::FuncInfo *FI = paPass->getFuncInfoOrClone(*F);
      op = FI->MapValueToOriginal(op);
      if (!op) return; //abort();
    }
#endif    
#endif    
    Function *Fnew = F;
    Value *PH = 0;
    if (Argument *arg = dyn_cast<Argument>(op)) {
      Fnew = arg->getParent();
      PH = getPoolHandle(op, Fnew);
    } else if (Instruction *Inst = dyn_cast<Instruction>(op)) {
      Fnew = Inst->getParent()->getParent();
      PH = getPoolHandle(op, Fnew);
    } else if (isa<Constant>(op)) {
      return;
      //      abort();
    } else if (!isa<ConstantPointerNull>(op)) {
      //has to be a global
      abort();
    }
    op = SCI->getOperand(operand);
    if (!isa<ConstantPointerNull>(op)) {
      if (PH) {
	if (1) { //HACK fixed
	  Type *VoidPtrType = PointerType::get(Type::SByteTy); 
	  Value *PHVptr =  new CastInst(PH, VoidPtrType,
					PH->getName()+".casted",  SCI);
	  Value *OpVptr = op;
	  if (op->getType() != VoidPtrType)
	    OpVptr = new CastInst(op, VoidPtrType,
				  op->getName()+".casted",  SCI);
	  
	  std::vector<Value *> args = make_vector(PHVptr, OpVptr,0);
	  CallInst *CI = new CallInst(GetActualValue, args,"getval", SCI);
	  CastInst *CastBack = new CastInst(CI, op->getType(), op->getName()+".castback",SCI);
	  SCI->setOperand(operand, CastBack);
	}
      } else {
	//It shouldn't work if PH is not null
      }
    }
  }

void InsertPoolChecks::TransformFunction (Function & F) {
#ifndef LLVA_KERNEL  
  PA::FuncInfo * PAFI = paPass->getFuncInfoOrClone(F);
  if (PAFI->Clone && PAFI->Clone != &F) {
    //no need to transform
    return;
  }
#endif  
  inst_iterator I = inst_begin(F);
  while (I != inst_end(F)) {
    Instruction *iLocal = &*I;
    if (SetCondInst *SCI = dyn_cast<SetCondInst>(iLocal)) {
      //If it is neq, eq,
      if ((SCI->getOpcode() == BinaryOperator::SetEQ) || (SCI->getOpcode() == BinaryOperator::SetNE)) {
        //for all the pointer operands replace them by the getactualvalue
        assert((SCI->getNumOperands() == 2) && "nmber of operands for SCI different from 2 ");
        if (isa<PointerType>(SCI->getOperand(0)->getType())) {
          //we need to insert a call to getactualvalue
          //First get the poolhandle for the pointer
          // TODO: We don't have a working getactualvalue(), so don't waste
          // time calling it.
#if 0
          if ((!isa<ConstantPointerNull>(SCI->getOperand(0))) && (!isa<ConstantPointerNull>(SCI->getOperand(1)))) {
            addGetActualValue(SCI, 0);
            addGetActualValue(SCI, 1);
          }
#endif
        }
      }
    } else if (isa<CastInst>(iLocal)) {
      //Some times the getelementptr is an argument of cast instruction
      // and we don't want to miss a run-time check there
      if (isa<GetElementPtrInst>(iLocal->getOperand(0))) {
        iLocal = cast<Instruction>(iLocal->getOperand(0));
      }
    } //

    // we need to handle
    // alloca instructions
    // getelement ptrs
    // load store checks 
    if (isa<GetElementPtrInst>(iLocal)) {
      GetElementPtrInst *MAI = cast<GetElementPtrInst>(iLocal);
      ++I;
      std::set<Instruction *> CheckSet;
      AggregateGEPs (MAI, CheckSet);
      for (std::set<Instruction *>::iterator GEP = CheckSet.begin();
           GEP != CheckSet.end(); ++GEP) {
        Instruction * I = *GEP;
        GetElementPtrInst * GEPI = dyn_cast<GetElementPtrInst>(I);
        handleGetElementPtr(GEPI);
      }
      continue;
    } else if (AllocaInst *AI = dyn_cast<AllocaInst>(iLocal)) {
      AllocaInst * AIOrig = AI;
      if (!DisableStackChecks) {
#ifndef LLVA_KERNEL      
#ifdef NOEQUIV
        if (!equivPass->ContainsDSGraphFor(F)) {
          //some times the ECGraphs doesnt contain F
          //for newly created cloned functions
          PA::FuncInfo *FI = paPass->getFuncInfoOrClone(F);
          Value *temp = FI->MapValueToOriginal(AI);
          if (temp)
            AIOrig = dyn_cast<AllocaInst>(temp);
          else
            continue;
          assert(AIOrig && " Instruction not in value map (clone)\n");
        }
#endif       
#endif       
        ++I;
        registerAllocaInst(AI, AIOrig);
        continue;
      }
    } else if (CallInst *CI = dyn_cast<CallInst>(iLocal)) {
      ++I;
      handleCallInst(CI);
      continue;
    }

    //
    // Move to the next instruction.
    //
    ++I;
  } 
}


void InsertPoolChecks::registerAllocaInst(AllocaInst *AI, AllocaInst *AIOrig) {
  //
  // Get the pool handle for the node that this contributes to...
  //
  Function *FOrig  = AIOrig->getParent()->getParent();
  DSNode *Node = getDSNode(AIOrig, FOrig);
  if (!Node) return;
  assert ((Node->isAllocaNode()) && "DSNode for alloca is missing stack flag!");

  //
  // Only register the stack allocation if it may be the subject of a run-time
  // check.  This can only occur when the object is used like an array because:
  //  1) GEP checks are only done when accessing arrays.
  //  2) Load/Store checks are only done on collapsed nodes (which appear to be
  //     used like arrays).
  //
  if (!(Node->isArray()))
    return;

  //
  // Determine if any use (direct or indirect) escapes this function.  If not,
  // then none of the checks will consult the MetaPool, and we can forego
  // registering the alloca.
  //
  bool MustRegisterAlloca = false;
  std::vector<Value *> AllocaWorkList;
  AllocaWorkList.push_back (AI);
  while ((!MustRegisterAlloca) && (AllocaWorkList.size())) {
    Value * V = AllocaWorkList.back();
    AllocaWorkList.pop_back();
    Value::use_iterator UI = V->use_begin();
    for (; UI != V->use_end(); ++UI) {
      // We cannot handle PHI nodes or Select instructions
      if (isa<PHINode>(UI) || isa<SelectInst>(UI)) {
        MustRegisterAlloca = true;
        continue;
      }

      // The pointer escapes if it's stored to memory somewhere.
      StoreInst * SI;
      if ((SI = dyn_cast<StoreInst>(UI)) && (SI->getOperand(0) == V)) {
        MustRegisterAlloca = true;
        continue;
      }

      // GEP instructions are okay, but need to be added to the worklist
      if (isa<GetElementPtrInst>(UI)) {
        AllocaWorkList.push_back (*UI);
        continue;
      }

      // Cast instructions are okay as long as they cast to another pointer
      // type
      if (CastInst * CI = dyn_cast<CastInst>(UI)) {
        if (isa<PointerType>(CI->getType())) {
          AllocaWorkList.push_back (*UI);
          continue;
        } else {
          MustRegisterAlloca = true;
          continue;
        }
      }

      if (ConstantExpr *cExpr = dyn_cast<ConstantExpr>(UI)) {
        if (cExpr->getOpcode() == Instruction::Cast) {
          AllocaWorkList.push_back (*UI);
          continue;
        } else {
          MustRegisterAlloca = true;
          continue;
        }
      }

      CallInst * CI1;
      if (CI1 = dyn_cast<CallInst>(UI)) {
        if (!(CI1->getCalledFunction())) {
          MustRegisterAlloca = true;
          continue;
        }

        std::string FuncName = CI1->getCalledFunction()->getName();
        if (FuncName == "exactcheck3") {
          AllocaWorkList.push_back (*UI);
          continue;
        } else if ((FuncName == "llvm.memcpy.i32")    || 
                   (FuncName == "llvm.memcpy.i64")    ||
                   (FuncName == "llvm.memset.i32")    ||
                   (FuncName == "llvm.memset.i64")    ||
                   (FuncName == "llvm.memmove.i32")   ||
                   (FuncName == "llvm.memmove.i64")   ||
                   (FuncName == "llva_memcpy")        ||
                   (FuncName == "llva_memset")        ||
                   (FuncName == "llva_strncpy")       ||
                   (FuncName == "llva_invokememcpy")  ||
                   (FuncName == "llva_invokestrncpy") ||
                   (FuncName == "llva_invokememset")  ||
                   (FuncName == "memcmp")) {
           continue;
        } else {
          MustRegisterAlloca = true;
          continue;
        }
      }
    }
  }

  if (!MustRegisterAlloca) {
    ++SavedRegAllocs;
    return;
  }

  //
  // Insert the alloca registration.
  //
  Value *PH = getPoolHandle(AIOrig, FOrig);
  if (PH == 0 || isa<ConstantPointerNull>(PH)) return;

  Value *AllocSize =
    ConstantInt::get(Type::UIntTy, TD->getTypeSize(AI->getAllocatedType()));
  
  if (AI->isArrayAllocation())
    AllocSize = BinaryOperator::create(Instruction::Mul, AllocSize,
                                       AI->getOperand(0), "sizetmp", AI);

  // Insert object registration at the end of allocas.
  Instruction *iptI = AI->getNext();
  if (AI->getParent() == (&(AI->getParent()->getParent()->getEntryBlock()))) {
    BasicBlock::iterator InsertPt = AI->getParent()->begin();
    while (&(*(InsertPt)) != AI)
      ++InsertPt;
    while (isa<AllocaInst>(InsertPt))
      ++InsertPt;
    iptI = InsertPt;
  }

  //
  // Insert a call to register the object.
  //
  Instruction *Casted = new CastInst(AI, PointerType::get(Type::SByteTy),
                                     AI->getName()+".casted", iptI);
  Value *CastedPH = new CastInst(PH,
                                 PointerType::get(Type::SByteTy),
                                 "allocph",Casted);
  new CallInst(StackRegister,
               make_vector(CastedPH, Casted, AllocSize,0),
               "", iptI);

  //
  // Insert a call to unregister the object whenever the function can exit.
  //
  for (Function::iterator BB = AI->getParent()->getParent()->begin();
                          BB != AI->getParent()->getParent()->end();
                          ++BB) {
    iptI = BB->getTerminator();
    if (isa<ReturnInst>(iptI) || isa<UnwindInst>(iptI))
      new CallInst(StackFree,
                   make_vector(CastedPH, Casted, 0),
                   "", iptI);
  }

  // Update statistics
  ++StackRegisters;
}

//
// Method: insertAlignmentCheck()
//
// Description:
//  Insert an alignment check for the specified value.
//
void
InsertPoolChecks::insertAlignmentCheck (LoadInst * LI) {
  // Get the function containing the load instruction
  Function * F = LI->getParent()->getParent();

  // Get the DSNode for the result of the load instruction.  If it is type
  // unknown, then no alignment check is needed.
  DSNode * LoadResultNode = getDSNode (LI,F);
  if (!(LoadResultNode && (!(LoadResultNode->isNodeCompletelyFolded())))) {
    return;
  }

  //
  // Get the pool handle for the node.
  //
  Value *PH = getPoolHandle(LI, F);
  if (!PH) return;

  //
  // If the node is incomplete or unknown, then only perform the check if
  // checks to incomplete or unknown are allowed.
  //
  Function * ThePoolCheckFunction = PoolCheckAlign;
  if ((LoadResultNode->isUnknownNode()) || (LoadResultNode->isIncomplete())) {
    if (EnableUnknownChecks) {
      ThePoolCheckFunction = PoolCheckAlignUI;
    } else {
      ++MissedIncompleteChecks;
      return;
    }
  }

  //
  // A check is needed.  Scan through the links of the DSNode of the load's
  // pointer operand; we need to determine the offset for the alignment check.
  //
  DSNode * Node = getDSNode (LI->getPointerOperand(), F);
  if (!Node) return;
  for (unsigned i = 0 ; i < Node->getNumLinks(); ++i) {
    DSNodeHandle & LinkNode = Node->getLink(i);
    if (LinkNode.getNode() == LoadResultNode) {
      // Insertion point for this check is *after* the load.
      Instruction * InsertPt = LI->getNext();

      // Create instructions to cast the checked pointer and the checked pool
      // into sbyte pointers.
      Value *CastVI  = castTo (LI, PointerType::get(Type::SByteTy), InsertPt);
      Value *CastPHI = castTo (PH, PointerType::get(Type::SByteTy), InsertPt);

      // Create the call to poolcheck
      std::vector<Value *> args(1,CastPHI);
      args.push_back(CastVI);
      args.push_back (ConstantInt::get(Type::UIntTy, LinkNode.getOffset()));
      new CallInst (PoolCheckAlign,args, "", InsertPt);

      // Update the statistics
      ++AlignLSChecks;

      break;
    }
  }
}

#ifdef LLVA_KERNEL
//
// Method: addLSChecks()
//
// Description:
//  Insert a poolcheck() into the code for a load or store instruction.
//
void InsertPoolChecks::addLSChecks(Value *V, Instruction *I, Function *F) {
  // Get the DSNode for the pointer to check
  DSNode * Node = getDSNode (V,F);

  //
  // Do not perform any checks if there is no DSNode, if the node is not folded,
  // or if the node is incomplete.
  //
  if (!(Node && Node->isNodeCompletelyFolded()))
    return;

  //
  // If the node is not registered, don't bother to check it.
  //
  if (!(isNodeRegistered (Node)))
    return;

  //
  // We will perform checks on incomplete or unknown nodes, but we must accept
  // the possibility that the object will not be found.
  //
  Function * ThePoolCheckFunction = PoolCheck;
  if ((Node->isUnknownNode()) || (Node->isIncomplete())) {
    if (EnableUnknownChecks) {
      ThePoolCheckFunction = PoolCheckUI;
    } else {
      ++MissedIncompleteChecks;
      return;
    }
  }

  //
  // This may be a load instruction that loads a pointer that:
  //  1) Points to a type known pool, and
  //  2) Loaded from a type unknown pool
  //
  // If this is the case, we need to perform an alignment check on the result
  // of the load.  Do that here.
  //
  if (LoadInst * LI = dyn_cast<LoadInst>(I)) {
    insertAlignmentCheck (LI);
  }

  //  
  // Do not perform a load/store check if the pointer used for this operation
  // has already been checked.
  //
  if (findCheckedPointer(V)) {
    ++SavedPoolChecks;
    return;
  }

  // Get the pool handle associated with this pointer.  If there is no pool
  // handle, use a NULL pointer value and let the runtime deal with it.
  Value *PH = getPoolHandle(V, F);

  if (!PH) {
    // Update the number of poolchecks that won't do anything
    ++NullChecks;

    // Update the stats on why there will be no check
    ++MissedNullChecks;

    // Don't bother to insert the NULL check unless the user requested it
    if (!EnableNullChecks)
      return;
    PH = Constant::getNullValue(PointerType::get(Type::SByteTy));
  } else {
    // This will be a full check; update the stats.
    assert (isa<GlobalValue>(PH));
    ++FullChecks;
  }

  // Create instructions to cast the checked pointer and the checked pool
  // into sbyte pointers.
  Value *CastVI  = castTo (V, PointerType::get(Type::SByteTy), I);
  Value *CastPHI = castTo (PH, PointerType::get(Type::SByteTy), I);

  // Create the call to poolcheck
  std::vector<Value *> args(1,CastPHI);
  args.push_back(CastVI);
  new CallInst (ThePoolCheckFunction ,args, "", I);
}

void InsertPoolChecks::addLoadStoreChecks (Function & FR) {
  Function *F = &FR;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    if (LoadInst *LI = dyn_cast<LoadInst>(&*I)) {
      Value *P = LI->getPointerOperand();
      addLSChecks(P, LI, F);
    } else if (StoreInst *SI = dyn_cast<StoreInst>(&*I)) {
      Value *P = SI->getPointerOperand();
      addLSChecks(P, SI, F);
    } else if (CallInst * CI = dyn_cast<CallInst>(&*I)) {
      if (Function * CalledFunc = CI->getCalledFunction()) {
        if ((CalledFunc->getName() == "llva_atomic_compare_and_swap") ||
            (CalledFunc->getName() == "llva_atomic_cas_lw") ||
            (CalledFunc->getName() == "llva_atomic_cas_h") ||
            (CalledFunc->getName() == "llva_atomic_cas_b") ||
            (CalledFunc->getName() == "llva_atomic_fetch_add_store") ||
            (CalledFunc->getName() == "llva_atomic_and") ||
            (CalledFunc->getName() == "llva_atomic_or")) {
          Value * P = CI->getOperand(1);
          addLSChecks (P, CI, F);
        }
      }
    }
  }
}
#else

void InsertPoolChecks::addLSChecks(Value *Vnew, const Value *V, Instruction *I, Function *F) {

  PA::FuncInfo *FI = paPass->getFuncInfoOrClone(*F);
  Value *PH = getPoolHandle(V, F, *FI );
  DSNode* Node = getDSNode(V, F);
  if (!PH) {
    return;
  } else {
    if (PH && isa<ConstantPointerNull>(PH)) {
      //we have a collapsed/Unknown pool
      Value *PH = getPoolHandle(V, F, *FI, true); 

      if (dyn_cast<CallInst>(I)) {
	// GEt the globals list corresponding to the node
	return;
	std::vector<Function *> FuncList;
	Node->addFullFunctionList(FuncList);
	std::vector<Function *>::iterator flI= FuncList.begin(), flE = FuncList.end();
	unsigned num = FuncList.size();
	if (flI != flE) {
	  const Type* csiType = Type::getPrimitiveType(Type::UIntTyID);
	  Value *NumArg = ConstantInt::get(csiType, num);	
					 
	  CastInst *CastVI = 
	    new CastInst(Vnew, 
			 PointerType::get(Type::SByteTy), "casted", I);
	
	  std::vector<Value *> args(1, NumArg);
	  args.push_back(CastVI);
	  for (; flI != flE ; ++flI) {
	    Function *func = *flI;
	    CastInst *CastfuncI = 
	      new CastInst(func, 
			   PointerType::get(Type::SByteTy), "casted", I);
	    args.push_back(CastfuncI);
	  }
	  new CallInst(FunctionCheck, args,"", I);
	}
      } else {


	CastInst *CastVI = 
	  new CastInst(Vnew, 
		       PointerType::get(Type::SByteTy), "casted", I);
	CastInst *CastPHI = 
	  new CastInst(PH, 
		       PointerType::get(Type::SByteTy), "casted", I);
	std::vector<Value *> args(1,CastPHI);
	args.push_back(CastVI);
	
	new CallInst(PoolCheck,args,"", I);
      }
    }
  }
}

void InsertPoolChecks::addLoadStoreChecks(Function & Func){
  Function *F = &Func;
  //here we check that we only do this on original functions
  //and not the cloned functions, the cloned functions may not have the
  //DSG
  bool isClonedFunc = false;
  if (paPass->getFuncInfo(*F))
    isClonedFunc = false;
  else
    isClonedFunc = true;
  Function *Forig = F;
  PA::FuncInfo *FI = paPass->getFuncInfoOrClone(*F);
  if (isClonedFunc) {
    Forig = paPass->getOrigFunctionFromClone(F);
  }
  //we got the original function

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    if (LoadInst *LI = dyn_cast<LoadInst>(&*I)) {
//we need to get the LI from the original function
Value *P = LI->getPointerOperand();
if (isClonedFunc) {
  assert(FI->NewToOldValueMap.count(LI) && " not in the value map \n");
  const LoadInst *temp = dyn_cast<LoadInst>(FI->NewToOldValueMap[LI]);
  assert(temp && " Instruction  not there in the NewToOldValue map");
  const Value *Ptr = temp->getPointerOperand();
  addLSChecks(P, Ptr, LI, Forig);
} else {
  addLSChecks(P, P, LI, Forig);
}
    } else if (StoreInst *SI = dyn_cast<StoreInst>(&*I)) {
Value *P = SI->getPointerOperand();
if (isClonedFunc) {
  assert(FI->NewToOldValueMap.count(SI) && " not in the value map \n");
  const StoreInst *temp = dyn_cast<StoreInst>(FI->NewToOldValueMap[SI]);
  assert(temp && " Instruction  not there in the NewToOldValue map");
  const Value *Ptr = temp->getPointerOperand();
  addLSChecks(P, Ptr, SI, Forig);
} else {
  addLSChecks(P, P, SI, Forig);
}
    } else if (CallInst *CI = dyn_cast<CallInst>(&*I)) {
Value *FunctionOp = CI->getOperand(0);
if (!isa<Function>(FunctionOp)) {
  if (isClonedFunc) {
    assert(FI->NewToOldValueMap.count(CI) && " not in the value map \n");
    const CallInst *temp = dyn_cast<CallInst>(FI->NewToOldValueMap[CI]);
    assert(temp && " Instruction  not there in the NewToOldValue map");
    const Value* FunctionOp1 = temp->getOperand(0);
    addLSChecks(FunctionOp, FunctionOp1, CI, Forig);
  } else {
    addLSChecks(FunctionOp, FunctionOp, CI, Forig);
  }
}
    } 
  }
}

#endif

//
// Method: getDSGraph()
//
// Description:
//  Return the DSGraph for the given function.  This method automatically
//  selects the correct pass to query for the graph based upon whether we're
//  doing user-space or kernel analysis.
//
DSGraph &
InsertPoolChecks::getDSGraph(Function & F) {
#ifndef LLVA_KERNEL
  return equivPass->getDSGraph(F);
#else  
  return TDPass->getDSGraph(F);
#endif  
}

DSNode* InsertPoolChecks::getDSNode(const Value *V, Function *F) {
  DSGraph &TDG = getDSGraph(*F);
  DSNode *DSN = TDG.getNodeForValue((Value *)V).getNode();
  return DSN;
}

unsigned InsertPoolChecks::getDSNodeOffset(const Value *V, Function *F) {
  DSGraph &TDG = getDSGraph(*F);
  return TDG.getNodeForValue((Value *)V).getOffset();
}

#ifndef LLVA_KERNEL
Value *InsertPoolChecks::getPoolHandle(const Value *V, Function *F, PA::FuncInfo &FI, bool collapsed) {
  const DSNode *Node = getDSNode(V,F);
  // Get the pool handle for this DSNode...
  //  assert(!Node->isUnknownNode() && "Unknown node \n");
  Type *VoidPtrType = PointerType::get(Type::SByteTy); 
  Type *PoolDescType = ArrayType::get(VoidPtrType, 50);
  Type *PoolDescPtrTy = PointerType::get(PoolDescType);
  if (!Node) {
    return 0; //0 means there is no isse with the value, otherwise there will be a callnode
  }
  if (Node->isUnknownNode()) {
    //FIXME this should be in a top down pass or propagated like collapsed pools below 
    if (!collapsed) {
      assert(!getDSNodeOffset(V, F) && " we don't handle middle of structs yet\n");
      return Constant::getNullValue(PoolDescPtrTy);
    }
  }
  std::map<const DSNode*, Value*>::iterator I = FI.PoolDescriptors.find(Node);
  map <Function *, set<Value *> > &
    CollapsedPoolPtrs = efPass->CollapsedPoolPtrs;
  
  if (I != FI.PoolDescriptors.end()) {
    // Check that the node pointed to by V in the TD DS graph is not
    // collapsed
    
    if (!collapsed && CollapsedPoolPtrs.count(F)) {
      Value *v = I->second;
      if (CollapsedPoolPtrs[F].find(I->second) != CollapsedPoolPtrs[F].end()) {
#ifdef DEBUG
        std::cerr << "Collapsed pools \n";
#endif
        return Constant::getNullValue(PoolDescPtrTy);
      } else {
        return v;
      }
    } else {
      return I->second;
    } 
  }
  return 0;
}

Value *
InsertPoolChecks::getPoolHandle(const Value *V, Function *F, bool collapsed) {
  const DSNode *Node = getDSNode(V,F);
  PA::FuncInfo * FI = paPass->getFuncInfoOrClone(*F);

  // Get the pool handle for this DSNode...
  //  assert(!Node->isUnknownNode() && "Unknown node \n");
  Type *VoidPtrType = PointerType::get(Type::SByteTy); 
  Type *PoolDescType = ArrayType::get(VoidPtrType, 50);
  Type *PoolDescPtrTy = PointerType::get(PoolDescType);
  if (!Node) {
    return 0; //0 means there is no isse with the value, otherwise there will be a callnode
  }
  if (Node->isUnknownNode()) {
    //FIXME this should be in a top down pass or propagated like collapsed pools below 
    if (!collapsed) {
      assert(!getDSNodeOffset(V, F) && " we don't handle middle of structs yet\n");
      return Constant::getNullValue(PoolDescPtrTy);
    }
  }
  std::map<const DSNode*, Value*>::iterator I = FI->PoolDescriptors.find(Node);
  map <Function *, set<Value *> > &
    CollapsedPoolPtrs = efPass->CollapsedPoolPtrs;
  
  if (I != FI->PoolDescriptors.end()) {
    // Check that the node pointed to by V in the TD DS graph is not
    // collapsed
    
    if (!collapsed && CollapsedPoolPtrs.count(F)) {
      Value *v = I->second;
      if (CollapsedPoolPtrs[F].find(I->second) != CollapsedPoolPtrs[F].end()) {
#ifdef DEBUG
        std::cerr << "Collapsed pools \n";
#endif
        return Constant::getNullValue(PoolDescPtrTy);
      } else {
        return v;
      }
    } else {
      return I->second;
    } 
  }
  return 0;
}
#else
Value *
InsertPoolChecks::getPoolHandle(const Value *V, Function *F) {
  DSGraph &TDG =  getDSGraph(*F);
  DSNode *Node = TDG.getNodeForValue((Value *)V).getNode();

  // Register that we will need allocations with this DSNode registered.
  if (Node)
    PHNeeded.insert (Node);

  // Get the pool handle for this DSNode...
  //  assert(!Node->isUnknownNode() && "Unknown node \n");
  //  if (Node->isUnknownNode()) {
  //    return 0;
  //  }
  return getPD (Node, *F->getParent());
}
#endif
