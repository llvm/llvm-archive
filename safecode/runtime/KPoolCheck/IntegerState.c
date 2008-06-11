/*===- IntegerState.cpp - Implementation of Integer State Swapping --------===*/
/*                                                                            */
/*                  The Secure Virtual Architecture Project                   */
/*                                                                            */
/* This file was developed by the LLVM research group and is distributed      */
/* under the University of Illinois Open Source License. See LICENSE.TXT for  */
/* details.                                                                   */
/*                                                                            */
/*===----------------------------------------------------------------------===*/
/*                                                                            */
/* This file implements the SVA state manipulation instructions with memory   */
/* safety checks.                                                             */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

#include "PoolCheck.h"
#include "PoolSystem.h"
#include "adl_splay.h"
#define DEBUG(x) 

/* Global splay for holding saved integer state */
static void * IntegerStates;

/* Internal implementations of llva_load_integer and llva_save_integer */
extern void         llva_load_integer (void * p) __attribute__ ((regparm(0)));
extern unsigned int llva_save_integer (void * p) __attribute__ ((regparm(0)));

/*
 * Intrinsic: llva_swap_integer()
 *
 * Description:
 *  This intrinsic saves the current integer state and swaps in a new one.
 *
 * Inputs:
 *  old - The memory used to hold the integer state currently on the processor.
 *  new - The new integer state to load on to the processor.
 *
 * Return value:
 *  An identifier that can be used to reload the integer state on to the
 *  processor.
 */
void *
sva_swap_integer (void * old, void * new) {
#if 0
  unsigned int state[24];
  void * old = &state;
  void * new = &state;
#endif

  /*
   * Determine whether the integer state is valid.
   */
  if ((pchk_check_int (old)) == 0){
    poolcheckfail ("sva_swap_integer: Bad integer state:", (unsigned)old, (void*)__builtin_return_address(0));
  }

  /*
   * Save the current integer state.
   */
  if (llva_save_integer (old)) {
    /*
     * We've awakened.  Mark the integer state invalid and return to the
     * caller.
     */
    pchk_drop_int (old);
    return new;
  }

  /*
   * Register the saved integer state in the splay tree.
   */
  pchk_reg_int (old);

  /*
   * Now, reload the integer state pointed to by new.
   */
  llva_load_integer (new);

  /*
   * The loading of integer state failed.
   */
  pchk_drop_int (old);
  return (void *)(0);
}

