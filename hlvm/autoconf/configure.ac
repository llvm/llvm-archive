dnl **************************************************************************
dnl * Initialize
dnl **************************************************************************
AC_INIT([[[HLVM]]],[[[0.1svn]]],[hlvm-dev@hlvm.org])
AC_SUBST(HLVM_COPYRIGHT,["Copyright (c) 2006 HLVM Programmer's Group"])
AC_COPYRIGHT[Copyright (c) 2006 HLVM Programmer's Group])
AC_PREREQ(2.59)
AC_CONFIG_SRCDIR(["lib/Makefile"])
AC_CONFIG_FILES(Makefile.common)

dnl Configure project makefiles
dnl List every Makefile that exists within your source tree
AC_CONFIG_MAKEFILE(Makefile)
AC_CONFIG_MAKEFILE(lib/Makefile)
AC_CONFIG_MAKEFILE(tools/Makefile)

dnl **************************************************************************
dnl * Set the location of various third-party software packages
dnl **************************************************************************
FIND_PACKAGE([llvm],[llvm/Module.h],[LLVMCore.o],[llc],
             [Low Level Virtual Machine])
FIND_PACKAGE([apr],[apr-1/apr.h],[libapr-1.so],[],[Apache Portable Runtime])

dnl **************************************************************************
dnl * Determine which system we are building on
dnl **************************************************************************

dnl **************************************************************************
dnl * Check for programs.
dnl **************************************************************************

dnl **************************************************************************
dnl * Check for libraries.
dnl **************************************************************************

dnl **************************************************************************
dnl * Checks for header files.
dnl **************************************************************************

dnl **************************************************************************
dnl * Checks for typedefs, structures, and compiler characteristics.
dnl **************************************************************************

dnl **************************************************************************
dnl * Checks for library functions.
dnl **************************************************************************

dnl **************************************************************************
dnl * Enable various compile-time options
dnl **************************************************************************

dnl **************************************************************************
dnl * Create the output files
dnl **************************************************************************

dnl This must be last
AC_OUTPUT
