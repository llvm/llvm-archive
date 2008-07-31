/* This administrivia gets added to the beginning of limits.h
   if the system has its own version of limits.h.  */

/* APPLE LOCAL begin 4401222 */
/* LLVM LOCAL */
#ifdef CONFIG_DARWIN_H

#ifndef _LIBC_LIMITS_H_
/* Use "..." so that we find syslimits.h only in this same directory.  */
#include "syslimits.h"
#endif
#ifdef _GCC_NEXT_LIMITS_H
#include_next <limits.h>
#undef _GCC_NEXT_LIMITS_H
#endif

/* LLVM LOCAL begin */
#endif /* not CONFIG_DARWIN_H */

/* We use _GCC_LIMITS_H_ because we want this not to match
   any macros that the system's limits.h uses for its own purposes.  */
/* LLVM LOCAL */
#if !defined(_GCC_LIMITS_H_) && !defined(CONFIG_DARWIN_H)  /* Terminated in limity.h.  */
#define _GCC_LIMITS_H_

#ifndef _LIBC_LIMITS_H_
/* Use "..." so that we find syslimits.h only in this same directory.  */
#include "syslimits.h"
#endif
#ifdef _GCC_NEXT_LIMITS_H
#include_next <limits.h>
#undef _GCC_NEXT_LIMITS_H
#endif
/* LLVM LOCAL end */
/* APPLE LOCAL end 4401222 */
