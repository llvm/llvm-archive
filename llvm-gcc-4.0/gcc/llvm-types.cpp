/* APPLE LOCAL begin LLVM (ENTIRE FILE!)  */
/* Tree type to LLVM type converter 
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

//===----------------------------------------------------------------------===//
// This is the code that converts GCC tree types into LLVM types.
//===----------------------------------------------------------------------===//

#include "llvm-internal.h"
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/TypeSymbolTable.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringExtras.h"
#include <iostream>
#include <map>

extern "C" {
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
}
#include "llvm-abi.h"


//===----------------------------------------------------------------------===//
//                   Matching LLVM types with GCC trees
//===----------------------------------------------------------------------===//
//
// LTypes is a vector of LLVM types. GCC tree nodes keep track of LLVM types 
// using this vector's index. It is easier to save and restore the index than 
// the LLVM type pointer while usig PCH. STL vector does not provide fast 
// searching mechanism which is required to remove LLVM Type entry when type is 
// refined and replaced by another LLVM Type. This is achieved by maintaining 
// a map.

// Collection of LLVM Types
static std::vector<const Type *> LTypes;
typedef DenseMap<const Type *, unsigned> LTypesMapTy;
static LTypesMapTy LTypesMap;

// GET_TYPE_LLVM/SET_TYPE_LLVM - Associate an LLVM type with each TREE type.
// These are lazily computed by ConvertType.

#define SET_TYPE_SYMTAB_LLVM(NODE, index) \
  (TYPE_CHECK (NODE)->type.symtab.llvm = index)

// Note down LLVM type for GCC tree node.
static const Type * llvm_set_type(tree Tr, const Type *Ty) {

  unsigned &TypeSlot = LTypesMap[Ty];
  if (TypeSlot) {
    // Already in map.
    SET_TYPE_SYMTAB_LLVM(Tr, TypeSlot);
    return Ty;
  }

  unsigned Index = LTypes.size() + 1;
  LTypes.push_back(Ty);
  SET_TYPE_SYMTAB_LLVM(Tr, Index);
  LTypesMap[Ty] = Index;

  return Ty;
}

#define SET_TYPE_LLVM(NODE, TYPE) (const Type *)llvm_set_type(NODE, TYPE)

// Get LLVM Type for the GCC tree node based on LTypes vector index.
// When GCC tree node is initialized, it has 0 as the index value. This is
// why all recorded indexes are offset by 1. 
extern "C" const Type *llvm_get_type(unsigned Index) {
  if (Index == 0)
    return NULL;
  assert ((Index - 1) < LTypes.size() && "Invalid LLVM Type index");
  return LTypes[Index - 1];
}

#define GET_TYPE_LLVM(NODE) \
  (const Type *)llvm_get_type( TYPE_CHECK (NODE)->type.symtab.llvm)

// Erase type from LTypes vector
static void llvmEraseLType(const Type *Ty) {

  LTypesMapTy::iterator I = LTypesMap.find(Ty);

  if (I != LTypesMap.end()) {
    // It is OK to clear this entry instead of removing this entry
    // to avoid re-indexing of other entries.
    LTypes[ LTypesMap[Ty] - 1] = NULL;
    LTypesMap.erase(I);
  }
}

// Read LLVM Types string table
void readLLVMTypesStringTable() {

  GlobalValue *V = TheModule->getNamedGlobal("llvm.pch.types");
  if (!V)
    return;

  //  Value *GV = TheModule->getValueSymbolTable().lookup("llvm.pch.types");
  GlobalVariable *GV = cast<GlobalVariable>(V);
  ConstantStruct *LTypesNames = cast<ConstantStruct>(GV->getOperand(0));

  for (unsigned i = 0; i < LTypesNames->getNumOperands(); ++i) {
    const Type *Ty = NULL;

    if (ConstantArray *CA = 
        dyn_cast<ConstantArray>(LTypesNames->getOperand(i))) {
      std::string Str = CA->getAsString();
      Ty = TheModule->getTypeByName(Str);
      assert (Ty != NULL && "Invalid Type in LTypes string table");
    } 
    // If V is not a string then it is empty. Insert NULL to represent 
    // empty entries.
    LTypes.push_back(Ty);
  }

  // Now, llvm.pch.types value is not required so remove it from the symbol
  // table.
  GV->eraseFromParent();
}


// GCC tree's uses LTypes vector's index to reach LLVM types.
// Create a string table to hold these LLVM types' names. This string
// table will be used to recreate LTypes vector after loading PCH.
void writeLLVMTypesStringTable() {
  
  if (LTypes.empty()) 
    return;

  std::vector<Constant *> LTypesNames;
  std::map < const Type *, std::string > TypeNameMap;

  // Collect Type Names in advance.
  const TypeSymbolTable &ST = TheModule->getTypeSymbolTable();
  TypeSymbolTable::const_iterator TI = ST.begin();
  for (; TI != ST.end(); ++TI) {
    TypeNameMap[TI->second] = TI->first;
  }

  // Populate LTypesNames vector.
  for (std::vector<const Type *>::iterator I = LTypes.begin(),
         E = LTypes.end(); I != E; ++I)  {
    const Type *Ty = *I;

    // Give names to nameless types.
    if (Ty && TypeNameMap[Ty].empty()) {
      std::string NewName =
        TheModule->getTypeSymbolTable().getUniqueName("llvm.fe.ty");
      TheModule->addTypeName(NewName, Ty);
      TypeNameMap[*I] = NewName;
    }

    const std::string &TypeName = TypeNameMap[*I];
    LTypesNames.push_back(ConstantArray::get(TypeName, false));
  }

  // Create string table.
  Constant *LTypesNameTable = ConstantStruct::get(LTypesNames, false);

  // Create variable to hold this string table.
  GlobalVariable *GV = new GlobalVariable(LTypesNameTable->getType(), true,
                                          GlobalValue::ExternalLinkage, 
                                          LTypesNameTable,
                                          "llvm.pch.types", TheModule);
}

//===----------------------------------------------------------------------===//
//                   Recursive Type Handling Code and Data
//===----------------------------------------------------------------------===//

// Recursive types are a major pain to handle for a couple of reasons.  Because
// of this, when we start parsing a struct or a union, we globally change how
// POINTER_TYPE and REFERENCE_TYPE are handled.  In particular, instead of
// actually recursing and computing the type they point to, they will return an
// opaque*, and remember that they did this in PointersToReresolve.



//===----------------------------------------------------------------------===//
//                       Type Conversion Utilities
//===----------------------------------------------------------------------===//

// isPassedByInvisibleReference - Return true if an argument of the specified
// type should be passed in by invisible reference.
//
bool isPassedByInvisibleReference(tree Type) {
  // FIXME: Search for TREE_ADDRESSABLE in calls.c, and see if there are other
  // cases that make arguments automatically passed in by reference.
  return TREE_ADDRESSABLE(Type) || TYPE_SIZE(Type) == 0 ||
         TREE_CODE(TYPE_SIZE(Type)) != INTEGER_CST;
}

/// GetTypeName - Return a fully qualified (with namespace prefixes) name for
/// the specified type.
static std::string GetTypeName(const char *Prefix, tree type) {
  const char *Name = "anon";
  if (TYPE_NAME(type))
    if (TREE_CODE(TYPE_NAME(type)) == IDENTIFIER_NODE)
      Name = IDENTIFIER_POINTER(TYPE_NAME(type));
    else 
      Name = IDENTIFIER_POINTER(DECL_NAME(TYPE_NAME(type)));
  
  std::string ContextStr;
  tree Context = TYPE_CONTEXT(type);
  while (Context) {
    switch (TREE_CODE(Context)) {
    case TRANSLATION_UNIT_DECL: Context = 0; break;  // Done.
    case RECORD_TYPE:
    case NAMESPACE_DECL:
      if (TREE_CODE(Context) == RECORD_TYPE) {
        if (TYPE_NAME(Context)) {
          std::string NameFrag;
          if (TREE_CODE(TYPE_NAME(Context)) == IDENTIFIER_NODE) {
            NameFrag = IDENTIFIER_POINTER(TYPE_NAME(Context));
          } else {
            NameFrag = IDENTIFIER_POINTER(DECL_NAME(TYPE_NAME(Context)));
          }

          ContextStr = NameFrag + "::" + ContextStr;
          Context = TYPE_CONTEXT(Context);
          break;
        }
        // Anonymous record, fall through.
      } else if (DECL_NAME(Context)
                 /*&& DECL_NAME(Context) != anonymous_namespace_name*/){
        assert(TREE_CODE(DECL_NAME(Context)) == IDENTIFIER_NODE);
        std::string NamespaceName = IDENTIFIER_POINTER(DECL_NAME(Context));
        ContextStr = NamespaceName + "::" + ContextStr;
        Context = DECL_CONTEXT(Context);
        break;
      }
      // FALL THROUGH for anonymous namespaces and records!
      
    default: {
      // If this is a structure type defined inside of a function or other block
      // scope, make sure to make the type name unique by putting a unique ID
      // in it.
      static unsigned UniqueID = 0;
      ContextStr = "." + utostr(UniqueID++);
      Context = 0;   // Stop looking at context
      break;
    }
    }      
  }  
  return Prefix + ContextStr + Name;
}

