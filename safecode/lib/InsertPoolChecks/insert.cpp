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
#include "llvm/Instruction.h"
#include "llvm/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/ADT/VectorExtras.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"

#include <iostream>

using namespace llvm;

extern Value *getRepresentativeMetaPD(Value *);

RegisterPass<InsertPoolChecks> ipc("safecode", "insert runtime checks");

cl::opt<bool> InsertPoolChecksForArrays("boundschecks-usepoolchecks",
                cl::Hidden, cl::init(false),
                cl::desc("Insert pool checks instead of exact bounds checks"));
  
// Options for Enabling/Disabling the Insertion of Various Checks
cl::opt<bool> EnableIncompleteChecks  ("enable-incompletechecks", cl::Hidden,
                                cl::init(false),
                                cl::desc("Enable Checks on Incomplete Nodes"));

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

cl::opt<bool> DisableIntrinsicChecks ("disable-intrinchecks", cl::Hidden,
                                      cl::init(false),
                                      cl::desc("Disable Intrinsic Checks"));

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

static Statistic<> MissedStackChecks  ("safecode", "Missed stack checks");
static Statistic<> MissedGlobalChecks ("safecode", "Missed global checks");
static Statistic<> MissedNullChecks   ("safecode", "Missed PD checks");

// Exact Check Statistics
static Statistic<> ExactChecks        ("safecode", "Exactchecks inserted");
static Statistic<> ConstExactChecks   ("safecode", "Omitted Exactchecks with constant arguments");
 
//Kernel support rutines
static GlobalVariable* makeMetaPool(Module* M, DSNode* N) {
  //Here we insert a global meta pool
  //Now create a meta pool for this value, DSN Node
  const Type * VoidPtrType = PointerType::get(Type::SByteTy);
  std::vector<const Type*> MPTV;
  for (int x = 0; x < 8; ++x)
    MPTV.push_back(VoidPtrType);

  const StructType* MPT = StructType::get(MPTV);

  static int x = 0;
  std::string Name = "_metaPool_";
  if (N) {
    if(N->isUnknownNode())
      Name += "U";
    if(N->isIncomplete())
      Name += "I";
    if(N->isAllocaNode())
      Name += "A";
    if(N->isGlobalNode())
      Name += "G";
    if(N->isHeapNode())
      Name += "H";
    if(N->isNodeCompletelyFolded())
      Name += "F";
  }
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
    } else if (name == "kmalloc" ||
               name == "__vmalloc") {
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
  Function* KMF = M.getNamedFunction("kfree");
  Function* VMF = M.getNamedFunction("vfree");
  std::list<Function*> L;
  if (KMF) L.push_back(KMF);
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
  // Some times the ECGraphs doesnt contain F for newly created cloned
  // functions
  if (!equivPass->ContainsDSGraphFor(*F)) {
    PA::FuncInfo *FI = paPass->getFuncInfoOrClone(*F);
    newSrc = FI->MapValueToOriginal(MAI);
    assert(newSrc && " Instruction not in value map (clone)\n");
  }
  Function *Fnew = newSrc->getParent()->getParent();
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
  DSGraph & TDG = TDPass->getDSGraph(*F);
  DSNode * Node = TDG.getNodeForValue(I).getNode();

  //
  // Do not bother to insert checks for nodes that do not have the any of the
  // stack, heap, or global flags set.
  //
  if (!Node || !((Node->isAllocaNode()) ||
                 (Node->isHeapNode())   ||
                 (Node->isGlobalNode())))
  {
    ++NoSHGBoundsChecks;
    return Dest;
  }

  // Record statistics on the incomplete checks we do.  Note that a node may
  // be counted more than once.
  if (Node->isIncomplete())
    ++IBoundsChecks;
#if 0
  if (Node->isUnknownNode())
    ++UBoundsChecks;
#else
  if (Node->isUnknownNode()) {
    ++UBoundsChecks;
    return Dest;
  }
#endif
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

  //
  // Insert the bounds check.
  //
  std::vector<Value *> args(1, PH);
  args.push_back (SrcCast);
  args.push_back (DestCast);
  Instruction * CI;
  if ((Node == 0) ||
      (Node->isAllocaNode()) ||
      (Node->isIncomplete()) ||
      (Node->isUnknownNode()))
    CI = new CallInst(UIBoundsCheck, args, "uibc",InsertPt);
  else
    CI = new CallInst(BoundsCheck, args, "bc", InsertPt);

  // Update statistics
  ++BoundsChecks;
  return CI;
}

