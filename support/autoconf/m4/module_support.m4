dnl
dnl Provide the arguments and other processing needed for an LLVM module
dnl
dnl This script must be used after AC_INIT. It sets up various LLVM_MODULE
dnl variables, extracts the module's dependencies, validates the directory
dnl is correct, etc.
AC_DEFUN([THIS_IS_LLVM_MODULE],[
  LLVM_MODULE_NAME="$1"
  LLVM_MODULE_FULLNAME="AC_PACKAGE_NAME"
  LLVM_MODULE_TARNAME="AC_PACKAGE_TARNAME"
  LLVM_MODULE_VERSION="AC_PACKAGE_VERSION"
  LLVM_MODULE_BUGREPORT="AC_PACKAGE_BUGREPORT"
  LLVM_TOP=`cd .. ; pwd`
  LLVM_MODULE_DEPENDS_ON=`grep DepModules: ModuleInfo.txt | \
    sed 's/DepModules: *//'`
  AC_CONFIG_SRCDIR([$2])
  if test -r Makefile.config.in ; then
    AC_CONFIG_FILES([Makefile.config])
  fi
  if test -r Makefile.common.in ; then
    AC_CONFIG_FILES([Makefile.common])
  fi
  AC_SUBST(LLVM_TOP,[$LLVM_TOP])
  AC_SUBST(LLVM_MODULE_NAME,"$1")
  AC_SUBST(LLVM_MODULE_FULLNAME,"AC_PACKAGE_NAME")
  AC_SUBST(LLVM_MODULE_TARNAME,"AC_PACKAGE_TARNAME")
  AC_SUBST(LLVM_MODULE_VERSION,"AC_PACKAGE_VERSION")
  AC_SUBST(LLVM_MODULE_BUGREPORT,"AC_PACKAGE_BUGREPORT")
  AC_SUBST(LLVM_MODULE_DEPENDS_ON,"$LLVM_MODULE_DEPENDS_ON")
  AC_ARG_WITH([llvmsrc],
    AS_HELP_STRING([--with-llvmsrc],[Location of LLVM Source Code]),
    [llvm_src="$withval"],[llvm_src="]$1["])
  AC_SUBST(LLVM_SRC,$llvm_src)
  AC_ARG_WITH([llvmobj],
    AS_HELP_STRING([--with-llvmobj],[Location of LLVM Object Code]),
    [llvm_obj="$withval"],[llvm_obj="]$2["])
  AC_SUBST(LLVM_OBJ,$llvm_obj)
  AC_CONFIG_COMMANDS([setup],,[LLVM_TOP="${LLVM_TOP}"])
])

dnl
dnl Configure a Makefile without clobbering it if it exists and is not out of
dnl date.  This macro is unique to LLVM.
dnl
AC_DEFUN([CONFIG_LLVM_MAKEFILE],
  [AC_CONFIG_COMMANDS($1,
    [${LLVM_TOP}/support/autoconf/mkinstalldirs `dirname $1`
     ${SHELL} ${LLVM_TOP}/support/autoconf/install-sh -c ${srcdir}/$1 $1])
])