/// arrayLength - Return a tree expressing the number of elements in an array
/// of the specified type, or NULL if the type does not specify the length.
tree_node *arrayLength(tree_node *type) {
  tree Domain = TYPE_DOMAIN(type);

  if (!Domain || !TYPE_MAX_VALUE(Domain))
    return NULL;

  tree length = fold_convert(sizetype, TYPE_MAX_VALUE(Domain));
  if (TYPE_MIN_VALUE(Domain))
    length = size_binop (MINUS_EXPR, length,
                         fold_convert(sizetype, TYPE_MIN_VALUE(Domain)));
  return size_binop (PLUS_EXPR, length, size_one_node);
}


//===----------------------------------------------------------------------===//
//                     Abstract Type Refinement Helpers
//===----------------------------------------------------------------------===//
//
// This code is built to make sure that the TYPE_LLVM field on tree types are
// updated when LLVM types are refined.  This prevents dangling pointers from
// occuring due to type coallescing.
//
namespace {
  class TypeRefinementDatabase : public AbstractTypeUser {
    virtual void refineAbstractType(const DerivedType *OldTy,
                                    const Type *NewTy);
    virtual void typeBecameConcrete(const DerivedType *AbsTy);
    
    // TypeUsers - For each abstract LLVM type, we keep track of all of the GCC
    // types that point to it.
    std::map<const Type*, std::vector<tree> > TypeUsers;
  public:
    /// setType - call SET_TYPE_LLVM(type, Ty), associating the type with the
    /// specified tree type.  In addition, if the LLVM type is an abstract type,
    /// we add it to our data structure to track it.
    inline const Type *setType(tree type, const Type *Ty) {
      if (GET_TYPE_LLVM(type))
        RemoveTypeFromTable(type);

      if (Ty->isAbstract()) {
        std::vector<tree> &Users = TypeUsers[Ty];
        if (Users.empty()) Ty->addAbstractTypeUser(this);
        Users.push_back(type);
      }
      return SET_TYPE_LLVM(type, Ty);
    }

    void RemoveTypeFromTable(tree type);
    void dump() const;
  };
  
  /// TypeDB - The main global type database.
  TypeRefinementDatabase TypeDB;
}

/// RemoveTypeFromTable - We're about to change the LLVM type of 'type'
///
void TypeRefinementDatabase::RemoveTypeFromTable(tree type) {
  const Type *Ty = GET_TYPE_LLVM(type);
  if (!Ty->isAbstract()) return;
  std::map<const Type*, std::vector<tree> >::iterator I = TypeUsers.find(Ty);
  assert(I != TypeUsers.end() && "Using an abstract type but not in table?");
  
  bool FoundIt = false;
  for (unsigned i = 0, e = I->second.size(); i != e; ++i)
    if (I->second[i] == type) {
      FoundIt = true;
      std::swap(I->second[i], I->second.back());
      I->second.pop_back();
      break;
    }
  assert(FoundIt && "Using an abstract type but not in table?");
  
  // If the type plane is now empty, nuke it.
  if (I->second.empty()) {
    TypeUsers.erase(I);
    Ty->removeAbstractTypeUser(this);
  }
}

/// refineAbstractType - The callback method invoked when an abstract type is
/// resolved to another type.  An object must override this method to update
/// its internal state to reference NewType instead of OldType.
///
void TypeRefinementDatabase::refineAbstractType(const DerivedType *OldTy,
                                                const Type *NewTy) {
  if (OldTy == NewTy && OldTy->isAbstract()) return; // Nothing to do.
  
  std::map<const Type*, std::vector<tree> >::iterator I = TypeUsers.find(OldTy);
  assert(I != TypeUsers.end() && "Using an abstract type but not in table?");

  if (!NewTy->isAbstract()) {
    // If the type became concrete, update everything pointing to it, and remove
    // all of our entries from the map.
    if (OldTy != NewTy)
      for (unsigned i = 0, e = I->second.size(); i != e; ++i)
        SET_TYPE_LLVM(I->second[i], NewTy);
  } else {
    // Otherwise, it was refined to another instance of an abstract type.  Move
    // everything over and stop monitoring OldTy.
    std::vector<tree> &NewSlot = TypeUsers[NewTy];
    if (NewSlot.empty()) NewTy->addAbstractTypeUser(this);
    
    for (unsigned i = 0, e = I->second.size(); i != e; ++i) {
      NewSlot.push_back(I->second[i]);
      SET_TYPE_LLVM(I->second[i], NewTy);
    }
  }
  
  llvmEraseLType(OldTy);
  TypeUsers.erase(I);
  
  // Next, remove OldTy's entry in the TargetData object if it has one.
  if (const StructType *STy = dyn_cast<StructType>(OldTy))
    getTargetData().InvalidateStructLayoutInfo(STy);
  
  OldTy->removeAbstractTypeUser(this);
}

/// The other case which AbstractTypeUsers must be aware of is when a type
/// makes the transition from being abstract (where it has clients on it's
/// AbstractTypeUsers list) to concrete (where it does not).  This method
/// notifies ATU's when this occurs for a type.
///
void TypeRefinementDatabase::typeBecameConcrete(const DerivedType *AbsTy) {
  assert(TypeUsers.count(AbsTy) && "Not using this type!");
  // Remove the type from our collection of tracked types.
  TypeUsers.erase(AbsTy);
  AbsTy->removeAbstractTypeUser(this);
}
void TypeRefinementDatabase::dump() const {
  std::cerr << "TypeRefinementDatabase\n";
}


