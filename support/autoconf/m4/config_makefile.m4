#
# Configure a Makefile without clobbering it if it exists and is not out of
# date.  This macro is unique to LLVM.
#
AC_DEFUN([AC_CONFIG_MAKEFILE],
[AC_CONFIG_COMMANDS($1,
  [${LLVM_TOP}/support/autoconf/mkinstalldirs `dirname $1`
   ${SHELL} ${LLVM_TOP}/support/autoconf/install-sh -c ${srcdir}/$1 $1])
])