//
// Function: addExactCheck2()
//
// Description:
//  Utility routine that inserts a call to exactcheck2().
//
// Inputs:
//  GEP    - The GEP for which the check will be done.
//  Bounds - An LLVM Value representing the bounds of the check.
//
void
InsertPoolChecks::addExactCheck2 (GetElementPtrInst * GEP,
                                  Value * Bounds) {
  // The LLVM type for a void *
  Type *VoidPtrType = PointerType::get(Type::SByteTy); 

  //
  // Determine the insertion point.
  //
  Instruction * InsertPt = GEP->getNext();

  //
  // Cast the operands to the correct type.
  //
  Value * BasePointer = GEP->getPointerOperand();
  if (BasePointer->getType() != VoidPtrType)
    BasePointer = new CastInst(GEP->getPointerOperand(), VoidPtrType,
                              GEP->getPointerOperand()->getName()+".ec2.casted",
                              InsertPt);

  Value * ResultPointer = GEP;
  if (ResultPointer->getType() != VoidPtrType)
    ResultPointer = new CastInst(GEP, VoidPtrType,
                                 GEP->getName()+".ec2.casted",
                                 InsertPt);

  Value * CastBounds = Bounds;
  if (Bounds->getType() != Type::UIntTy)
    CastBounds = new CastInst(Bounds, Type::UIntTy,
                              Bounds->getName()+".ec.casted", InsertPt);

  std::vector<Value *> args(1, BasePointer);
  args.push_back(ResultPointer);
  args.push_back(CastBounds);
  new CallInst(ExactCheck2, args, "", InsertPt);

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
  // First, determine whether we need to perform a check at all.
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
                             Index->getName()+".ec.casted", GEP);

  Value * CastBounds = Bounds;
  if (Bounds->getType() != Type::IntTy)
    CastBounds = new CastInst(Bounds, Type::IntTy,
                              Bounds->getName()+".ec.casted", GEP);

  std::vector<Value *> args(1, CastIndex);
  args.push_back(CastBounds);
  new CallInst(ExactCheck, args, "", GEP);

  // Update statistics
  ++ExactChecks;
  return;
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
  DSGraph & TDG = TDPass->getDSGraph(*F);
  DSNode * Node = TDG.getNodeForValue(GEP).getNode();
  assert (Node && "boundscheck: DSNode is NULL!");

  //
  // Sometimes the pointer operand to a GEP is a cast; get the pointer that is
  // being casted.
  //
  if (ConstantExpr *cExpr = dyn_cast<ConstantExpr>(PointerOperand)) {
    if (cExpr->getOpcode() == Instruction::Cast)
      PointerOperand = cExpr->getOperand(0);
  }

  //
  // Make sure the pointer operand really is a pointer.
  if (!isa<PointerType>(PointerOperand->getType()))
    return false;

  //
  // Attempt to use a call to exactcheck() to check this value if it is a
  // global array with a non-zero size.  We do not check zero length arrays
  // because in C they are often used to declare an external array of unknown
  // size as follows:
  //        extern struct foo the_array[];
  //
  if (GlobalVariable *GV = dyn_cast<GlobalVariable>(PointerOperand)) {
    const ArrayType *AT = dyn_cast<ArrayType>(GV->getType()->getElementType());
    if (AT && (AT->getNumElements())) {
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
          //Handle non constant index two dimensional arrays later
          abort();
        }
      } else {
        //Handle Multi dimensional cases later
        Value* AllocSize=ConstantInt::get(Type::IntTy, TD->getTypeSize(GV->getType()->getElementType()));
        addExactCheck2 (GEP, AllocSize);
        return true;
        GEP->dump();
        ++MissedMultDimArrayChecks;
      }
      DEBUG(std::cerr << " Global variable ok \n");
    }
  }

  //
  // If the pointer was generated by a dominating alloca instruction, we can
  // do an exactcheck on it, too.
  //
  if (AllocaInst *AI = dyn_cast<AllocaInst>(GEP->getPointerOperand())) {
    const Type * AllocaType = AI->getAllocatedType();
    Value *AllocSize=ConstantInt::get(Type::IntTy, TD->getTypeSize(AllocaType));

    if (AI->isArrayAllocation())
      AllocSize = BinaryOperator::create(Instruction::Mul,
                                         AllocSize,
                                         AI->getOperand(0), "sizetmp", GEP);

    addExactCheck2 (GEP, AllocSize);
    return true;
  }

  //
  // If the pointer was an allocation, we should be able to do exact checks
  //
  if(CallInst* CI = dyn_cast<CallInst>(GEP->getPointerOperand())) {
    if (CI->getCalledFunction() && (
                              CI->getCalledFunction()->getName() == "__vmalloc" || 
                              CI->getCalledFunction()->getName() == "kmalloc")) {
      Value* Cast = new CastInst(CI->getOperand(1), Type::IntTy, "", GEP);
      addExactCheck2(GEP, Cast);
      return true;
    }
  }

  //
  // If the pointer is to a structure, we may be able to perform a simple
  // exactcheck on it, too, unless the array is at the end of the structure.
  // Then, we assume it's a variable length array and must be full checked.
  //
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

  /*
   * We were not able to insert a call to exactcheck().
   */
  return false;
}