//===----------------------------------------------------------------------===//
//                      Main Type Conversion Routines
//===----------------------------------------------------------------------===//

const Type *TypeConverter::ConvertType(tree orig_type) {
  if (orig_type == error_mark_node) return Type::Int32Ty;
  
  // LLVM doesn't care about variants such as const, volatile, or restrict.
  tree type = TYPE_MAIN_VARIANT(orig_type);

  switch (TREE_CODE(type)) {
  default:
    fprintf(stderr, "Unknown type to convert:\n");
    debug_tree(type);
    abort();
  case VOID_TYPE:   return SET_TYPE_LLVM(type, Type::VoidTy);
  case RECORD_TYPE: return ConvertRECORD(type, orig_type);
  case UNION_TYPE:  return ConvertUNION(type, orig_type);
  case BOOLEAN_TYPE: {
    if (const Type *Ty = GET_TYPE_LLVM(type))
      return Ty;
    return SET_TYPE_LLVM(type,
                         IntegerType::get(TREE_INT_CST_LOW(TYPE_SIZE(type))));
  }
  case ENUMERAL_TYPE:
    // Use of an enum that is implicitly declared?
    if (TYPE_SIZE(type) == 0) {
      // If we already compiled this type, use the old type.
      if (const Type *Ty = GET_TYPE_LLVM(type))
        return Ty;

      const Type *Ty = OpaqueType::get();
      TheModule->addTypeName(GetTypeName("enum.", orig_type), Ty);
      return TypeDB.setType(type, Ty);
    }
    // FALL THROUGH.
  case INTEGER_TYPE:
    if (const Type *Ty = GET_TYPE_LLVM(type)) return Ty;

    // FIXME: eliminate this when 128-bit integer types in LLVM work.
    switch (TREE_INT_CST_LOW(TYPE_SIZE(type))) {
    case 1:
    case 8:
    case 16:
    case 32:
    case 64:
      break;
    default:
      static bool Warned = false;
      if (!Warned)
        fprintf(stderr, "WARNING: %d-bit integers not supported!\n",
                (int)TREE_INT_CST_LOW(TYPE_SIZE(type)));
      Warned = true;
      return Type::Int64Ty;
    }
    return SET_TYPE_LLVM(type, 
                         IntegerType::get(TREE_INT_CST_LOW(TYPE_SIZE(type))));
  case REAL_TYPE:
    if (const Type *Ty = GET_TYPE_LLVM(type)) return Ty;
    switch (TYPE_PRECISION(type)) {
    default:
      fprintf(stderr, "Unknown FP type!\n");
      debug_tree(type);
      abort();        
    case 32: return SET_TYPE_LLVM(type, Type::FloatTy);
    case 64: return SET_TYPE_LLVM(type, Type::DoubleTy);
    case 128:
      // 128-bit long doubles map onto { double, double }.
      const Type *Ty = Type::DoubleTy;
      Ty = StructType::get(std::vector<const Type*>(2, Ty), false);
      return SET_TYPE_LLVM(type, Ty);
    }
    
  case COMPLEX_TYPE: {
    if (const Type *Ty = GET_TYPE_LLVM(type)) return Ty;
    const Type *Ty = ConvertType(TREE_TYPE(type));
    assert(!Ty->isAbstract() && "should use TypeDB.setType()");
    Ty = StructType::get(std::vector<const Type*>(2, Ty), false);
    return SET_TYPE_LLVM(type, Ty);
  }
  case VECTOR_TYPE: {
    if (const Type *Ty = GET_TYPE_LLVM(type)) return Ty;
    const Type *Ty = ConvertType(TREE_TYPE(type));
    assert(!Ty->isAbstract() && "should use TypeDB.setType()");
    Ty = VectorType::get(Ty, TYPE_VECTOR_SUBPARTS(type));
    return SET_TYPE_LLVM(type, Ty);
  }
    
  case POINTER_TYPE:
  case REFERENCE_TYPE:
    if (const PointerType *Ty = cast_or_null<PointerType>(GET_TYPE_LLVM(type))){
      // We already converted this type.  If this isn't a case where we have to
      // reparse it, just return it.
      if (PointersToReresolve.empty() || PointersToReresolve.back() != type ||
          ConvertingStruct)
        return Ty;
      
      // Okay, we know that we're !ConvertingStruct and that type is on the end
      // of the vector.  Remove this entry from the PointersToReresolve list and
      // get the pointee type.  Note that this order is important in case the
      // pointee type uses this pointer.
      assert(isa<OpaqueType>(Ty->getElementType()) && "Not a deferred ref!");
      
      // We are actively resolving this pointer.  We want to pop this value from
      // the stack, as we are no longer resolving it.  However, we don't want to
      // make it look like we are now resolving the previous pointer on the
      // stack, so pop this value and push a null.
      PointersToReresolve.back() = 0;
      
      
      // Do not do any nested resolution.  We know that there is a higher-level
      // loop processing deferred pointers, let it handle anything new.
      ConvertingStruct = true;
      
      // Note that we know that Ty cannot be resolved or invalidated here.
      const Type *Actual = ConvertType(TREE_TYPE(type));
      assert(GET_TYPE_LLVM(type) == Ty && "Pointer invalidated!");

      // Restore ConvertingStruct for the caller.
      ConvertingStruct = false;
      
      if (Actual->getTypeID() == Type::VoidTyID) 
        Actual = Type::Int8Ty;  // void* -> sbyte*
      
      // Update the type, potentially updating TYPE_LLVM(type).
      const OpaqueType *OT = cast<OpaqueType>(Ty->getElementType());
      const_cast<OpaqueType*>(OT)->refineAbstractTypeTo(Actual);
      return GET_TYPE_LLVM(type);
    } else {
      const Type *Ty;

      // If we are converting a struct, and if we haven't converted the pointee
      // type, add this pointer to PointersToReresolve and return an opaque*.
      if (ConvertingStruct) {
        // If the pointee type has not already been converted to LLVM, create 
        // a new opaque type and remember it in the database.
        Ty = GET_TYPE_LLVM(TYPE_MAIN_VARIANT(TREE_TYPE(type)));
        if (Ty == 0) {
          PointersToReresolve.push_back(type);
          return TypeDB.setType(type, PointerType::get(OpaqueType::get()));
        }

        // A type has already been computed.  However, this may be some sort of 
        // recursive struct.  We don't want to call ConvertType on it, because 
        // this will try to resolve it, and not adding the type to the 
        // PointerToReresolve collection is just an optimization.  Instead, 
        // we'll use the type returned by GET_TYPE_LLVM directly, even if this 
        // may be resolved further in the future.
      } else {
        // If we're not in a struct, just call ConvertType.  If it has already 
        // been converted, this will return the precomputed value, otherwise 
        // this will compute and return the new type.
        Ty = ConvertType(TREE_TYPE(type));
      }
    
      if (Ty->getTypeID() == Type::VoidTyID) 
        Ty = Type::Int8Ty;  // void* -> sbyte*
      return TypeDB.setType(type, PointerType::get(Ty));
    }
   
  case METHOD_TYPE:
  case FUNCTION_TYPE: {
    if (const Type *Ty = GET_TYPE_LLVM(type))
      return Ty;
    
    unsigned CallingConv;
    return TypeDB.setType(type, ConvertFunctionType(type, NULL, CallingConv));
  }
  case ARRAY_TYPE: {
    if (const Type *Ty = GET_TYPE_LLVM(type))
      return Ty;

    unsigned NumElements;
    tree length = arrayLength(type);
    if (length) {
      if (host_integerp(length, 1)) {
        // Normal array.
        NumElements = tree_low_cst(length, 1);
      } else {
        // This handles cases like "int A[n]" which have a runtime constant
        // number of elements, but is a compile-time variable.  Since these are
        // variable sized, we just represent them as the element themself.
        return TypeDB.setType(type, ConvertType(TREE_TYPE(type)));
      }
    } else {
      // We get here is if they have something that is globally declared as an
      // array with no dimension, this becomes just a zero size array of the
      // element type so that: int X[] becomes *'%X = external global [0 x int]'
      //
      // Note that this also affects new expressions, which return a pointer to
      // an unsized array of elements.
      NumElements = 0;
    }
    
    return TypeDB.setType(type, ArrayType::get(ConvertType(TREE_TYPE(type)),
                                                NumElements));
  }
  case OFFSET_TYPE:
    // Handle OFFSET_TYPE specially.  This is used for pointers to members,
    // which are really just integer offsets.  As such, return the appropriate
    // integer directly.
    switch (getTargetData().getPointerSize()) {
    default: assert(0 && "Unknown pointer size!");
    case 4: return Type::Int32Ty;
    case 8: return Type::Int64Ty;
    }
  }
}

