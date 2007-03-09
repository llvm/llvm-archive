/*===- PoolCheck.cpp - Implementation of poolcheck runtime ----------------===*/
/*                                                                            */
/*                       The LLVM Compiler Infrastructure                     */
/*                                                                            */
/* This file was developed by the LLVM research group and is distributed      */
/* under the University of Illinois Open Source License. See LICENSE.TXT for  */
/* details.                                                                   */
/*                                                                            */
/*===----------------------------------------------------------------------===*/
/*                                                                            */
/* This file implements the poolcheck interface w/ metapools and opaque       */
/* pool ids.                                                                  */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

#include "PoolCheck.h"
#include "PoolSystem.h"
#include "adl_splay.h"
#ifdef LLVA_KERNEL
#include <stdarg.h>
#endif
#define DEBUG(x) 

/* Flag whether we are ready to perform pool operations */
static int ready = 0;

static const int use_oob = 0;

/*
 * Function: pchk_init()
 *
 * Description:
 *  Initialization function to be called when the memory allocator run-time
 *  intializes itself.
 *
 * Preconditions:
 *  1) The OS kernel is able to handle callbacks from the Execution Engine.
 */
void pchk_init(void) {

  /* initialize runtime */
  adl_splay_libinit(poolcheckmalloc);

  /*
   * Register all of the global variables in their respective meta pools.
   */
  poolcheckglobals();

  /*
   * Flag that we're ready to rumble!
   */
  ready = 1;
  return;
}

/* Register a slab */
void pchk_reg_slab(MetaPoolTy* MP, void* PoolID, void* addr, unsigned len) {
  if (!MP) { poolcheckinfo("reg slab on null pool", (int)addr); return; }
  adl_splay_insert(&MP->Slabs, addr, len, PoolID);
}

/* Remove a slab */
void pchk_drop_slab(MetaPoolTy* MP, void* PoolID, void* addr) {
  if (!MP) return;
  /* TODO: check that slab's tag is == PoolID */
  adl_splay_delete(&MP->Slabs, addr);
}

/* Register a non-pool allocated object */
void pchk_reg_obj(MetaPoolTy* MP, void* addr, unsigned len) {
  if (!MP) { poolcheckinfo("reg obj on null pool", (int)addr); return; }
  adl_splay_insert(&MP->Objs, addr, len, 0);
}

/* Remove a non-pool allocated object */
void pchk_drop_obj(MetaPoolTy* MP, void* addr) {
  if (!MP) return;
  adl_splay_delete(&MP->Objs, addr);
}

/* Register a pool */
/* The MPLoc is the location the pool wishes to store the metapool tag for */
/* the pool PoolID is in at. */
/* MP is the actual metapool. */
void pchk_reg_pool(MetaPoolTy* MP, void* PoolID, void* MPLoc) {
  if(!MP) return;
  *(void**)MPLoc = (void*) MP;
}

/* A pool is deleted.  free it's resources (necessary for correctness of checks) */
void pchk_drop_pool(MetaPoolTy* MP, void* PoolID) {
  if(!MP) return;
  adl_splay_delete_tag(&MP->Slabs, PoolID);
}

/* check that addr exists in pool MP */
void poolcheck(MetaPoolTy* MP, void* addr) {
  if (!ready || !MP) return;
  if (adl_splay_find(&MP->Slabs, addr) || adl_splay_find(&MP->Objs, addr))
    return;
  poolcheckfail ("poolcheck failure: \n", (unsigned)addr, (void*)__builtin_return_address(0));
}

/* check that src and dest are same obj or slab */
void poolcheckarray(MetaPoolTy* MP, void* src, void* dest) {
  if (!ready || !MP) return;
  /* try slabs first */
  void* S = src;
  void* D = dest;
  adl_splay_retrieve(&MP->Slabs, &S, 0, 0);
  adl_splay_retrieve(&MP->Slabs, &D, 0, 0);
  if (S == D)
    return;
  /* try objs */
  S = src;
  D = dest;
  adl_splay_retrieve(&MP->Objs, &S, 0, 0);
  adl_splay_retrieve(&MP->Objs, &D, 0, 0);
  if (S == D)
    return;
  poolcheckfail ("poolcheck failure: \n", (unsigned)src, (void*)__builtin_return_address(0));
}

/* check that src and dest are same obj or slab */
/* if src and dest do not exist in the pool, pass */
void poolcheckarray_i(MetaPoolTy* MP, void* src, void* dest) {
  if (!ready || !MP) return;
  /* try slabs first */
  void* S = src;
  void* D = dest;
  int fs = adl_splay_retrieve(&MP->Slabs, &S, 0, 0);
  int fd = adl_splay_retrieve(&MP->Slabs, &D, 0, 0);
  if (S == D)
    return;
  if (fs || fd) { /*fail if we found one but not the other*/ 
    poolcheckfail ("poolcheck failure: \n", (unsigned)src, (void*)__builtin_return_address(0));
    return;
  }
  /* try objs */
  S = src;
  D = dest;
  fs = adl_splay_retrieve(&MP->Objs, &S, 0, 0);
  fd = adl_splay_retrieve(&MP->Objs, &D, 0, 0);
  if (S == D)
    return;
  if (fs || fd) { /*fail if we found one but not the other*/
    poolcheckfail ("poolcheck failure: \n", (unsigned)src, (void*)__builtin_return_address(0));
    return;
  }
  return; /*default is to pass*/
}

const unsigned InvalidUpper = 4096;
const unsigned InvalidLower = 0x03;


