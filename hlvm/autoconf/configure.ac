dnl **************************************************************************
dnl * Initialize
dnl **************************************************************************
AC_INIT([High Level Virtual Machine],[0.1svn],[hlvm-dev@hlvm.org],HLVM)
AC_PREREQ(2.59)
HLVM_COPYRIGHT="Coyright (c) 2006 HLVM Group"
AC_SUBST(HLVM_COPYRIGHT)
AC_COPYRIGHT($HLVM_COPYRIGHT)
AC_CONFIG_SRCDIR([hlvm/AST/Bundle.h])
AC_REVISION([$Id$])

HLVM_VERSION="${PACKAGE_VERSION}"
AC_SUBST(HLVM_PACKAGE,$PACKAGE)
AC_SUBST(HLVM_VERSION)
AC_PREFIX_DEFAULT([/usr/local/hlvm])


dnl ----------------------------------------------------------------------------
dnl -- Shared Object Versioning For libtool
dnl -- 
dnl -- The following substitution, HLVM_SO_VERSION defines the version number 
dnl -- for the libHLVM.so shared object. This version is not the same as the 
dnl -- HLVM release version! It is dependent on the *ACTUAL* interface (not 
dnl -- implementation) changes in the shared object. The value of 
dnl -- HLVM_SO_VERSION is passed to libtool as the --version-info argument. 
dnl -- Here are the rules for updating these numbers (adapted from libtool 
dnl -- documentation)
dnl -- 
dnl -- 0. CURRENT(C)= current interface version number (of the release)
dnl --    REVISION(R)= revision of the *impelmentation* of the current interface
dnl --    AGE(A)= number of prior interface version numbers also supported
dnl --
dnl -- 1. Start with version information of `0:0:0'.
dnl --
dnl -- 2. Update the version information *only* immediately before a public
dnl --    release of your software.  More frequent updates are unnecessary,
dnl --    and only say that the current interface number gets larger faster.
dnl --
dnl -- 3. If the library source code has changed at all since the last 
dnl --    update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
dnl --
dnl -- 4. If any interfaces have been added, removed, or changed since the
dnl --     last update, increment CURRENT, and set REVISION to 0.
dnl -- 
dnl -- 5. If any interfaces have been added (only!) since the last public 
dnl --    release, then increment AGE.
dnl --
dnl -- 6. If any interfaces have been removed since the last public 
dnl --    release, then set AGE to 0.
dnl ----------------------------------------------------------------------------
HLVM_SO_CURRENT="0"
HLVM_SO_REVISION="10"
HLVM_SO_AGE="0"
HLVM_SO_VERSION="${HLVM_SO_CURRENT}:${HLVM_SO_REVISION}:${HLVM_SO_AGE}"
AC_SUBST(HLVM_SO_VERSION)
AC_SUBST(HLVM_SO_CURRENT)
AC_SUBST(HLVM_SO_REVISION)
AC_SUBST(HLVM_SO_AGE)

dnl ----------------------------------------------------------------------------
dnl --
dnl -- SECTION 2: Setup The Command Line Arguments For "configure"
dnl --
dnl ----------------------------------------------------------------------------
dnl --
dnl -- enable-sharedprefix: specifies that the installation prefix is shared 
dnl -- with other packages and that per-configuration directories should not 
dnl -- be created
AC_MSG_CHECKING(whether to install in a shared prefix)
AC_ARG_ENABLE(sharedprefix, 
  AS_HELP_STRING([--enable-sharedprefix],
    [Install in a "--prefix" that is shared. (default=yes)]),
  [case "${enableval}" in
    yes) HLVM_SHAREDPREFIX=true;;
    no)  HLVM_SHAREDPREFIX=false;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-sharedprefix);;
  esac],HLVM_SHAREDPREFIX=true)
AC_MSG_RESULT($HLVM_SHAREDPREFIX)
AC_SUBST(HLVM_SHAREDPREFIX)
if test "$HLVM_SHAREDPREFIX" = true ; then
  AC_DEFINE(HLVM_SHAREDPREFIX,[],
            [Defined if all configurations share the same prefix])
fi

dnl -- enable-debug: specifies that the build be done with debug settings 
dnl -- which makes HLVM (potentially) much more verbose, larger, and slower 
dnl -- (but debuggable)
AC_MSG_CHECKING(whether to build debug version of HLVM)
AC_ARG_ENABLE(debug, 
  AS_HELP_STRING([--enable-debug],
    [Build with debug settings (slower but debuggable). (default=no)]),
  [case "${enableval}" in
   yes) HLVM_DEBUG=true;;
   no)  HLVM_DEBUG=false;;
   *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug);;
  esac],HLVM_DEBUG=false)
AC_MSG_RESULT($HLVM_DEBUG)
AC_SUBST(HLVM_DEBUG)

dnl -- enable-optimize: specifies that the build be done top optimize the speed
dnl -- of the resulting executables and libraries. Debugging generally will be
dnl -- difficult with this kind of build.
AC_MSG_CHECKING(whether to optimize compiliation of HLVM)
AC_ARG_ENABLE(optimize, 
  AS_HELP_STRING([--enable-optimize],
    [Optimized build (faster but less debuggable). (default=yes)]),
  [case "${enableval}" in
    yes) HLVM_OPTIMIZE=true;;
    no)  HLVM_OPTIMIZE=false;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-OPTIMIZE);;
  esac], HLVM_OPTIMIZE=true)
AC_MSG_RESULT($HLVM_OPTIMIZE)
AC_SUBST(HLVM_OPTIMIZE)

dnl -- enable-inline: specifies that the build use inline functions or not
dnl -- inline functions make HLVM much faster but harder to debug
AC_MSG_CHECKING(whether to use inline functions)
AC_ARG_ENABLE(INLINE, 
  AS_HELP_STRING([--enable-inline],
    [Build with inline functions. (default=yes)]),
  [case "${enableval}" in
    yes) HLVM_INLINE=true;;
    no)  HLVM_INLINE=false;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-inline);;
  esac],HLVM_INLINE=true)
AC_MSG_RESULT($HLVM_INLINE)
AC_SUBST(HLVM_INLINE)

dnl -- enable-assert: specifies that the build use assertion checks
dnl -- Building with assertions is safer but slower
AC_MSG_CHECKING(whether to perform assertion checking)
AC_ARG_ENABLE(ASSERT, 
  AS_HELP_STRING([--enable-assert],
    [Build with inline functions. (default=yes)]),
  [case "${enableval}" in
    yes) HLVM_ASSERT=true;;
    no)  HLVM_ASSERT=false;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-assert);;
  esac],HLVM_ASSERT=true)
AC_MSG_RESULT($HLVM_ASSERT)
AC_SUBST(HLVM_ASSERT)

dnl -- enable-trace: specifies that the build permit runtime tracing
dnl -- This is useful for debugging but will produce MUCH larger executable
AC_MSG_CHECKING(whether to allow runtime tracing)
AC_ARG_ENABLE(TRACE, 
  AS_HELP_STRING([--enable-trace],[Build with tracing support. (default=no)]),
  [case "${enableval}" in
    yes) HLVM_TRACE=true;;
    no)  HLVM_TRACE=false;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-trace);;
  esac],HLVM_TRACE=false)
AC_MSG_RESULT($HLVM_TRACE)
AC_SUBST(HLVM_TRACE)

dnl -- enable-small: specifies that the build should be done to make HLVM 
dnl -- as small as possible, omitting non-essential features to reduce memory
dnl -- footprint.
AC_MSG_CHECKING(whether to build a small version of HLVM)
AC_ARG_ENABLE(SMALL,
  AS_HELP_STRING([--enable-small],
    [Optimize for smaller executable rather than speed. (default=no)]),
  [case "${enableval}" in
    yes) HLVM_SMALL=true;;
    no)  HLVM_SMALL=false;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-small);;
  esac],HLVM_SMALL=false)
AC_MSG_RESULT($HLVM_SMALL)
AC_SUBST(HLVM_SMALL)

dnl -- enable-efence: specifies that Electric Fence should be used for malloc(3)
dnl -- and related functions to catch memory overrun/underrun errors. When not 
dnl -- enabled, the standard system malloc(3) is used.
AC_MSG_CHECKING(whether to use Electric Fence)
AC_ARG_ENABLE(efence,
  AS_HELP_STRING([--enable-efence],
    [Build with Electric Fence support. (default=no)]),
  [case "${enableval}" in
    yes) HLVM_EFENCE=true;;
    no)  HLVM_EFENCE=false;;
    *)   AC_MSG_ERROR(bad value ${enableval} for --enable-efence);;
  esac],HLVM_EFENCE=false)
AC_MSG_RESULT($HLVM_EFENCE)
AC_SUBST(HLVM_EFENCE)

dnl -- enable-profiling: specifies that the build should create a profiling 
dnl -- executable. The compiler will be passed the -pg switch so that call 
dnl -- graph profiles are generated
AC_MSG_CHECKING(whether to build with profiling enabled)
AC_ARG_ENABLE(profiling, 
  AS_HELP_STRING([--enable-profiling],
    [Build with profiling enabled. (default=no)]),
  [case "${enableval}" in
    yes) HLVM_PROFILING=true;;
    no)  HLVM_PROFILING=false;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-profiling);;
  esac],HLVM_PROFILING=false)
AC_MSG_RESULT($HLVM_PROFILING)
AC_SUBST(HLVM_PROFILING)

dnl -- with-workspace: specifies the location of where workspaces are to be 
dnl -- stored
AC_MSG_CHECKING(location of HLVM workspace)
AC_ARG_WITH(workspace,
  AS_HELP_STRING([--with-workspace=<dir>],
    [dir=location of the HLVM workspace storage (default=/opt/hlvm)]),
  [case "${withval}" in   
    /*|*/*) HLVM_WITH_WORKSPACE=$withval ;;
    *) AC_MSG_ERROR(bad value ${withval} for --with-WORKSPACE) ;;
  esac],[HLVM_WITH_WORKSPACE=/opt/hlvm])
AC_MSG_RESULT($HLVM_WITH_WORKSPACE)
AC_SUBST(HLVM_WITH_WORKSPACE)

dnl -- with-includes: specifies the location of additional header file dirs
AC_MSG_CHECKING(location of additional header files)
AC_ARG_WITH(includes,
  AS_HELP_STRING([--with-includes=-I<dir>...],
    [Specify additional header file directories (efault=none)]),
  [case "${withval}" in   
     -I/*) HLVM_WITH_INCLUDES=$withval ;;
     *) AC_MSG_ERROR(bad value ${withval} for --with-includes) ;;
  esac],[HLVM_WITH_INCLUDES=])
AC_MSG_RESULT($HLVM_WITH_INCLUDES)
AC_SUBST(HLVM_WITH_INCLUDES)
dnl Set up the CPPFLAGS to include the --with places
CPPFLAGS="-D_REENTRANT -D_GNU_SOURCE $HLVM_WITH_INCLUDES"

dnl -- with-llvm-src: specifies the location of the LLVM header files 
AC_MSG_CHECKING(location of LLVM source code)
AC_ARG_WITH(llvm-src,
  AS_HELP_STRING([--with-llvm-src=<dir>],
    [dir=the location of LLVM sources (default=/usr)]),
  [case "${withval}" in 
    /*|*/*) HLVM_WITH_LLVM_SRC=$withval ;;
    *) AC_MSG_ERROR(bad value ${withval} for --with-llvm-src) ;;
  esac],[HLVM_WITH_LLVM_SRC=/usr])
AC_MSG_RESULT($HLVM_WITH_LLVM_SRC)
AC_SUBST(HLVM_WITH_LLVM_SRC)

dnl -- with-llvm-obj: specifies the location of the LLVM object files 
AC_MSG_CHECKING(location of LLVM object code)
AC_ARG_WITH(llvm-obj,
  AS_HELP_STRING([--with-llvm-obj=<dir>],
    [dir=the location of LLVM objects directory (default=/usr)]),
  [case "${withval}" in   
    /*|*/*) HLVM_WITH_LLVM_OBJ=$withval ;;
    *) AC_MSG_ERROR(bad value ${withval} for --with-llvm-obj) ;;
  esac],[HLVM_WITH_LLVM_OBJ=/usr])
AC_MSG_RESULT($HLVM_WITH_LLVM_OBJ)
AC_SUBST(HLVM_WITH_LLVM_OBJ)

dnl -- with-llvm-gcc: specifies the location of the llvm-gcc Compiler
AC_MSG_CHECKING(location of LLVM GCC compiler)
AC_ARG_WITH(llvm-gcc,
  AS_HELP_STRING([--with-llvm-gcc=<dir>],
   [dir=the location of LLVM's llvm-gcc compiler (default=use normal gcc)]),
  [case "${withval}" in
    /*|*/*) HLVM_WITH_LLVMGCC=$withval ;;
    *) AC_MSG_ERROR(bad value ${withval} for --with-llvmgcc) ;;
  esac],[HLVM_WITH_LLVMGCC=""])
AC_MSG_RESULT($HLVM_WITH_LLVMGCC)
AC_SUBST(HLVM_WITH_LLVMGCC)
dnl ----------------------------------------------------------------------------
dnl --
dnl -- SECTION 3: Check For Programs We Need
dnl --
dnl ----------------------------------------------------------------------------

dnl -- Check for the various programs needed for building and installing HLVM
dnl -- Perl is our choice for utility scripts because of its flexibility and
dnl -- wide platform support
AC_PROG_CPP
AC_PROG_CC(gcc)
AC_PROG_CXX(g++)
AC_CHECK_GNU_MAKE
dnl AC_PATH_PROG(path_EGREP, egrep, egrep)
dnl AC_PATH_PROG(path_GPERF, gperf, gperf)
dnl AC_PATH_PROG(path_GPP, g++, g++)
dnl AC_PATH_PROG(path_GPROF, gprof, gprof)
dnl AC_PATH_PROG(path_PERL, perl, perl)
dnl AC_PATH_PROG(path_PKGDATA, pkgdata, pkgdata)
dnl AC_PATH_PROG(path_SORT, sort, sort)
dnl AC_PATH_PROG(path_UNIQ, uniq, uniq)

dnl ----------------------------------------------------------------------------
dnl --
dnl -- SECTION 4: Check For Needed Libraries
dnl --
dnl ----------------------------------------------------------------------------
HLVM_FIND_LIBRARY([apr],[apr-1/apr.h],[apr-1],[apr_allocator_alloc],
                  [Apache Portable Runtime])
HLVM_FIND_LIBRARY([apru],[apr-1/apu.h],[aprutil-1],[apu_version],
                  [Apache Portable Runtime Utilities])
HLVM_FIND_LIBRARY([xml2],[libxml/parser.h],[xml2],
                  [xmlCreateFileParserCtxt],[libxml2 XML C Parser])
HLVM_FIND_LIBRARY([syck],[syck.h],[syck],[syck_parse],[Syck Yaml Handler])

dnl -- Check for specific libraries we depend on.
if test "$HLVM_EFENCE" = true ; then 
  AC_CHECK_LIB(efence,malloc,,
               AC_MSG_ERROR(Electric Fence library is required: -lefence) )
fi

dnl ----------------------------------------------------------------------------
dnl --
dnl -- SECTION 5: Check For Needed Header Files
dnl --
dnl ----------------------------------------------------------------------------
AC_CHECK_HEADERS([expat_external.h llvm/Module.h syck_st.h])

dnl ----------------------------------------------------------------------------
dnl --
dnl -- SECTION 6: HLVM Project specific configuration
dnl --
dnl ----------------------------------------------------------------------------

HLVM_CFGNAME=""
if test "$HLVM_SMALL"    = true ; then HLVM_CFGNAME="${HLVM_CFGNAME}S" ; else
  HLVM_CFGNAME="${HLVM_CFGNAME}s" ; fi
if test "$HLVM_TRACE"    = true ; then HLVM_CFGNAME="${HLVM_CFGNAME}T" ; else
  HLVM_CFGNAME="${HLVM_CFGNAME}t" ; fi
if test "$HLVM_ASSERT"   = true ; then HLVM_CFGNAME="${HLVM_CFGNAME}A" ; else
  HLVM_CFGNAME="${HLVM_CFGNAME}a" ; fi
if test "$HLVM_DEBUG"    = true ; then HLVM_CFGNAME="${HLVM_CFGNAME}D" ; else
  HLVM_CFGNAME="${HLVM_CFGNAME}d" ; fi
if test "$HLVM_INLINE"   = true ; then HLVM_CFGNAME="${HLVM_CFGNAME}I" ; else
  HLVM_CFGNAME="${HLVM_CFGNAME}i" ; fi
if test "$HLVM_OPTIMIZE" = true ; then HLVM_CFGNAME="${HLVM_CFGNAME}O" ; else
  HLVM_CFGNAME="${HLVM_CFGNAME}o" ; fi
AC_SUBST(HLVM_CFGNAME)

if test "$HLVM_SHAREDPREFIX" = false ; then
  AC_MSG_NOTICE([Configuring For Unique Installation])
  eval HLVM_PREFIX="${prefix}/${HLVM_CFGNAME}"
  if test "${exec_prefix}" = "NONE"; then
    eval HLVM_EXEC_PREFIX="${prefix}/${HLVM_CFGNAME}"
  else
    eval HLVM_EXEC_PREFIX="${exec_prefix}/${HLVM_CFGNAME}"
  fi
else 
  AC_MSG_NOTICE([Configuring For Shared Installation: ${HLVM_CFGNAME}])
  eval HLVM_PREFIX="${prefix}"
  if test "${exec_prefix}" = "NONE"; then
    eval HLVM_EXEC_PREFIX="${prefix}"
  else
    eval HLVM_EXEC_PREFIX="${exec_prefix}"
  fi
fi

dnl Check the --with-* variables for sanity
dnl if test \! -f "$HLVM_WITH_LLVM_SRC/lib/VMCore/Module.cpp" ; then
dnl   AC_MSG_ERROR([LLVM sources are not in $HLVM_WITH_LLVM_SRC])
dnl fi
dnl if test \! -f "$HLVM_WITH_LLVM_OBJ/Makefile.config" ; then
dnl   AC_MSG_ERROR([LLVM objects are not in $HLVM_WITH_LLVM_OBJ])
dnl fi

HLVM_CONFIGTIME=`date`
AC_SUBST(HLVM_CONFIGTIME)
HLVM_PREFIX="$prefix"
AC_SUBST(HLVM_PREFIX)

dnl ----------------------------------------------------------------------------
dnl --
dnl -- SECTION 7: Output the configure script and makefiles.
dnl --
dnl ----------------------------------------------------------------------------

AC_CONFIG_HEADER([hlvm/Base/Config.h])
AC_CONFIG_FILES([VERSION Makefile.config docs/Doxyfile])
AC_CONFIG_FILES([hlvm/Base/ConfigData.cpp])
AC_CONFIG_MAKEFILE(Makefile)
AC_CONFIG_MAKEFILE(Makefile.hlvm)
AC_CONFIG_MAKEFILE(docs/Makefile)
AC_CONFIG_MAKEFILE(hlvm/Makefile)
AC_CONFIG_MAKEFILE(tools/Makefile)
AC_CONFIG_MAKEFILE(test/Makefile)

dnl This must be last
AC_OUTPUT
echo ""
echo "Configuration of HLVM is now finished."
echo ""