//===----------------------------------------------------------------------===//
//                  FUNCTION/METHOD_TYPE Conversion Routines
//===----------------------------------------------------------------------===//

namespace {
  class FunctionTypeConversion : public DefaultABIClient {
    const Type *&RetTy;
    std::vector<const Type*> &ArgTypes;
    unsigned &CallingConv;
    bool isStructRet;
    bool KNRPromotion;
  public:
    FunctionTypeConversion(const Type *&retty, std::vector<const Type*> &AT,
                           unsigned &CC, bool KNR)
      : RetTy(retty), ArgTypes(AT), CallingConv(CC), KNRPromotion(KNR) {
      CallingConv = CallingConv::C;
      isStructRet = false;
    }

    bool isStructReturn() const { return isStructRet; }
    
    /// HandleScalarResult - This callback is invoked if the function returns a
    /// simple scalar result value.
    void HandleScalarResult(const Type *RetTy) {
      this->RetTy = RetTy;
    }
    
    /// HandleAggregateResultAsScalar - This callback is invoked if the function
    /// returns an aggregate value by bit converting it to the specified scalar
    /// type and returning that.
    void HandleAggregateResultAsScalar(const Type *ScalarTy) {
      RetTy = ScalarTy;
    }
    
    /// HandleAggregateShadowArgument - This callback is invoked if the function
    /// returns an aggregate value by using a "shadow" first parameter.  If 
    /// RetPtr is set to true, the pointer argument itself is returned from 
    /// the function.
    void HandleAggregateShadowArgument(const PointerType *PtrArgTy,
                                       bool RetPtr) {
      // If this function returns a structure by value, it either returns void
      // or the shadow argument, depending on the target.
      this->RetTy = RetPtr ? PtrArgTy : Type::VoidTy;
      
      // In any case, there is a dummy shadow argument though!
      ArgTypes.push_back(PtrArgTy);
      
      // Also, switch the to C Struct Return.
      isStructRet = true;
    }
    
    void HandleScalarArgument(const llvm::Type *LLVMTy, tree type) {
      if (KNRPromotion) {
        if (LLVMTy == Type::FloatTy)
          LLVMTy = Type::DoubleTy;
        else if (LLVMTy == Type::Int16Ty || LLVMTy == Type::Int8Ty ||
                 LLVMTy == Type::Int1Ty)
          LLVMTy = Type::Int32Ty;
      }
      ArgTypes.push_back(LLVMTy);
    }
  };
}

/// ConvertParamListToLLVMSignature - This method is used to build the argument
/// type list for K&R prototyped functions.  In this case, we have to figure out
/// the type list (to build a FunctionType) from the actual DECL_ARGUMENTS list
/// for the function.  This method takes the DECL_ARGUMENTS list (Args), and
/// fills in Result with the argument types for the function.  It returns the
/// specified result type for the function.
const FunctionType *TypeConverter::
ConvertArgListToFnType(tree ReturnType, tree Args, tree static_chain,
                       unsigned &CallingConv) {
  std::vector<const Type*> ArgTys;
  const Type *RetTy;
  
  FunctionTypeConversion Client(RetTy, ArgTys, CallingConv, true /*K&R*/);
  TheLLVMABI<FunctionTypeConversion> ABIConverter(Client);
  
  ABIConverter.HandleReturnType(ReturnType);

  if (static_chain)
    // Pass the static chain as the first parameter.
    ABIConverter.HandleArgument(TREE_TYPE(static_chain));

  for (; Args && TREE_TYPE(Args) != void_type_node; Args = TREE_CHAIN(Args))
    ABIConverter.HandleArgument(TREE_TYPE(Args));

  FunctionType::ParamAttrsList ParamAttrs;

  // Something for the return type.
  ParamAttrs.push_back(FunctionType::NoAttributeSet);

  if (static_chain) {
    // Pass the static chain in a register.
    ParamAttrs.push_back(FunctionType::InRegAttribute);
  }

  return FunctionType::get(RetTy, ArgTys, false, ParamAttrs);
}

