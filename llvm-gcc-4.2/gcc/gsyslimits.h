/* syslimits.h stands for the system's own limits.h file.
   If we can use it ok unmodified, then we install this text.
   If fixincludes fixes it, then the fixed version is installed
   instead of this text.  */

#define _GCC_NEXT_LIMITS_H		/* tell gcc's limits.h to recurse */
/* APPLE LOCAL begin 4401222 */
/* LLVM LOCAL */
#ifndef CONFIG_DARWIN_H
#include_next <limits.h>
#undef _GCC_NEXT_LIMITS_H
/* LLVM LOCAL */
#endif /* not CONFIG_DARWIN_H */
/* APPLE LOCAL end 4401222 */
