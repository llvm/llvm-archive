#
# Provide the arguments and other processing needed for an LLVM module
#
# This script must be used after AC_INIT. It checks various axioms, sets up 
# various LLVM_MODULE variables, extracts the module's dependencies, validates 
# the directory is correct, etc.
AC_DEFUN([THIS_IS_LLVM_MODULE],[

dnl Get the llvm-top directory which is just any directory that contains all 
dnl the modules and doesn't have to be an llvm-top checked out thing. This is
dnl important because its how we find all the modules. To support building a
dnl module in an obtuse place (other than llvm-top) we provide the 
dnl with-llvm-top=/path/to/llvm/top option to allow the configure script to
dnl be told where it is. However, the default ".." usually just works.
  LLVM_TOP=`cd .. ; pwd`
  AC_ARG_WITH(llvm-top,
  AS_HELP_STRING([--with-llvm-top],
                 [Specify where the llvm-top directory is]),,withval=default)
  case "$withval" in
    default)  ;; dnl nothing to do, LLVM_TOP already defaulted
    *) LLVM_TOP=$withval ;;
  esac
  if test ! -d "$LLVM_TOP" ; then
    AC_MSG_ERROR("The LLVM_TOP directory ($LLVM_TOP) is not a directory!")
  fi
  AC_SUBST(LLVM_TOP,[$LLVM_TOP])

dnl Make sure we can find the support/autoconf dir .. everything depends on it!
  if test ! -d "$LLVM_TOP/support/autoconf" ; then
    AC_MSG_ERROR("Your llvm-top directory needs to have the support module checked out")
  fi

dnl We have a standardized auxilliary directory in LLVM so that we only need
dnl one copy of the various tools found there instead of copying them to all
dnl the modules as well. The support/autoconf directory is specified here as
dnl the auxilliary directory which must contain install-sh at least.
  AC_CONFIG_AUX_DIR($LLVM_TOP/support/autoconf)

dnl This stuff is based on autoconf 2.60, enforce it.
  AC_PREREQ(2.60)

dnl Get the copyright in a substitution variable
  AC_SUBST(LLVM_COPYRIGHT,["Copyright (c) 2003-2007 University of Illinois at Urbana-Champaign."])

dnl Declare the copyright to autoconf
  AC_COPYRIGHT([Copyright (c) 2003-2007 University of Illinois at Urbana-Champaign.])

dnl Set up the LLVM_MODULE_NAME variables. The NAME comes from the argument to 
dnl this macro and should be the name of the module's directory. We verify that.
dnl This is necessary because we expect modules to be in a directory that is
dnl the official name of the modules. If they are not the construction of paths
dnl can go wrong.
  LLVM_MODULE_NAME="$1"
  cwd=`pwd`
  if test `basename $cwd` != "$1" ; then
    AC_MSG_ERROR([Module $1 is checked out to $cwd which is not allowed])
  fi
  AC_SUBST(LLVM_MODULE_NAME,"$1")

dnl Set up other LLVM_MODULE_* variables that are copies of the values provided
dnl to the AC_INIT macro. 
  LLVM_MODULE_FULLNAME="AC_PACKAGE_NAME"
  LLVM_MODULE_TARNAME="AC_PACKAGE_TARNAME"
  LLVM_MODULE_VERSION="AC_PACKAGE_VERSION"
  LLVM_MODULE_BUGREPORT="AC_PACKAGE_BUGREPORT"
  AC_SUBST(LLVM_MODULE_FULLNAME,"AC_PACKAGE_NAME")
  AC_SUBST(LLVM_MODULE_TARNAME,"AC_PACKAGE_TARNAME")
  AC_SUBST(LLVM_MODULE_VERSION,"AC_PACKAGE_VERSION")
  AC_SUBST(LLVM_MODULE_BUGREPORT,"AC_PACKAGE_BUGREPORT")

dnl We need to know which modules depend on this one. Fortunately, every module
dnl is required to document that in the ModuleInfo.txt file so we just extract
dnl it.
  LLVM_MODULE_DEPENDS_ON=`grep -i "DepModule:" ModuleInfo.txt | \
    sed -e 's/DepModule: *//g'`
  for x in $LLVM_MODULE_DEPENDS_ON ; do
    AC_MSG_NOTICE([$1 depends on $x])
    dnl we should probably validate the module names, but that would require
    dnl several round trips via subversion .. slowness.
  done
  AC_SUBST(LLVM_MODULE_DEPENDS_ON,"$LLVM_MODULE_DEPENDS_ON")
  AC_CONFIG_SRCDIR([$2])

dnl The module can have a Makefile.config.in or a Makefile.common.in. In fact, 
dnl it must have one of the two. Set these up to be configured and check thatw
dnl we have at least one.
  has_configured_makefile=0
  if test -r Makefile.config.in ; then
    AC_CONFIG_FILES([Makefile.config])
    has_configured_makefile=1
  fi
  if test -r Makefile.common.in ; then
    AC_CONFIG_FILES([Makefile.common])
    has_configured_makefile=1
  fi
  if test "$has_configured_makefile" -eq 0 ; then
    AC_MSG_ERROR("Your module is lacking a Makefile.common.in or Makefile.config.in file to hold configured variables")
  fi

dnl This is a quirky thing. This is basically getting the LLVM_TOP variable 
dnl into the start of the config.status script which otherwise contains nothing
dnl from the configure script except things like srcdir and prefix. We need
dnl this for the CONFIG_LLVM_MAKEFILE which has to find the mkinstalldirs 
dnl program in the support module's autoconf directory.
  AC_CONFIG_COMMANDS([setup],,[LLVM_TOP="${LLVM_TOP}"])

dnl Get the approximate time of configuration so it can be substituted
  LLVM_CONFIGTIME=`date`
  AC_SUBST(LLVM_CONFIGTIME)
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
