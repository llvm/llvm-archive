dnl Process this file with autoconf 2.5x to create a configure script.

dnl Initialize
AC_INIT([[[LLVM]]],[[[1.0]]],[llvmbugs@cs.uiuc.edu])

dnl Place all of the extra autoconf files into the config subdirectory
AC_CONFIG_AUX_DIR([autoconf])

dnl Configure Makefiles
AC_CONFIG_MAKEFILE(Makefile)
AC_CONFIG_MAKEFILE(lib/Makefile)
AC_CONFIG_MAKEFILE(lib/Snapshot/Makefile)
AC_CONFIG_MAKEFILE(tools/Makefile)
AC_CONFIG_MAKEFILE(tools/llvm-tv/Makefile)

dnl Verify that the source directory is valid
AC_CONFIG_SRCDIR(["Makefile.common.in"])

dnl Find the LLVM source and object directories, whose locations may
dnl have been specified by the user. By default, assume we've unpacked this
dnl project in projects/<project-name>, as is customary, and look in ../..
dnl for the main LLVM source and object trees.
AC_ARG_WITH(llvmsrc,AC_HELP_STRING([--with-llvmsrc],[Location of LLVM Source Code]),AC_SUBST(LLVM_SRC,[$withval]),AC_SUBST(LLVM_SRC,[`cd ${srcdir}/../..; pwd`]))
AC_ARG_WITH(llvmobj,AC_HELP_STRING([--with-llvmobj],[Location of LLVM Object Code]),AC_SUBST(LLVM_OBJ,[$withval]),AC_SUBST(LLVM_OBJ,[`cd ../..; pwd`]))

dnl Create the output files
AC_OUTPUT([Makefile.common Makefile.config])

AC_MSG_WARN([Remember to configure wxWindows manually; see README.txt])
