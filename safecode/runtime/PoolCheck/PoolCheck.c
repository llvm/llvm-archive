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

#if 1
/*
 * These are symbols exported by the kernel so that we can figure out where
 * various sections are.
 */
extern char _etext;
extern char _edata;
extern char __bss_start;
extern char _end;
#endif

/*===----------------------------------------------------------------------===*/
extern unsigned PageSize;

/* Flag whether we are ready to perform pool operations */
static int ready = 0;

/*
 * Function: poolcheckinit()
 *
 * Description:
 *  Initialization function to be called when the memory allocator run-time
 *  intializes itself.  For now, this does nothing.
 */
void
poolcheckinit(void *Pool, unsigned NodeSize) {
  ready = 1;
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
  ready = 0;
  return;
}


void AddPoolDescToMetaPool(MetaPoolTy **MP, void *P) {
  if (!ready) return;
  MetaPoolTy  *MetaPoolPrev = *MP;
  MetaPoolTy *MetaPool = *MP;
  if (MetaPool) {
    MetaPool = MetaPool->next;
  } else {
    if (*MP = (MetaPoolTy *) poolcheckmalloc (sizeof(MetaPoolTy)))
    {
      (*MP)->Pool = P;
      (*MP)->next = 0;
    }
    return;
  }

  /*
   * Scan for the end of the list.  If we run across the pool already in the
   * meta-pool list, then don't bother adding it again.
   */
  while (MetaPool) {
    if (MetaPool->Pool == P) {
      return;
    }
    MetaPoolPrev = MetaPool;
    MetaPool = MetaPool->next;
  }
  /* MetaPool is null; */
  if (MetaPoolPrev->next = (MetaPoolTy *) poolcheckmalloc (sizeof(MetaPoolTy)))
  {
    MetaPoolPrev->next->Pool = P;
    MetaPoolPrev->next->next = 0;
  }
}


/*
 * Function: poolcheckoptim()
 *
 * Description:
 *  Determine whether the pointer is within the pool.
 *
 * Return value:
 *  true  - The pointer is within the pool.
 *  false - The pointer is not within the pool.
 */
unsigned char poolcheckoptim(void *Pool, void *Node) {
  Splay *psplay;
  Splay *ref;
#if 0
  /*
   * Determine if this is a global variable in the data section.
   */
  if (((Node) >= &_etext) && ((Node) <= &_edata))
  {
    return;
  }
                                                                                
  /*
   * Determine if this is a global variable in the BSS section.
   */
  if ((((Node)) >= &__bss_start) && (((Node)) <= &_end))
  {
    return;
  }
#endif

  /*
   * Determine the size of the slabs used in this pool.
   * Then, assuming this is the pool in which this node lives, determine
   * the slab to which the node would belong.
   */
  unsigned int SlabSize = poolcheckslabsize (Pool);
  void *PS = (void *)((unsigned)Node & ~(SlabSize-1));
  poolcheckinfo ("Slab Size is ", SlabSize);
  poolcheckinfo ("Looking for slab", PS);

  /*
   * Scan through the list of slabs belonging to the pool and determine
   * whether this node is in one of those slabs.
   */
  PoolCheckSlab * PCS = poolcheckslab(Pool);
  while (PCS) {
    poolcheckinfo ("Checking slab", PCS->Slab);
    if (PCS->Slab == PS) return true;
    /* we can optimize by moving it to the front of the list */
    PCS = PCS->nextSlab;
  }

  /*
   * Here we check for the splay tree
   */
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


/*
 * Function: poolcheckarrayoptim()
 *
 * Description:
 *  This function determines:
 *    1) Whether the source node of a GEP is in the correct pool, and
 *    2) Whether the result of the GEP expression is in the same pool as the
 *       source.
 *
 * Return value:
 *  true  - The source and destination were found in the pool.
 *  false - Either the source or the destination were not found in the pool.
 */
unsigned char poolcheckarrayoptim(MetaPoolTy *Pool, void *NodeSrc, void *NodeResult) {
  Splay *psplay = poolchecksplay(Pool);
  Splay *ref = splay_find_ptr(psplay, (unsigned long)NodeSrc);
  if (ref) {
    return refcheck(ref, NodeResult);
  } 
  return false;
}

/*
 * Function: poolcheckarray()
 *
 * Description:
 *  This function performs the same check as poolcheckarrayoptim(), but
 *  checks all the pools associated with a meta-pool.
 */
void poolcheckarray(MetaPoolTy **MP, void *NodeSrc, void *NodeResult) {
  if (!ready) return;
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
  poolcheckfail ("poolcheckarray failure: Result \n", NodeResult);
}

/*
 * Function: poolcheckiarray ()
 *
 * Description:
 *  This function performs the same check as poolcheckarray() but
 *  allows the check to succeed if the source node is not found.
 */
void poolcheckiarray(MetaPoolTy **MP, void *NodeSrc, void *NodeResult) {
  if (!ready) return;
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
    if (poolcheckoptim (Pool, NodeSrc))
      if (poolcheckoptim (Pool, NodeResult))
        return;
      else
        poolcheckfail ("poolcheckiarray failure: Result \n", NodeResult);
    MetaPool = MetaPool->next;
  }

  return;
}

/*
 * Function: poolcheck()
 *
 * Description:
 *  Verify whether a node is located within one of the pools associated with
 *  the MetaPool.
 */
void poolcheck(MetaPoolTy **MP, void *Node) {
  if (!ready) return;
  MetaPoolTy *MetaPool = *MP;
  if (!MetaPool) {
    poolcheckfail ("Empty meta pool? \n", Node);
  }
  /*
   * iteratively search through the list
   * Check if there are other efficient data structures.
  */
  poolcheckinfo ("Poolcheck: Called from ", __builtin_return_address(0));
  poolcheckinfo ("Checking metapool ", MetaPool);
  while (MetaPool) {
    void *Pool = MetaPool->Pool;
#if 1
    printpoolinfo (Pool);
#endif
    if (poolcheckoptim(Pool, Node))   return;
    MetaPool = MetaPool->next;
  }
  poolcheckfail ("poolcheck failure \n", Node);
}


void poolcheckAddSlab(PoolCheckSlab **PCSPtr, void *Slab) {
  PoolCheckSlab  *PCSPrev = *PCSPtr;
  PoolCheckSlab *PCS = *PCSPtr;
  if (!ready) return;
  if (PCS) {
    PCS = PCS->nextSlab;
  } else {
    if (*PCSPtr = (PoolCheckSlab *) poolcheckmalloc (sizeof(PoolCheckSlab)))
    {
      (*PCSPtr)->Slab = Slab;
      (*PCSPtr)->nextSlab = 0;
    }
    return;
  }
  while (PCS) {
    PCSPrev = PCS;
    PCS = PCS->nextSlab;
  }
  /* PCS is null; */
  if (PCSPrev->nextSlab = (PoolCheckSlab *) poolcheckmalloc (sizeof(PoolCheckSlab)))
  {
    PCSPrev->nextSlab->Slab = Slab;
    PCSPrev->nextSlab->nextSlab = 0;
  }
}


  void exactcheck(int a, int b) {
    if ((0 > a) || (a >= b)) {
      poolcheckfail ("exact check failed\n",  (a << 16) & (b));
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
    if (!ready) return;
    splay_insert_ptr(splay, (unsigned long)(allocaptr), NumBytes);
  }
