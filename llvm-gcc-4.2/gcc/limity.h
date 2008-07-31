/* APPLE LOCAL begin 4401222 */

/* This administrivia gets added to the end of limits.h
   if the system has its own version of limits.h.  */

#else /* not _GCC_LIMITS_H_ */

/* LLVM LOCAL */
#ifndef CONFIG_DARWIN_H

#ifdef _GCC_NEXT_LIMITS_H
#include_next <limits.h>/* recurse down to the real one */
#endif

/* LLVM LOCAL */
#endif /* not CONFIG_DARWIN_H */

#endif /* not _GCC_LIMITS_H_ */
/* APPLE LOCAL end 4401222 */
