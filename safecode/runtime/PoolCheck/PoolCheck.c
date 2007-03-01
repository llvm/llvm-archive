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

/* This defines the value of an invalid pointer address */
static const unsigned char * invalidptr = 0x00000004;

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
  MetaPoolTy  *MetaPoolPrev = *MP;
  MetaPoolTy *MetaPool = *MP;
  if (!ready) return;
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
 *  1  - The pointer is within the pool.
 *  0 - The pointer is not within the pool.
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

  /*
   * Scan through the list of slabs belonging to the pool and determine
   * whether this node is in one of those slabs.
   */
  PoolCheckSlab * PCS = poolcheckslab(Pool);
  while (PCS) {
    if (PCS->Slab == PS) return 1;
    /* we can optimize by moving it to the front of the list */
    PCS = PCS->nextSlab;
  }

  /*
   * Here we check for the splay tree
   */
  psplay = poolchecksplay(Pool);
  ref = splay_find_ptr(psplay, (unsigned long) Node);
  if (ref) {
    return 1;
  }
  return 0;
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
 *  1 - The source and destination were found in the pool.
 *  0 - The source was not found in the pool
 * -1 - The source was found in the pool, but the result is outside of the pool.
 */
char
poolcheckarrayoptim(MetaPoolTy *Pool, void *NodeSrc, void *NodeResult) {
  Splay *psplay = poolchecksplay(Pool);
  Splay *ref = splay_find_ptr(psplay, (unsigned long)NodeSrc);
  if (ref) {
    return refcheck(ref, NodeResult);
  } 
  return 0;
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

inline int refcheck(Splay *splay, void *Node) {
  unsigned long base = (unsigned long) (splay->key);
  unsigned long length = (unsigned long) (splay->val);
  unsigned long result = (unsigned long) Node;
  if ((result >= base) && (result < (base + length))) return 1;
  return -1;
                                                        
}

inline signed char * getactualvalue(signed char *Pool, signed char *val) {
  if (!ready) return val;
  if ((unsigned long) val != invalidptr) {
    return val;
  } else {
    poolcheckfail("Bounds check failure : value in the first page \n", val);
  }
}

inline  void exactcheck2(signed char *base, signed char *result, unsigned size) {
  if (result >= base + size ) {
    poolcheckfail("Array bounds violation detected \n", base);
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
void *
boundscheck(MetaPoolTy **MP, void *NodeSrc, void *NodeResult) {
  MetaPoolTy *MetaPool = *MP;
  void *Pool;
  if (!ready) return NodeResult;
  if (!MetaPool) {
    poolcheckfail ("Empty meta pool? \n", 0);
  }
  //iteratively search through the list
  //Check if there are other efficient data structures.
  while (MetaPool) {
    Pool = MetaPool->Pool;
    int ret = poolcheckarrayoptim(Pool, NodeSrc, NodeResult);
    if (ret) {
      if (ret == -1) return invalidptr;
      return NodeResult;
    }
    MetaPool = MetaPool->next;
  }
  poolcheckfail ("boundscheck failure 1\n", NodeSrc);
  return NodeResult;
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
void *
uiboundscheck (MetaPoolTy **MP, void *NodeSrc, void *NodeResult) {
  MetaPoolTy *MetaPool = *MP;
  void *Pool;

  /* Don't do a check if the kernel is not ready */
  if (!ready) return NodeResult;

  /* It is an error if there is no MetaPool */
  if (!MetaPool) {
    poolcheckfail ("Empty meta pool? \n", 0);
  }

  /*
   * iteratively search through the list
   * Check if there are other efficient data structures.
   */
  while (MetaPool) {
    Pool = MetaPool->Pool;
    int ret = poolcheckarrayoptim(Pool, NodeSrc, NodeResult);
    switch (ret) {
      case -1:
        return invalidptr;
        break;
      case 1:
        return NodeResult;
        break;
      case 0:
        break;
    }
    MetaPool = MetaPool->next;
  }

  return NodeResult;
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

//
// Function: poolcheckregister()
//
// Description:
//  This function registers a range of memory as belonging to the splay.  It is
//  currently used to register arrays, stack nodes, and global nodes.
//
void poolcheckregister(Splay *splay, void * allocaptr, unsigned NumBytes) {
  if (!ready) return;
  splay_insert_ptr(splay, (unsigned long)(allocaptr), NumBytes);
}