bool InsertPoolChecks::runOnModule(Module &M) {
  cuaPass = &getAnalysis<ConvertUnsafeAllocas>();
  //  budsPass = &getAnalysis<CompleteBUDataStructures>();
#ifndef LLVA_KERNEL  
  paPass = &getAnalysis<PoolAllocate>();
  equivPass = &(paPass->getECGraphs());
  efPass = &getAnalysis<EmbeCFreeRemoval>();
  TD  = &getAnalysis<TargetData>();
#else
  TD  = &getAnalysis<TargetData>();
  TDPass = &getAnalysis<TDDataStructures>();
#endif

  //add the new poolcheck prototype 
  addPoolCheckProto(M);

  //register global arrays and collapsed nodes with global pools
  if (!DisableRegisterGlobals)
    registerGlobalArraysWithGlobalPools(M);

  //Replace old poolcheck with the new one 
  addPoolChecks(M);

  //Add obj drops
  addObjFrees(M);

  //
  // Update the statistics.
  //
  PoolChecks = NullChecks + FullChecks;
  
  return true;
}

static void AddCallToRegFunc(Function* F, GlobalVariable* GV, Function* PR, Value* PH, Value* AllocSize) {
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
  new CallInst(PR, make_vector(PHCasted, GVCasted, AllocSize, 0), "", InsertPt);
}

