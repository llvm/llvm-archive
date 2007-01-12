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
/* This file is one possible implementation of the LLVM pool allocator        */
/* runtime library.                                                           */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

#include "PoolCheck.h"
#ifdef LLVA_KERNEL
#include <stdarg.h>
#endif
#define DEBUG(x) 

/*===----------------------------------------------------------------------===*/
extern unsigned PageSize;

/*
 * Function: poolcheckinit()
 *
 * Description:
 *  Initialization function to be called when the memory allocator run-time
 *  intializes itself.  For now, this does nothing.
 */
void
poolcheckinit(void *Pool, unsigned NodeSize) {
  return;
}

/*
 * Function: poolcheckdestroy()
 *
 * Description:
 *  To be called from pooldestroy.
 */
void
poolcheckdestroy(void *Pool) {
  /* Do nothing as of now since all MetaPools are global */
#if 0
  free_splay(Pool->splay);
#endif
  return;
}


void AddPoolDescToMetaPool(MetaPoolTy **MP, void *P) {
  MetaPoolTy  *MetaPoolPrev = *MP;
  MetaPoolTy *MetaPool = *MP;
  if (MetaPool) {
    MetaPool = MetaPool->next;
  } else {
    *MP = (MetaPoolTy *) poolcheckmalloc (sizeof(MetaPoolTy));
    (*MP)->Pool = P;
    (*MP)->next = 0;
    return;
  }
  while (MetaPool) {
    MetaPoolPrev = MetaPool;
    MetaPool = MetaPool->next;
  }
  /* MetaPool is null; */
  MetaPoolPrev->next = (MetaPoolTy *) poolcheckmalloc (sizeof(MetaPoolTy));
  MetaPoolPrev->next->Pool = P;
  MetaPoolPrev->next->next = 0;
}


unsigned char poolcheckoptim(void *Pool, void *Node) {
  Splay *psplay;
  Splay *ref;
  /* PageSize needs to be modified accordingly */
#if 0
  void *PS = (void *)((unsigned)Node & ~(PageSize-1));
#else
  /* Slab Size in bytes */
  unsigned int SlabSize = poolcheckslabsize (Pool);
  void *PS = (void *)((unsigned)Node & ~(SlabSize-1));
#endif
  PoolCheckSlab * PCS = poolcheckslab(Pool);
  while (PCS) {
    if (PCS->Slab == PS) return true;
    /* we can optimize by moving it to the front of the list */
    PCS = PCS->nextSlab;
  }
  /* here we check for the splay tree */
  psplay = poolchecksplay(Pool);
  ref = splay_find_ptr(psplay, (unsigned long) Node);
  if (ref) {
    return true;
  }
  return false;
}


inline unsigned char refcheck(Splay *splay, void *Node) {
  unsigned long base = (unsigned long) (splay->key);
  unsigned long length = (unsigned long) (splay->val);
  unsigned long result = (unsigned long) Node;
  if ((result >= base) && (result < (base + length))) return true;
  return false;
}


unsigned char poolcheckarrayoptim(MetaPoolTy *Pool, void *NodeSrc, void *NodeResult) {
  Splay *psplay = poolchecksplay(Pool);
  Splay *ref = splay_find_ptr(psplay, (unsigned long)NodeSrc);
  if (ref) {
    return refcheck(ref, NodeResult);
  } 
  return false;
}

void poolcheckarray(MetaPoolTy **MP, void *NodeSrc, void *NodeResult) {
  MetaPoolTy *MetaPool = *MP;
  if (!MetaPool) {
    poolcheckfail ("Empty meta pool? Src \n", NodeSrc);
  }
  /*
   * iteratively search through the list
   * Check if there are other efficient data structures.
   */
  while (MetaPool) {
    void *Pool = MetaPool->Pool;
    if (poolcheckarrayoptim(Pool, NodeSrc, NodeResult)) return ;
    MetaPool = MetaPool->next;
  }
  poolcheckfail ("poolcheck failure: Result \n", NodeResult);
}

void poolcheck(MetaPoolTy **MP, void *Node) {
  MetaPoolTy *MetaPool = *MP;
  if (!MetaPool) {
    poolcheckfail ("Empty meta pool? \n", Node);
  }
  /*
   * iteratively search through the list
   * Check if there are other efficient data structures.
  */
  
  while (MetaPool) {
    void *Pool = MetaPool->Pool;
    if (poolcheckoptim(Pool, Node))   return;
    MetaPool = MetaPool->next;
  }
  poolcheckfail ("poolcheck failure \n", Node);
}


void poolcheckAddSlab(PoolCheckSlab **PCSPtr, void *Slab) {
  PoolCheckSlab  *PCSPrev = *PCSPtr;
  PoolCheckSlab *PCS = *PCSPtr;
  if (PCS) {
    PCS = PCS->nextSlab;
  } else {
    *PCSPtr = (PoolCheckSlab *) poolcheckmalloc (sizeof(PoolCheckSlab));
    (*PCSPtr)->Slab = Slab;
    (*PCSPtr)->nextSlab = 0;
    return;
  }
  while (PCS) {
    PCSPrev = PCS;
    PCS = PCS->nextSlab;
  }
  /* PCS is null; */
  PCSPrev->nextSlab = (PoolCheckSlab *) poolcheckmalloc (sizeof(PoolCheckSlab));
  PCSPrev->nextSlab->Slab = Slab;
  PCSPrev->nextSlab->nextSlab = 0;
}


  void exactcheck(int a, int b) {
    if ((0 > a) || (a >= b)) {
      poolcheckfail ("exact check failed\n", a);
    }
  }

  /*
   * Disable this for kernel code.  I'm not sure how kernel code handles
   * va_list type functions.
   */
#ifdef LLVA_KERNEL
  void funccheck(unsigned num, void *f, void *g, ...) {
    va_list ap;
    unsigned i = 0;
    if (f == g) return;
    i++;
    va_start(ap, g);
    for ( ; i != num; ++i) {
      void *h = va_arg(ap, void *);
      if (f == h) {
	return;
      }
    }
    abort();
  }
#endif

  void poolcheckregister(Splay *splay, void * allocaptr, unsigned NumBytes) {
    splay_insert_ptr(splay, (unsigned long)(allocaptr), NumBytes);
  }