/* if src is an out of object pointer, get the original value */
void* pchk_getActualValue(MetaPoolTy* MP, void* src) {
  if (!ready || !MP) return src;
  if ((unsigned)src <= InvalidLower) return src;
  void* tag = 0;
  /* outside rewrite zone */
  if ((unsigned)src & ~(InvalidUpper - 1)) return src;
  if (adl_splay_retrieve(&MP->OOB, &src, 0, &tag))
    return tag;
  poolcheckfail("GetActualValue failure: \n", (unsigned) src, (void*)__builtin_return_address(0));
  return tag;
}


inline  void exactcheck2(signed char *base, signed char *result, unsigned size) {
  if (result >= base + size ) {
    poolcheckfail("Array bounds violation detected \n", (unsigned)base, (void*)__builtin_return_address(0));
  }
}

/*
 * Function: boundscheck()
 *
 * Description:
 *  Perform a precise array bounds check on source and result.  If the result
 *  is out of range for the array, return 0x1 so that getactualvalue() will
 *  know that the pointer is bad and should not be dereferenced.
 */
void* pchk_bounds(MetaPoolTy* MP, void* src, void* dest) {
  if (!ready || !MP) return dest;
  /* try objs */
  void* S = src;
  void* D = dest;
  int fs = adl_splay_retrieve(&MP->Objs, &S, 0, 0);
  adl_splay_retrieve(&MP->Objs, &D, 0, 0);
  if (S == D)
    return dest;
  else if (fs) {
    if (!use_oob) return dest;
    if (MP->invalidptr == 0) MP->invalidptr = (unsigned char*)InvalidLower;
    ++MP->invalidptr;
    if ((unsigned)MP->invalidptr & ~(InvalidUpper - 1)) {
      poolcheckfail("poolcheck failure: out of rewrite ptrs\n", 0, (void*)__builtin_return_address(0));
      return dest;
    }
    poolcheckinfo("Returning oob pointer of ", (int)MP->invalidptr);
    adl_splay_insert(&MP->OOB, MP->invalidptr, 1, dest);
    return MP->invalidptr;
  }

  /* try slabs */
  S = src;
  D = dest;
  fs = adl_splay_retrieve(&MP->Slabs, &S, 0, 0);
  adl_splay_retrieve(&MP->Slabs, &D, 0, 0);
  if (S == D)
    return dest;
  else if (fs) {
    if (!use_oob) return dest;
    if (MP->invalidptr == 0) MP->invalidptr = (unsigned char*)InvalidLower;
    ++MP->invalidptr;
    if ((unsigned)MP->invalidptr & ~(InvalidUpper - 1)) {
      poolcheckfail("poolcheck failure: out of rewrite ptrs\n", 0, (void*)__builtin_return_address(0));
      return dest;
    }
    poolcheckinfo("Returning oob pointer of ", (int)MP->invalidptr);
    adl_splay_insert(&MP->OOB, MP->invalidptr, 1, dest);
    return MP->invalidptr;
  }

  /*
   * The node is not found or is not within bounds; fail!
   */
  poolcheckfail ("boundscheck failure 1\n", (unsigned)src, (void*)__builtin_return_address(0));
  return dest;
}

/*
 * Function: uiboundscheck()
 *
 * Description:
 *  Perform a precise array bounds check on source and result.  If the result
 *  is out of range for the array, return a sentinel so that getactualvalue()
 *  will know that the pointer is bad and should not be dereferenced.
 *
 *  This version differs from boundscheck() in that it does not generate a
 *  poolcheck failure if the source node cannot be found within the MetaPool.
 */
void* pchk_bounds_i(MetaPoolTy* MP, void* src, void* dest) {
  if (!ready || !MP) return dest;
  /* try objs */
  void* S = src;
  void* D = dest;
  int fs = adl_splay_retrieve(&MP->Objs, &S, 0, 0);
  adl_splay_retrieve(&MP->Objs, &D, 0, 0);
  if (S == D)
    return dest;
  else if (fs) {
    if (!use_oob) return dest;
     if (MP->invalidptr == 0) MP->invalidptr = (unsigned char*)0x03;
    ++MP->invalidptr;
    if ((unsigned)MP->invalidptr & ~(InvalidUpper - 1)) {
      poolcheckfail("poolcheck failure: out of rewrite ptrs\n", 0, (void*)__builtin_return_address(0));
      return dest;
    }
    adl_splay_insert(&MP->OOB, MP->invalidptr, 1, dest);
    return MP->invalidptr;
  }

  /* try slabs */
  S = src;
  D = dest;
  fs = adl_splay_retrieve(&MP->Slabs, &S, 0, 0);
  adl_splay_retrieve(&MP->Slabs, &D, 0, 0);
  if (S == D)
    return dest;
  else if (fs) {
    if (!use_oob) return dest;
     if (MP->invalidptr == 0) MP->invalidptr = (unsigned char*)0x03;
    ++MP->invalidptr;
    if ((unsigned)MP->invalidptr & ~(InvalidUpper - 1)) {
      poolcheckfail("poolcheck failure: out of rewrite ptrs\n", 0, (void*)__builtin_return_address(0));
      return dest;
    }
    adl_splay_insert(&MP->OOB, MP->invalidptr, 1, dest);
    return MP->invalidptr;
  }

  /*
   * The node is not found or is not within bounds; pass!
   */
  return dest;
}

void exactcheck(int a, int b) {
  if ((0 > a) || (a >= b)) {
    poolcheckfail ("exact check failed\n", (a), (void*)__builtin_return_address(0));
    poolcheckfail ("exact check failed\n", (b), (void*)__builtin_return_address(0));
  }
}