const FunctionType *TypeConverter::ConvertFunctionType(tree type,
                                                       tree static_chain,
                                                       unsigned &CallingConv) {
  const Type *RetTy = 0;
  std::vector<const Type*> ArgTypes;
  bool isVarArg = false;
  FunctionTypeConversion Client(RetTy, ArgTypes, CallingConv, false/*not K&R*/);
  TheLLVMABI<FunctionTypeConversion> ABIConverter(Client);
  
  ABIConverter.HandleReturnType(TREE_TYPE(type));
  
  // Allow the target to set the CC for things like fastcall etc.
#ifdef TARGET_ADJUST_LLVM_CC
  TARGET_ADJUST_LLVM_CC(CallingConv, type);
#endif

  if (static_chain)
    // Pass the static chain as the first parameter.
    ABIConverter.HandleArgument(TREE_TYPE(static_chain));

  // Loop over all of the arguments, adding them as we go.
  tree Args = TYPE_ARG_TYPES(type);
  for (; Args && TREE_VALUE(Args) != void_type_node; Args = TREE_CHAIN(Args)){
    if (!isPassedByInvisibleReference(TREE_VALUE(Args)) &&
        isa<OpaqueType>(ConvertType(TREE_VALUE(Args)))) {
      // If we are passing an opaque struct by value, we don't know how many
      // arguments it will turn into.  Because we can't handle this yet,
      // codegen the prototype as (...).
      if (CallingConv == CallingConv::C)
        ArgTypes.clear();
      else
        ArgTypes.resize(1);   // Don't nuke last argument.
      Args = 0;
      break;        
    }
    
    ABIConverter.HandleArgument(TREE_VALUE(Args));
  }
  
  // If the argument list ends with a void type node, it isn't vararg.
  isVarArg = (Args == 0);
  
  assert(RetTy && "Return type not specified!");

  // If this is the C Calling Convention then scan the FunctionType's result 
  // type and argument types looking for integers less than 32-bits and set
  // the parameter attribute in the FunctionType so any arguments passed to
  // the function will be correctly sign or zero extended to 32-bits by
  // the LLVM code gen.
  FunctionType::ParamAttrsList ParamAttrs;
  unsigned RAttributes = FunctionType::NoAttributeSet;
  if (CallingConv == CallingConv::C) {
    tree ResultTy = TREE_TYPE(type);  
    if (TREE_CODE(ResultTy) == BOOLEAN_TYPE) {
      if (TREE_INT_CST_LOW(TYPE_SIZE(ResultTy)) < INT_TYPE_SIZE)
        RAttributes |= FunctionType::ZExtAttribute;
    } else {
      if (TREE_CODE(ResultTy) == INTEGER_TYPE && 
          TREE_INT_CST_LOW(TYPE_SIZE(ResultTy)) < INT_TYPE_SIZE)
        if (TYPE_UNSIGNED(ResultTy))
          RAttributes |= FunctionType::ZExtAttribute;
        else 
          RAttributes |= FunctionType::SExtAttribute;
    }
  }
  ParamAttrs.push_back(FunctionType::ParameterAttributes(RAttributes));
  
  unsigned Idx = 1;
  bool isFirstArg = true;

  int lparam = 0;
#ifdef LLVM_TARGET_ENABLE_REGPARM
  LLVM_TARGET_INIT_REGPARM(lparam, type);
#endif // LLVM_TARGET_ENABLE_REGPARM

  if (static_chain)
    // Pass the static chain in a register.
    ParamAttrs.push_back(FunctionType::InRegAttribute);
  
  // Handle struct return
  if (ABIConverter.isStructReturn())
    ParamAttrs.push_back(FunctionType::StructRetAttribute);
  
  for (tree Args = TYPE_ARG_TYPES(type);
       Args && TREE_VALUE(Args) != void_type_node; Args = TREE_CHAIN(Args)) {
    unsigned Attributes = FunctionType::NoAttributeSet;
    tree Ty = TREE_VALUE(Args);
    
    if (CallingConv == CallingConv::C) {
      if (TREE_CODE(Ty) == BOOLEAN_TYPE) {
        if (TREE_INT_CST_LOW(TYPE_SIZE(Ty)) < INT_TYPE_SIZE)
          Attributes |= FunctionType::ZExtAttribute;
      } else if (TREE_CODE(Ty) == INTEGER_TYPE && 
                 TREE_INT_CST_LOW(TYPE_SIZE(Ty)) < INT_TYPE_SIZE) {
        if (TYPE_UNSIGNED(Ty))
          Attributes |= FunctionType::ZExtAttribute;
        else
          Attributes |= FunctionType::SExtAttribute;
      }
    }

#ifdef LLVM_TARGET_ENABLE_REGPARM
    if (TREE_CODE(Ty) == INTEGER_TYPE || TREE_CODE(Ty) == POINTER_TYPE)
      LLVM_ADJUST_REGPARM_ATTRIBUTE(Attributes, TREE_INT_CST_LOW(TYPE_SIZE(Ty)),
                                    isVarArg, lparam);
#endif // LLVM_TARGET_ENABLE_REGPARM

    Idx++;

    ParamAttrs.push_back(FunctionType::ParameterAttributes(Attributes));
  }
  return FunctionType::get(RetTy, ArgTypes, isVarArg, ParamAttrs);
}

//===----------------------------------------------------------------------===//
//                      RECORD/Struct Conversion Routines
//===----------------------------------------------------------------------===//

/// StructTypeConversionInfo - A temporary structure that is used when
/// translating a RECORD_TYPE to an LLVM type.
struct StructTypeConversionInfo {
  std::vector<const Type*> Elements;
  std::vector<uint64_t> ElementOffsetInBytes;
  std::vector<uint64_t> ElementSizeInBytes;
  const TargetData &TD;
  unsigned GCCStructAlignmentInBytes;

  StructTypeConversionInfo(TargetMachine &TM, unsigned GCCAlign)
    : TD(*TM.getTargetData()), GCCStructAlignmentInBytes(GCCAlign) {}

  unsigned getGCCStructAlignmentInBytes() const {
    return GCCStructAlignmentInBytes;
  }
  
  /// getTypeAlignment - Return the alignment of the specified type in bytes.
  ///
  unsigned getTypeAlignment(const Type *Ty) const {
    return TD.getABITypeAlignment(Ty);
  }
  
  /// getTypeSize - Return the size of the specified type in bytes.
  ///
  unsigned getTypeSize(const Type *Ty) const {
    return TD.getTypeSize(Ty);
  }
  
  /// getLLVMType - Return the LLVM type for the specified object.
  ///
  const Type *getLLVMType() const {
    return StructType::get(Elements, false);
  }
  
  /// getSizeAsLLVMStruct - Return the size of this struct if it were converted
  /// to an LLVM type.  This is the end of last element push an alignment pad at
  /// the end.
  uint64_t getSizeAsLLVMStruct() const {
    if (Elements.empty()) return 0;
    unsigned MaxAlign = 1;
    for (unsigned i = 0, e = Elements.size(); i != e; ++i)
      MaxAlign = std::max(MaxAlign, getTypeAlignment(Elements[i]));
    
    uint64_t Size = ElementOffsetInBytes.back()+ElementSizeInBytes.back();
    return (Size+MaxAlign-1) & ~(MaxAlign-1);
  }
  
  /// RemoveLastElementIfOverlapsWith - If the last element in the struct
  /// includes the specified byte, remove it.
  void RemoveLastElementIfOverlapsWith(uint64_t ByteOffset) {
    if (Elements.empty()) return;
    assert(ElementOffsetInBytes.back() <= ByteOffset &&
           "Cannot go backwards in struct");
    if (ElementOffsetInBytes.back()+ElementSizeInBytes.back() > ByteOffset) {
      // The last element overlapped with this one, remove it.
      Elements.pop_back();
      ElementOffsetInBytes.pop_back();
      ElementSizeInBytes.pop_back();
    }
  }
  
  /// FieldNo - Remove the specified field and all of the fields that come after
  /// it.
  void RemoveFieldsAfter(unsigned FieldNo) {
    Elements.erase(Elements.begin()+FieldNo, Elements.end());
    ElementOffsetInBytes.erase(ElementOffsetInBytes.begin()+FieldNo, 
                               ElementOffsetInBytes.end());
    ElementSizeInBytes.erase(ElementSizeInBytes.begin()+FieldNo,
                             ElementSizeInBytes.end());
  }
  
  /// getNewElementByteOffset - If we add a new element with the specified
  /// alignment, what byte offset will it land at?
  unsigned getNewElementByteOffset(unsigned ByteAlignment) {
    if (Elements.empty()) return 0;
    uint64_t LastElementEnd = 
      ElementOffsetInBytes.back() + ElementSizeInBytes.back();
    
    return (LastElementEnd+ByteAlignment-1) & ~(ByteAlignment-1);
  }
  
