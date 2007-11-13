
/* LLVM LOCAL begin */
#ifdef ENABLE_LLVM

/* LLVM specific stuff for supporting dllimport & dllexport linkage output */

extern bool i386_pe_dllimport_p(tree);
extern bool i386_pe_dllexport_p(tree);

#define TARGET_ADJUST_LLVM_LINKAGE(GV, decl)            \
  {                                                     \
    if (i386_pe_dllimport_p((decl))) {                  \
      (GV)->setLinkage(GlobalValue::DLLImportLinkage);  \
    } else if (i386_pe_dllexport_p((decl))) {           \
      (GV)->setLinkage(GlobalValue::DLLExportLinkage);  \
    }                                                   \
  }

/* Add general target specific stuff */
#include "llvm-i386-target.h"

/* LLVM LOCAL end */

#endif
                                                          
