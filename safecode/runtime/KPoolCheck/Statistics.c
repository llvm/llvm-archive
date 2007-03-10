/*===- Stats.cpp - Statistics held by the runtime -------------------------===*/
/*                                                                            */
/*                       The LLVM Compiler Infrastructure                     */
/*                                                                            */
/* This file was developed by the LLVM research group and is distributed      */
/* under the University of Illinois Open Source License. See LICENSE.TXT for  */
/* details.                                                                   */
/*                                                                            */
/*===----------------------------------------------------------------------===*/
/*                                                                            */
/* This file implements functions that can be used to hold statistics         */
/* information                                                                */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

/* The number of stack to heap promotions executed dynamically */
static int stack_promotes = 0;

void
stackpromote()
{
  ++stack_promotes;
  return;
}

int
getstackpromotes()
{
  return stack_promotes;
}

