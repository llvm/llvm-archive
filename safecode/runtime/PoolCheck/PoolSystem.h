/*===- PoolSystem.h - Pool check runtime interface to the system ----------===*/
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

#ifndef POOLSYSTEM_RUNTIME_H
#define POOLSYSTEM_RUNTIME_H

#ifdef __cplusplus
extern "C" {
#endif
  /* Functions that need to be provided by the pool allocation run-time */
  void poolcheckfail (const char * msg, int);
  void poolcheckfatal (const char * msg, int);
  void poolcheckinfo (const char * msg, int);
  void * poolcheckmalloc (unsigned int size);
  void printpoolinfo (void *Pool);
#ifdef __cplusplus
}
#endif

#endif