  /// addElement - Add an element to the structure with the specified type,
  /// offset and size.
  void addElement(const Type *Ty, uint64_t Offset, uint64_t Size) {
    Elements.push_back(Ty);
    ElementOffsetInBytes.push_back(Offset);
    ElementSizeInBytes.push_back(Size);
  }
  
  /// getFieldEndOffsetInBytes - Return the byte offset of the byte immediately
  /// after the specified field.  For example, if FieldNo is 0 and the field
  /// is 4 bytes in size, this will return 4.
  unsigned getFieldEndOffsetInBytes(unsigned FieldNo) const {
    assert(FieldNo < ElementOffsetInBytes.size() && "Invalid field #!");
    return ElementOffsetInBytes[FieldNo]+ElementSizeInBytes[FieldNo];
  }
  
  /// getEndUnallocatedByte - Return the first byte that isn't allocated at the
  /// end of a structure.  For example, for {}, it's 0, for {int} it is 4, for
  /// {int,short}, it is 6.
  unsigned getEndUnallocatedByte() const {
    if (ElementOffsetInBytes.empty()) return 0;
    return getFieldEndOffsetInBytes(ElementOffsetInBytes.size()-1);
  }

  /// getLLVMFieldFor - When we are assigning DECL_LLVM indexes to FieldDecls,
  /// this method determines which struct element to use.  Since the offset of
  /// the fields cannot go backwards, CurFieldNo retains the last element we
  /// looked at, to keep this a nice fast linear process.  If isZeroSizeField
  /// is true, this should return some zero sized field that starts at the
  /// specified offset.
  ///
  /// This returns the first field that contains the specified bit.
  ///
  unsigned getLLVMFieldFor(uint64_t FieldOffsetInBits, unsigned &CurFieldNo,
                           bool isZeroSizeField) {
    if (!isZeroSizeField) {
      // Skip over LLVM fields that start and end before the GCC field starts.
      while (CurFieldNo < ElementOffsetInBytes.size() &&
             getFieldEndOffsetInBytes(CurFieldNo)*8 <= FieldOffsetInBits)
        ++CurFieldNo;
      if (CurFieldNo < ElementOffsetInBytes.size())
        return CurFieldNo;
      // Otherwise, we couldn't find the field!
      // FIXME: this works around a latent bug!
      //assert(0 && "Could not find field!");
      return ~0U;
    }

    // Handle zero sized fields now.  If the next field is zero sized, return
    // it.  This is a nicety that causes us to assign C fields different LLVM
    // fields in cases like struct X {}; struct Y { struct X a, b, c };
    if (CurFieldNo+1 < ElementOffsetInBytes.size() &&
        ElementSizeInBytes[CurFieldNo+1] == 0) {
      return ++CurFieldNo;
    }
    
    // Otherwise, if this is a zero sized field, return it.
    if (CurFieldNo < ElementOffsetInBytes.size() &&
        ElementSizeInBytes[CurFieldNo] == 0) {
      return CurFieldNo;
    }
    
    // Otherwise, we couldn't find the field!
    assert(0 && "Could not find field!");
    return ~0U;
  }
  
  void dump() const;
};