void InsertPoolChecks::registerGlobalArraysWithGlobalPools(Module &M) {

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
        if ((isa<ArrayType>(GV->getType()->getElementType())) ||
            (DSN && DSN->isNodeCompletelyFolded())) {
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

void InsertPoolChecks::addPoolChecks(Module &M) {
#if 0
  simplifyGEPList();
#endif
  for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I)  {
    std::string Name = I->getName();
    if ((Name == "load_llva_binary") || (Name == "do_open") || (Name == "ide_build_dmatable"))
      continue;
    if (!I->isExternal()) TransformFunction(*I);
  }
  if (!DisableLSChecks)  addLoadStoreChecks(M);
  //  if (!DisableGEPChecks) addGetElementPtrChecks(M);
}

void InsertPoolChecks::handleCallInst(CallInst *CI) {
  if (CI && (!DisableIntrinsicChecks)) {
    Value *Fop = CI->getOperand(0);
    Function *F = CI->getParent()->getParent();
#ifdef LLVA_KERNEL    
    if (Fop->getName() == "llvm.memcpy.i32") {
      //
      // Create a call to an accurate bounds check that will check that the
      // memory to which memcpy is copying data is valid.
      //
      Instruction *InsertPt = CI;
      CastInst *CastCIUint = 
        new CastInst(CI->getOperand(1), Type::UIntTy, "node.mccasted", InsertPt);
      CastInst *CastCIOp3 = 
        new CastInst(CI->getOperand(3), Type::UIntTy, "node.mccasted", InsertPt);
      Instruction *Bop = BinaryOperator::create(Instruction::Add, CastCIUint,
                                                CastCIOp3, "memcpyadd",InsertPt);

      Instruction *Finalp = BinaryOperator::create(Instruction::Sub, Bop,
                                                ConstantInt::get (Type::UIntTy, 1), "memcpysub",InsertPt);

      // Create the call to do an accurate bounds check
      insertBoundsCheck (CI, CI->getOperand(1), Finalp, InsertPt);
    } else if (Fop->getName() == "llva_memcpy") {
      //
      // Create a call to an accurate bounds check.
      //
      Instruction *InsertPt = CI;
      CastInst *CastCIUint = 
        new CastInst(CI->getOperand(1), Type::UIntTy, "node.mccasted", InsertPt);
      CastInst *CastCIOp3 = 
        new CastInst(CI->getOperand(3), Type::UIntTy, "node.mccasted", InsertPt);
      Instruction *Bop = BinaryOperator::create(Instruction::Add, CastCIUint,
                                                CastCIOp3, "memcpyadd",InsertPt);

      // Create the call to do an accurate bounds check
      insertBoundsCheck (CI, CI->getOperand(1), Bop, InsertPt);
    } else if (Fop->getName() == "memcmp") {
      //
      // Create a call to an accurate bounds check for each string parameter.
      //
      Instruction *InsertPt = CI;
      CastInst *CastPointer1 = 
        new CastInst(CI->getOperand(1), Type::UIntTy, "memcmpcast1", InsertPt);
      CastInst *CastPointer2 = 
        new CastInst(CI->getOperand(2), Type::UIntTy, "memcmpcast2", InsertPt);
      CastInst *CastCIOp3 = 
        new CastInst(CI->getOperand(3), Type::UIntTy, "memcmp.len", InsertPt);

      Instruction *Bop1 = BinaryOperator::create(Instruction::Add, CastPointer1,
                                                 CastCIOp3, "memcpyadd",InsertPt);
      Instruction *Bop2 = BinaryOperator::create(Instruction::Add, CastPointer2,
                                                 CastCIOp3, "memcpyadd",InsertPt);
      Instruction *Finalp1 = BinaryOperator::create(Instruction::Sub, Bop1,
                                                 ConstantInt::get (Type::UIntTy, 1), "memcpysub",InsertPt);
      Instruction *Finalp2 = BinaryOperator::create(Instruction::Sub, Bop2,
                                                 ConstantInt::get (Type::UIntTy, 1), "memcpysub",InsertPt);

      // Create the call to do an accurate bounds check
      insertBoundsCheck (CI, CI->getOperand(1), Bop1, InsertPt);
      insertBoundsCheck (CI, CI->getOperand(2), Bop2, InsertPt);
#if 0
    } else if (Fop->getName() == "memset") {
      Value *PH = getPoolHandle(CI->getOperand(1), F); 
      Instruction *InsertPt = CI->getNext();
      if (!PH) {
        NullChecks++;
        // Don't bother to insert the NULL check unless the user asked
        if (!EnableNullChecks)
          return;
        PH = Constant::getNullValue(PointerType::get(Type::SByteTy));
      }
      CastInst *CastCIUint = 
        new CastInst(CI, Type::UIntTy, "node.lscasted", InsertPt);
      CastInst *CastCIOp3 = 
        new CastInst(CI->getOperand(3), Type::UIntTy, "node.lscasted", InsertPt);
      Instruction *Bop = BinaryOperator::create(Instruction::Add, CastCIUint,
                                                CastCIOp3, "memsetadd",InsertPt);
      
      // Create instructions to cast the checked pointer and the checked pool
      // into sbyte pointers.
      CastInst *CastSourcePointer = 
        new CastInst(CI->getOperand(1), 
                     PointerType::get(Type::SByteTy), "memset.1.casted", InsertPt);
      CastInst *CastCI = 
        new CastInst(Bop, 
                     PointerType::get(Type::SByteTy), "memset.2.casted", InsertPt);
      CastInst *CastPHI = 
        new CastInst(PH, 
                     PointerType::get(Type::SByteTy), "poolhandle.lscasted", InsertPt);
      
      // Create the call to poolcheck
      std::vector<Value *> args(1,CastPHI);
      args.push_back(CastSourcePointer);
      args.push_back(CastCI);
      new CallInst(PoolCheckArray,args,"", InsertPt);
#endif
    }
#endif    
  }
}

void InsertPoolChecks::simplifyGEPList() {
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

}


void InsertPoolChecks::handleGetElementPtr(GetElementPtrInst *MAI) {
  // Get the set of unsafe GEP instructions from the array bounds check pass
  // If this instruction is not within that set, then the result of the GEP
  // instruction has been proven safe, and there is no need to insert a check.
  std::set<Instruction *> & UnsafeGetElemPtrs = cuaPass->getUnsafeGetElementPtrsFromABC();
  std::set<Instruction *>::const_iterator iCurrent = UnsafeGetElemPtrs.find(MAI);
  if (iCurrent == UnsafeGetElemPtrs.end()) {
#if 0
    std::cerr << "statically proved safe : Not inserting checks " << *MAI << "\n";
#endif
    return;
  }

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

    DSGraph & TDG = TDPass->getDSGraph(*F);
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
          //
          // Insert a bounds check and use its return value in all subsequent
          // uses.
          //
          Instruction *nextIns = MAI->getNext();
          CastInst *CastBack = new CastInst(MAI, MAI->getType(), MAI->getName()+".castback",nextIns);
          MAI->replaceAllUsesWith(CastBack);
  
          Value * V;
          V = insertBoundsCheck (MAI, MAI->getPointerOperand(), MAI, CastBack);
          CastBack->setOperand(0, V);
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
    if (!equivPass->ContainsDSGraphFor(*F)) {
      //some times the ECGraphs doesnt contain F
      //for newly created cloned functions
      PA::FuncInfo *FI = paPass->getFuncInfoOrClone(*F);
      op = FI->MapValueToOriginal(op);
      if (!op) return; //abort();
    }
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

void InsertPoolChecks::TransformFunction(Function &F) {
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
          if ((!isa<ConstantPointerNull>(SCI->getOperand(0))) && (!isa<ConstantPointerNull>(SCI->getOperand(1)))) {
            addGetActualValue(SCI, 0);
            addGetActualValue(SCI, 1);
          }
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
      handleGetElementPtr(MAI);
      continue;
    } else if (AllocaInst *AI = dyn_cast<AllocaInst>(iLocal)) {
      AllocaInst * AIOrig = AI;
      if (!DisableStackChecks) {
#ifndef LLVA_KERNEL      
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
  // Get the pool handle for the node that this contributes to...
  Function *FOrig  = AIOrig->getParent()->getParent();
  DSNode *Node = getDSNode(AIOrig, FOrig);
  if (!Node) return;
#if 0
  if (Node->isArray()) {
#else
  if (1) {
#endif
    Value *PH = getPoolHandle(AIOrig, FOrig);
    if (PH == 0 || isa<ConstantPointerNull>(PH)) return;
    Value *AllocSize =
      ConstantInt::get(Type::UIntTy, TD->getTypeSize(AI->getAllocatedType()));
    
    if (AI->isArrayAllocation())
      AllocSize = BinaryOperator::create(Instruction::Mul, AllocSize,
                                         AI->getOperand(0), "sizetmp", AI);

    // Insert poolregister at the end of allocas and all poolinits
    // FIXME assuming that there is a load before a call??
    Instruction *iptI;
#if 0
    if (AI->getParent() == AI->getParent()->getParent()->getEntryBlock()) {
#else
    if (0) {
#endif
      BasicBlock::iterator InsertPt = AI->getParent()->getParent()->getEntryBlock().begin();
      while ((isa<CallInst>(InsertPt)) ||
              isa<CastInst>(InsertPt)  ||
              isa<AllocaInst>(InsertPt) ||
              isa<BinaryOperator>(InsertPt)) {
#if 0
        if (CallInst *PI = dyn_cast<CallInst>(InsertPt)) {
          if (PI->getName() != "poolinit") break;
        }
#endif
        ++InsertPt;
      }
      iptI = InsertPt;
    } else {
      iptI = AI->getNext();
    }

    //
    // Insert a call to register the object.
    //
    Instruction *Casted = new CastInst(AI, PointerType::get(Type::SByteTy),
                                       AI->getName()+".casted", iptI);
    Value *CastedPH = new CastInst(PH,
                                   PointerType::get(Type::SByteTy),
                                   "ph",Casted);
    new CallInst(PoolRegister,
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
        new CallInst(ObjFree,
                     make_vector(CastedPH, Casted, 0),
                     "", iptI);
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
  DSGraph & TDG = TDPass->getDSGraph(*F);
  DSNode * Node = TDG.getNodeForValue(V).getNode();
  
  if (Node && Node->isNodeCompletelyFolded()) {
    if (!EnableIncompleteChecks) {
      if (Node->isIncomplete()) {
        ++MissedIncompleteChecks;
        return;
      }
    }
    // Get the pool handle associated with this pointer.  If there is no pool
    // handle, use a NULL pointer value and let the runtime deal with it.
    Value *PH = getPoolHandle(V, F);

    // FIXME: We cannot handle checks to global or stack positions right now.
    if ((!PH) || (Node->isIncomplete()) ||
                 (Node->isAllocaNode()) ||
                 (Node->isUnknownNode()) ||
                 (!((Node->isHeapNode()) && (Node->isGlobalNode())))) {
      ++NullChecks;
      if (!PH) ++MissedNullChecks;
      if (Node->isAllocaNode()) ++MissedStackChecks;

      // Don't bother to insert the NULL check unless the user asked
      if (!EnableNullChecks)
        return;
      if (!PH) PH = Constant::getNullValue(PointerType::get(Type::SByteTy));
    } else {
      //
      // Only add the pool check if the pool is a global value or it
      // belongs to the same basic block.
      //
      if (isa<GlobalValue>(PH)) {
        ++FullChecks;
      } else if (isa<Instruction>(PH)) {
        Instruction * IPH = (Instruction *)(PH);
        if (IPH->getParent() == I->getParent()) {
          //
          // If the instructions belong to the same basic block, ensure that
          // the pool dominates the load/store.
          //
          Instruction * IP = IPH;
          for (IP=IPH; (IP->isTerminator()) || (IP == I); IP=IP->getNext()) {
            ;
          }
          if (IP == I)
            ++FullChecks;
          else {
            ++MissChecks;
            return;
          }
        } else {
          ++MissChecks;
          return;
        }
      } else {
        ++MissChecks;
        return;
      }
    }      
    // Create instructions to cast the checked pointer and the checked pool
    // into sbyte pointers.
    CastInst *CastVI = 
      new CastInst(V, 
		   PointerType::get(Type::SByteTy), "node.lscasted", I);
    CastInst *CastPHI = 
      new CastInst(PH, 
		   PointerType::get(Type::SByteTy), "poolhandle.lscasted", I);

    // Create the call to poolcheck
    std::vector<Value *> args(1,CastPHI);
    args.push_back(CastVI);
    new CallInst(PoolCheck,args,"", I);
  }
}

void InsertPoolChecks::addLoadStoreChecks(Module &M){
  Module::iterator mI = M.begin(), mE = M.end();
  for ( ; mI != mE; ++mI) {
    Function *F = mI;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      if (LoadInst *LI = dyn_cast<LoadInst>(&*I)) {
        Value *P = LI->getPointerOperand();
        addLSChecks(P, LI, F);
      } else if (StoreInst *SI = dyn_cast<StoreInst>(&*I)) {
        Value *P = SI->getPointerOperand();
        addLSChecks(P, SI, F);
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


void InsertPoolChecks::addLoadStoreChecks(Module &M){
  Module::iterator mI = M.begin(), mE = M.end();
  for ( ; mI != mE; ++mI) {
    Function *F = mI;
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
}

#endif

void InsertPoolChecks::addPoolCheckProto(Module &M) {
  const Type * VoidPtrType = PointerType::get(Type::SByteTy);
  /*
  const Type *PoolDescType = ArrayType::get(VoidPtrType, 50);
  //	StructType::get(make_vector<const Type*>(VoidPtrType, VoidPtrType,
  //                                               Type::UIntTy, Type::UIntTy, 0));
  const Type * PoolDescTypePtr = PointerType::get(PoolDescType);
  */  

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

  //Get the poolregister function
  PoolRegister = M.getOrInsertFunction("pchk_reg_obj", Type::VoidTy, VoidPtrType,
                                       VoidPtrType, Type::UIntTy, NULL);
  ObjFree = M.getOrInsertFunction("pchk_drop_obj", Type::VoidTy, VoidPtrType,
                                  VoidPtrType, NULL);
  
  PoolFindMP = M.getOrInsertFunction("pchk_getLoc", VoidPtrType, VoidPtrType, NULL);
  PoolRegMP = M.getOrInsertFunction("pchk_reg_pool", Type::VoidTy, VoidPtrType, VoidPtrType, VoidPtrType, NULL);

  std::vector<const Type *> FArg2(1, Type::IntTy);
  FArg2.push_back(Type::IntTy);
  FunctionType *ExactCheckTy = FunctionType::get(Type::VoidTy, FArg2, false);
  ExactCheck = M.getOrInsertFunction("exactcheck", ExactCheckTy);

  std::vector<const Type *> FArg3(1, Type::UIntTy);
  FArg3.push_back(VoidPtrType);
  FArg3.push_back(VoidPtrType);
  FunctionType *FunctionCheckTy = FunctionType::get(Type::VoidTy, FArg3, true);
  FunctionCheck = M.getOrInsertFunction("funccheck", FunctionCheckTy);

  std::vector<const Type*> FArg5(1, VoidPtrType);
  FArg5.push_back(VoidPtrType);
  FunctionType *GetActualValueTy = FunctionType::get(VoidPtrType, FArg5, false);
  GetActualValue = M.getOrInsertFunction("pchk_getActualValue", GetActualValueTy);
  
  std::vector<const Type*> FArg4(1, VoidPtrType); //base
  FArg4.push_back(VoidPtrType); //result
  FArg4.push_back(Type::UIntTy); //size
  FunctionType *ExactCheck2Ty = FunctionType::get(Type::VoidTy, FArg4, false);
  ExactCheck2 = M.getOrInsertFunction("exactcheck2", ExactCheck2Ty);
  
  
}

DSNode* InsertPoolChecks::getDSNode(const Value *V, Function *F) {
#ifndef LLVA_KERNEL
  DSGraph &TDG = equivPass->getDSGraph(*F);
#else  
  DSGraph &TDG = TDPass->getDSGraph(*F);
#endif  
  DSNode *DSN = TDG.getNodeForValue((Value *)V).getNode();
  return DSN;
}

unsigned InsertPoolChecks::getDSNodeOffset(const Value *V, Function *F) {
#ifndef LLVA_KERNEL
  DSGraph &TDG = equivPass->getDSGraph(*F);
#else  
  DSGraph &TDG = TDPass->getDSGraph(*F);
#endif  
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
      if (CollapsedPoolPtrs[F].find(I->second) !=
	  CollapsedPoolPtrs[F].end()) {
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
Value *InsertPoolChecks::getPoolHandle(const Value *V, Function *F) {
  DSGraph &TDG =  TDPass->getDSGraph(*F);
  DSNode *Node = TDG.getNodeForValue((Value *)V).getNode();
  // Get the pool handle for this DSNode...
  //  assert(!Node->isUnknownNode() && "Unknown node \n");
  //  if (Node->isUnknownNode()) {
  //    return 0;
  //  }
  return getPD(Node, *F->getParent());
}
#endif
