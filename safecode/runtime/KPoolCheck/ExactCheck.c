/*===- ExactCheck.cpp - Implementation of exactcheck functions ------------===*/
/*                                                                            */
/*                       The LLVM Compiler Infrastructure                     */
/*                                                                            */
/* This file was developed by the LLVM research group and is distributed      */
/* under the University of Illinois Open Source License. See LICENSE.TXT for  */
/* details.                                                                   */
/*                                                                            */
/*===----------------------------------------------------------------------===*/
/*                                                                            */
/* This file implements the exactcheck family of functions.                   */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

#include "PoolCheck.h"
#include "PoolSystem.h"
#include "adl_splay.h"
#ifdef LLVA_KERNEL
#include <stdarg.h>
#endif
#define DEBUG(x) 

/* Flag whether to print error messages on bounds violations */
static const int do_fail = 1;

extern int stat_exactcheck;
extern int stat_exactcheck2;
extern int stat_exactcheck3;

void * exactcheck(int a, int b, void * result) {
  ++stat_exactcheck;
  if ((0 > a) || (a >= b)) {
    if(do_fail) poolcheckfail ("exact check failed", (a), (void*)__builtin_return_address(0));
    if(do_fail) poolcheckfail ("exact check failed", (b), (void*)__builtin_return_address(0));
  }
  return result;
}

void * exactcheck2(signed char *base, signed char *result, unsigned size) {
  ++stat_exactcheck2;
  if ((result < base) || (result >= base + size )) {
    if(do_fail) poolcheckfail("Array bounds violation detected ", (unsigned)base, (void*)__builtin_return_address(0));
  }
  return result;
}

void * exactcheck2a(signed char *base, signed char *result, unsigned size) {
  ++stat_exactcheck2;
  if (result >= base + size ) {
    if(do_fail) poolcheckfail("Array bounds violation detected ", (unsigned)base, (void*)__builtin_return_address(0));
  }
  return result;
}

void * exactcheck3(signed char *base, signed char *result, signed char * end) {
  ++stat_exactcheck3;
  if ((result < base) || (result > end )) {
    if(do_fail) poolcheckfail("Array bounds violation detected ", (unsigned)base, (void*)__builtin_return_address(0));
  }
  return result;
}

void funccheck (unsigned num, void *f, void *t1, void *t2,
                                       void *t3, void *t4) {
  if ((t1) && (f == t1)) return;
  if ((t2) && (f == t2)) return;
  if ((t3) && (f == t3)) return;
  if ((t4) && (f == t4)) return;
  if (do_fail) poolcheckfail ("funccheck failed", f, (void*)__builtin_return_address(0));
  return;
}

void funccheck_g (unsigned num, void * f, void ** table) {
  unsigned int index;
  for (index = 0; index < num; ++index) {
    if (f == table[index])
      return;
  }
  if (do_fail) poolcheckfail ("funccheck_g failed", f, (void*)__builtin_return_address(0));
}

struct node {
  void* left;
  void* right;
  char* key;
  char* end;
  void* tag;
};

void * getBegin (void * node) {
  return ((struct node *)(node))->key;
}

void * getEnd (void * node) {
  return ((struct node *)(node))->end;
}