void StructTypeConversionInfo::dump() const {
  std::cerr << "Info has " << Elements.size() << " fields:\n";
  for (unsigned i = 0, e = Elements.size(); i != e; ++i) {
    std::cerr << "  Offset = " << ElementOffsetInBytes[i]
              << " Size = " << ElementSizeInBytes[i]
              << " Type = ";
    WriteTypeSymbolic(std::cerr, Elements[i], TheModule) << "\n";
  }
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

/// DecodeStructFields - This method decodes the specified field, if it is a
/// FIELD_DECL, adding or updating the specified StructTypeConversionInfo to
/// reflect it.  
void TypeConverter::DecodeStructFields(tree Field,
                                       StructTypeConversionInfo &Info) {
  if (TREE_CODE(Field) != FIELD_DECL ||
      TREE_CODE(DECL_FIELD_OFFSET(Field)) != INTEGER_CST)
    return;

  // Handle bit-fields specially.
  if (DECL_BIT_FIELD_TYPE(Field)) {
    DecodeStructBitField(Field, Info);
    return;
  }

  // Get the starting offset in the record.
  unsigned StartOffsetInBits = getFieldOffsetInBits(Field);
  assert((StartOffsetInBits & 7) == 0 && "Non-bit-field has non-byte offset!");
  unsigned StartOffsetInBytes = StartOffsetInBits/8;
  
  // Pop any previous elements out of the struct if they overlap with this one.
  // This can happen when the C++ front-end overlaps fields with tail padding in
  // C++ classes.
  Info.RemoveLastElementIfOverlapsWith(StartOffsetInBytes);
  
  // Get the LLVM type for the field.  If this field is a bitfield, use the
  // declared type, not the shrunk-to-fit type that GCC gives us in TREE_TYPE.
  const Type *Ty = ConvertType(TREE_TYPE(Field));
  unsigned ByteAlignment = Info.getTypeAlignment(Ty);
  unsigned NextByteOffset = Info.getNewElementByteOffset(ByteAlignment);
  if (NextByteOffset > StartOffsetInBytes || 
      ByteAlignment > Info.getGCCStructAlignmentInBytes()) {
    // If the LLVM type is aligned more than the GCC type, we cannot use this
    // LLVM type for the field.  Instead, insert the field as an array of bytes,
    // which we know will have the least alignment possible.
    Ty = ArrayType::get(Type::Int8Ty, Info.getTypeSize(Ty));
    ByteAlignment = 1;
    NextByteOffset = Info.getNewElementByteOffset(ByteAlignment);
  }
    
  // If alignment won't round us up to the right boundary, insert explicit
  // padding.
  if (NextByteOffset < StartOffsetInBytes) {
    unsigned CurOffset = Info.getNewElementByteOffset(1);
    const Type *Pad = Type::Int8Ty;
    if (StartOffsetInBytes-CurOffset != 1)
      Pad = ArrayType::get(Pad, StartOffsetInBytes-CurOffset);
    Info.addElement(Pad, CurOffset, StartOffsetInBytes-CurOffset);
  }
  
  // At this point, we know that adding the element will happen at the right
  // offset.  Add it.
  Info.addElement(Ty, StartOffsetInBytes, Info.getTypeSize(Ty));
}

/// DecodeStructBitField - This method decodes the specified bit-field, adding
/// or updating the specified StructTypeConversionInfo to reflect it.
///
/// Note that in general, we cannot produce a good covering of struct fields for
/// bitfields.  As such, we only make sure that all bits in a struct that
/// correspond to a bitfield are represented in the LLVM struct with
/// (potentially multiple) integer fields of integer type.  This ensures that
/// initialized globals with bitfields can have the initializers for the
/// bitfields specified.
void TypeConverter::DecodeStructBitField(tree_node *Field, 
                                         StructTypeConversionInfo &Info) {
  unsigned FieldSizeInBits = TREE_INT_CST_LOW(DECL_SIZE(Field));
  if (FieldSizeInBits == 0)   // Ignore 'int:0', which just affects layout.
    return;

  // Get the starting offset in the record.
  unsigned StartOffsetInBits = getFieldOffsetInBits(Field);
  unsigned EndBitOffset    = FieldSizeInBits+StartOffsetInBits;
  
  // If  the last inserted LLVM field completely contains this bitfield, just
  // ignore this field.
  if (!Info.Elements.empty()) {
    // If the last field does not completely contain *this* bitfield, extend
    // it.
    unsigned LastFieldBitOffset = Info.ElementOffsetInBytes.back()*8;
    unsigned LastFieldBitSize   = Info.ElementSizeInBytes.back()*8;
    assert(LastFieldBitOffset < StartOffsetInBits &&
           "This bitfield isn't part of the last field!");
    if (EndBitOffset <= LastFieldBitOffset+LastFieldBitSize &&
        LastFieldBitOffset+LastFieldBitSize >= StartOffsetInBits)
      return;  // Already contained in previous field!
  }
  
  // Otherwise, this bitfield lives (potentially) partially in the preceeding
  // field and in fields that exist after it.  Add integer-typed fields to the
  // LLVM struct such that there are no holes in the struct where the bitfield
  // is: these holes would make it impossible to statically initialize a global
  // of this type that has an initializer for the bitfield.
  
  // Compute the number of bits that we need to add to this struct to cover
  // this field.
  unsigned NumBitsToAdd = FieldSizeInBits;
  unsigned FirstUnallocatedByte = Info.getEndUnallocatedByte();
  // If there are any, subtract off bits that go in the previous field.
  if (StartOffsetInBits < FirstUnallocatedByte*8) {
    NumBitsToAdd -= FirstUnallocatedByte*8-StartOffsetInBits;
  } else if (StartOffsetInBits > FirstUnallocatedByte*8) {
    // If there is padding between the last field and the struct, insert
    // explicit bytes into the field to represent it.
    assert((StartOffsetInBits & 7) == 0 &&
           "Next field starts on a non-byte boundary!");
    unsigned PadBytes = StartOffsetInBits/8-FirstUnallocatedByte;
    const Type *Pad = Type::Int8Ty;
    if (PadBytes != 1)
      Pad = ArrayType::get(Pad, PadBytes);
    Info.addElement(Pad, FirstUnallocatedByte, PadBytes);
    FirstUnallocatedByte = StartOffsetInBits/8;
  }

  // Figure out the LLVM type that we will use for the new field.
  const Type *NewFieldTy;
  if (NumBitsToAdd <= 8)
    NewFieldTy = Type::Int8Ty;
  else if (NumBitsToAdd <= 16)
    NewFieldTy = Type::Int16Ty;
  else if (NumBitsToAdd <= 32)
    NewFieldTy = Type::Int32Ty;
  else {
    assert(NumBitsToAdd <= 64 && "Bitfield too large!");
    NewFieldTy = Type::Int64Ty;
  }

  // Check that the alignment of NewFieldTy won't cause a gap in the structure!
  unsigned ByteAlignment = Info.getTypeAlignment(NewFieldTy);
  if (FirstUnallocatedByte & (ByteAlignment-1)) {
    // Instead of inserting a nice whole field, insert a small array of ubytes.
    NewFieldTy = ArrayType::get(Type::Int8Ty, (NumBitsToAdd+7)/8);
  }
  
  // Finally, add the new field.
  Info.addElement(NewFieldTy, FirstUnallocatedByte,
                  Info.getTypeSize(NewFieldTy));
}


/// ConvertRECORD - We know that 'type' is a RECORD_TYPE: convert it to an LLVM
/// type.
const Type *TypeConverter::ConvertRECORD(tree type, tree orig_type) {
  if (const Type *Ty = GET_TYPE_LLVM(type)) {
    // If we already compiled this type, and if it was not a forward
    // definition that is now defined, use the old type.
    if (!isa<OpaqueType>(Ty) || TYPE_SIZE(type) == 0)
      return Ty;
  }
  
  if (TYPE_SIZE(type) == 0) {   // Forward declaration?
    const Type *Ty = OpaqueType::get();
    TheModule->addTypeName(GetTypeName("struct.", orig_type), Ty);
    return TypeDB.setType(type, Ty);
  }
  
  // Note that we are compiling a struct now.
  bool OldConvertingStruct = ConvertingStruct;
  ConvertingStruct = true;
  
  // Construct LLVM types for the base types... in order to get names for
  // base classes and to ensure they are laid out.
  if (tree binfo = TYPE_BINFO(type)) {
    for (unsigned i = 0, e = BINFO_N_BASE_BINFOS(binfo); i != e; ++i)
      ConvertType(BINFO_TYPE(BINFO_BASE_BINFO(binfo, i)));
  }
  
  StructTypeConversionInfo Info(*TheTarget, TYPE_ALIGN_UNIT(type));
  
  // Convert over all of the elements of the struct.
  for (tree Field = TYPE_FIELDS(type); Field; Field = TREE_CHAIN(Field))
    DecodeStructFields(Field, Info);
  
  // If the LLVM struct requires explicit tail padding to be the same size as
  // the GCC struct, insert tail padding now.  This handles, e.g., "{}" in C++.
  if (TYPE_SIZE(type) && TREE_CODE(TYPE_SIZE(type)) == INTEGER_CST) {
    uint64_t LLVMStructSize = Info.getSizeAsLLVMStruct();
    uint64_t GCCTypeSize = ((uint64_t)TREE_INT_CST_LOW(TYPE_SIZE(type))+7)/8;
    
    if (LLVMStructSize != GCCTypeSize) {
      assert(LLVMStructSize < GCCTypeSize &&
             "LLVM type size doesn't match GCC type size!");
      uint64_t LLVMLastElementEnd = Info.getNewElementByteOffset(1);
      const Type *PadTy = Type::Int8Ty;
      if (GCCTypeSize-LLVMLastElementEnd != 1)
        PadTy = ArrayType::get(PadTy, GCCTypeSize-LLVMStructSize);
      Info.addElement(PadTy, GCCTypeSize-LLVMLastElementEnd, 
                      GCCTypeSize-LLVMLastElementEnd);
    }
  }
  
  // Now that the LLVM struct is finalized, figure out a safe place to index to
  // and set the DECL_LLVM values for each FieldDecl that doesn't start at a
  // variable offset.
  unsigned CurFieldNo = 0;
  for (tree Field = TYPE_FIELDS(type); Field; Field = TREE_CHAIN(Field))
    if (TREE_CODE(Field) == FIELD_DECL &&
        TREE_CODE(DECL_FIELD_OFFSET(Field)) == INTEGER_CST) {
      unsigned FieldOffsetInBits = getFieldOffsetInBits(Field);
      tree FieldType = TREE_TYPE(Field);
      
      // If this is a bitfield, we may want to adjust the FieldOffsetInBits to
      // produce safe code.  In particular, bitfields will be loaded/stored as
      // their *declared* type, not the smallest integer type that contains
      // them.  As such, we need to respect the alignment of the declared type.
      if (tree DeclaredType = DECL_BIT_FIELD_TYPE(Field)) {
        // If this is a bitfield, the declared type must be an integral type.
        const Type *DeclFieldTy = ConvertType(DeclaredType);
        unsigned DeclBitAlignment = Info.getTypeAlignment(DeclFieldTy)*8;
        
        FieldOffsetInBits &= ~(DeclBitAlignment-1ULL);
      }
      
      // Figure out if this field is zero bits wide, e.g. {} or [0 x int].  Do
      // not include variable sized fields here.
      bool isZeroSizeField = !TYPE_SIZE(FieldType) ||
        integer_zerop(TYPE_SIZE(FieldType));

      unsigned FieldNo = 
        Info.getLLVMFieldFor(FieldOffsetInBits, CurFieldNo, isZeroSizeField);
      SET_DECL_LLVM(Field, ConstantInt::get(Type::Int32Ty, FieldNo));
    }
  
  const Type *ResultTy = Info.getLLVMType();
  
  const OpaqueType *OldTy = cast_or_null<OpaqueType>(GET_TYPE_LLVM(type));
  TypeDB.setType(type, ResultTy);
  
  // If there was a forward declaration for this type that is now resolved,
  // refine anything that used it to the new type.
  if (OldTy)
    const_cast<OpaqueType*>(OldTy)->refineAbstractTypeTo(ResultTy);
  
  // Finally, set the name for the type.
  TheModule->addTypeName(GetTypeName("struct.", orig_type),
                         GET_TYPE_LLVM(type));
  
  // We have finished converting this struct.  See if the is the outer-most
  // struct being converted by ConvertType.
  ConvertingStruct = OldConvertingStruct;
  if (!ConvertingStruct) {
    
    // If this is the outer-most level of structness, resolve any pointers
    // that were deferred.
    while (!PointersToReresolve.empty()) {
      if (tree PtrTy = PointersToReresolve.back()) {
        ConvertType(PtrTy);   // Reresolve this pointer type.
        assert((PointersToReresolve.empty() ||
                PointersToReresolve.back() != PtrTy) &&
               "Something went wrong with pointer resolution!");
      } else {
        // Null marker element.
        PointersToReresolve.pop_back();
      }
    }
  }
  
  return GET_TYPE_LLVM(type);
}


/// ConvertUNION - We know that 'type' is a UNION_TYPE: convert it to an LLVM
/// type.
const Type *TypeConverter::ConvertUNION(tree type, tree orig_type) {
  if (const Type *Ty = GET_TYPE_LLVM(type)) {
    // If we already compiled this type, and if it was not a forward
    // definition that is now defined, use the old type.
    if (!isa<OpaqueType>(Ty) || TYPE_SIZE(type) == 0)
      return Ty;
  }
  
  if (TYPE_SIZE(type) == 0) {   // Forward declaraion?
    const Type *Ty = OpaqueType::get();
    TheModule->addTypeName(GetTypeName("union.", orig_type), Ty);
    return TypeDB.setType(type, Ty);
  }
  
  // If this is a union with the transparent_union attribute set, it is
  // treated as if it were just the same as its first type.
  if (TYPE_TRANSPARENT_UNION(type)) {
    tree Field = TYPE_FIELDS(type);
    assert(Field && "Transparent union must have some elements!");
    while (TREE_CODE(Field) != FIELD_DECL) {
      Field = TREE_CHAIN(Field);
      assert(Field && "Transparent union must have some elements!");
    }
    
    return TypeDB.setType(type, ConvertType(TREE_TYPE(Field)));
  }
  
  // Note that we are compiling a struct now.
  bool OldConvertingStruct = ConvertingStruct;
  ConvertingStruct = true;
  
  // Find the type with the largest size, and if we have multiple things with
  // the same size, the thing with the largest alignment.
  const TargetData &TD = getTargetData();
  const Type *UnionTy = 0;
  unsigned MaxSize = 0, MaxAlign = 0;
  Value *Idx = Constant::getNullValue(Type::Int32Ty);
  for (tree Field = TYPE_FIELDS(type); Field; Field = TREE_CHAIN(Field)) {
    if (TREE_CODE(Field) != FIELD_DECL) continue;
    assert(getFieldOffsetInBits(Field) == 0 && "Union with non-zero offset?");

    // Set the field idx to zero for all fields.
    SET_DECL_LLVM(Field, Idx);
    
    const Type *TheTy = ConvertType(TREE_TYPE(Field));
    unsigned Size     = TD.getTypeSize(TheTy);
    unsigned Align = TD.getABITypeAlignment(TheTy);
    if (UnionTy == 0 || Size>MaxSize || (Size == MaxSize && Align > MaxAlign)) {
      UnionTy = TheTy;
      MaxSize = Size;
      MaxAlign = Align;
    }
  }

  std::vector<const Type*> UnionElts;
  unsigned UnionSize = 0;
  if (UnionTy) {            // Not an empty union.
    UnionSize = TD.getTypeSize(UnionTy);
    UnionElts.push_back(UnionTy);
  }
  
  // If the LLVM struct requires explicit tail padding to be the same size as
  // the GCC union, insert tail padding now.  This handles cases where the union
  // has larger alignment than the largest member does, thus requires tail
  // padding.
  if (TYPE_SIZE(type) && TREE_CODE(TYPE_SIZE(type)) == INTEGER_CST) {
    unsigned GCCTypeSize = ((unsigned)TREE_INT_CST_LOW(TYPE_SIZE(type))+7)/8;
    
    if (UnionSize != GCCTypeSize) {
      assert(UnionSize < GCCTypeSize &&
             "LLVM type size doesn't match GCC type size!");
      const Type *PadTy = Type::Int8Ty;
      if (GCCTypeSize-UnionSize != 1)
        PadTy = ArrayType::get(PadTy, GCCTypeSize-UnionSize);
      UnionElts.push_back(PadTy);
    }
  }
  
  // If this is an empty union, but there is tail padding, make a filler.
  if (UnionTy == 0) {
    unsigned Size = ((unsigned)TREE_INT_CST_LOW(TYPE_SIZE(type))+7)/8;
    UnionTy = Type::Int8Ty;
    if (Size != 1) UnionTy = ArrayType::get(UnionTy, Size);
  }
  
  const Type *ResultTy = StructType::get(UnionElts, false);
  const OpaqueType *OldTy = cast_or_null<OpaqueType>(GET_TYPE_LLVM(type));
  TypeDB.setType(type, ResultTy);
  
  // If there was a forward declaration for this type that is now resolved,
  // refine anything that used it to the new type.
  if (OldTy)
    const_cast<OpaqueType*>(OldTy)->refineAbstractTypeTo(ResultTy);
  
  // Finally, set the name for the type.
  TheModule->addTypeName(GetTypeName("struct.", orig_type),
                         GET_TYPE_LLVM(type));
  
  // We have finished converting this union.  See if the is the outer-most
  // union being converted by ConvertType.
  ConvertingStruct = OldConvertingStruct;
  if (!ConvertingStruct) {
    // If this is the outer-most level of structness, resolve any pointers
    // that were deferred.
    while (!PointersToReresolve.empty()) {
      if (tree PtrTy = PointersToReresolve.back()) {
        ConvertType(PtrTy);   // Reresolve this pointer type.
        assert((PointersToReresolve.empty() ||
                PointersToReresolve.back() != PtrTy) &&
               "Something went wrong with pointer resolution!");
      } else {
        // Null marker element.
        PointersToReresolve.pop_back();
      }
    }
  }
  
  return GET_TYPE_LLVM(type);
}

/* APPLE LOCAL end LLVM (ENTIRE FILE!)  */

