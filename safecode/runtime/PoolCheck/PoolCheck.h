/*===- PoolCheck.h - Pool check runtime interface file --------------------===*/
/*                                                                            */
/*                       The LLVM Compiler Infrastructure                     */
/*                                                                            */
/* This file was developed by the LLVM research group and is distributed      */
/* under the University of Illinois Open Source License. See LICENSE.TXT for  */
/* details.                                                                   */
/*                                                                            */
/*===----------------------------------------------------------------------===*/
/*                                                                            */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

#ifndef POOLCHECK_RUNTIME_H
#define POOLCHECK_RUNTIME_H

#include "PoolSystem.h"
#include "splay.h"

static const unsigned char true = 1;
static const unsigned char false = 0;

typedef struct PoolCheckSlab {
  void *Slab;
  struct PoolCheckSlab *nextSlab;
} PoolCheckSlab;

typedef struct MetaPoolTy {
  void * Pool;
  struct MetaPoolTy *next;
} MetaPoolTy;


#ifdef __cpluscplus
extern "C" {
#endif
  /* register that starting from allocaptr numbytes are a part of the pool */
  void poolcheck(MetaPoolTy **Pool, void *Node);
  unsigned char poolcheckoptim(void *Pool, void *Node);
  void poolcheckregister(Splay *splay, void * allocaptr, unsigned NumBytes);
  void AddPoolDescToMetaPool(MetaPoolTy **MetaPool, void *PoolDesc);
  void poolcheckarray(MetaPoolTy **Pool, void *Node, void * Node1);
  void poolcheckiarray(MetaPoolTy **Pool, void *Node, void * Node1);
  unsigned char  poolcheckarrayoptim(MetaPoolTy *Pool, void *Node, void * Node1);
  void poolcheckAddSlab(PoolCheckSlab **PoolCheckSlabPtr, void *Slab);
  void poolcheckinit(void *Pool, unsigned NodeSize);
  void poolcheckdestroy(void *Pool);
  void poolcheckfree(void *Pool, void *Node);

  /* Functions that need to be provided by the pool allocation run-time */
  PoolCheckSlab *poolcheckslab(void *Pool);
  Splay *poolchecksplay(void *Pool);
  unsigned poolcheckslabsize (void * Pool);
#ifdef __cpluscplus
}
#endif

#endif
