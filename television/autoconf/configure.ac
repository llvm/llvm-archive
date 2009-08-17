dnl Process this file with autoconf 2.5x to create a configure script.

dnl Initialize
AC_INIT([[llvm-tv]],[[pre-release]],[llvmbugs@cs.uiuc.edu])

dnl Place all of the extra autoconf files into the config subdirectory
AC_CONFIG_AUX_DIR([autoconf])

dnl Configure the makefile's configuration data
AC_CONFIG_FILES([Makefile.config])

dnl Configure Makefiles
AC_CONFIG_MAKEFILE(Makefile)
AC_CONFIG_MAKEFILE(Makefile.common)
AC_CONFIG_MAKEFILE(lib/Makefile)
AC_CONFIG_MAKEFILE(lib/Snapshot/Makefile)
AC_CONFIG_MAKEFILE(tools/Makefile)
AC_CONFIG_MAKEFILE(tools/llvm-tv/Makefile)

dnl Find the LLVM source and object directories, whose locations may
dnl have been specified by the user. By default, assume we've unpacked this
dnl project in projects/<project-name>, as is customary, and look in ../..
dnl for the main LLVM source and object trees.
AC_ARG_WITH(llvm-src,
            AS_HELP_STRING([--with-llvm-src],
                           [Location of LLVM Source Code]),
            AC_SUBST(LLVM_SRC, [$withval]),
            AC_SUBST(LLVM_SRC, [`cd ${srcdir}/../..; pwd`]))
AC_ARG_WITH(llvm-obj,
            AS_HELP_STRING([--with-llvm-obj],
                           [Location of LLVM Object Code]),
            AC_SUBST(LLVM_OBJ, [$withval]),
            AC_SUBST(LLVM_OBJ, [`cd ../..; pwd`]))

dnl We need to get the DataStructure Analysis headers and libraries from
dnl poolalloc, since that is where they live now.
AC_ARG_WITH(poolalloc-src,
            AS_HELP_STRING([--with-poolalloc-src],
                           [Location of PoolAlloc Source Code]),
            AC_SUBST(POOLALLOC_SRC, [$withval]),
            AC_SUBST(POOLALLOC_SRC, [`cd ${srcdir}/../..; pwd`]))
AC_ARG_WITH(poolalloc-obj,
            AS_HELP_STRING([--with-poolalloc-obj],
                           [Location of PoolAlloc Object Code]),
            AC_SUBST(POOLALLOC_OBJ, [$withval]),
            AC_SUBST(POOLALLOC_OBJ, [`cd ../..; pwd`]))

dnl Create the output files
AC_OUTPUT
