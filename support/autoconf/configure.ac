dnl === configure.ac --------------------------------------------------------===
dnl                     LLVM Support Module Configuration
dnl
dnl This file was developed by the LLVM research group and is distributed under
dnl the University of Illinois Open Source License. See LICENSE.TXT for details.
dnl 
dnl===-----------------------------------------------------------------------===
dnl This is the support module configuration script. It is processed by the 
dnl autoconf program to produce a script named configure. This script contains 
dnl the configuration checks that the support module needs in order to support 
dnl multiple platforms.
dnl
dnl This file is composed of 10 sections per the recommended organization of
dnl autoconf input defined in the autoconf documentation. As this file evolves,
dnl please keep the various types of checks within their sections. The sections
dnl are as follows:
dnl
dnl SECTION 1: Initialization & Setup
dnl SECTION 2: Architecture, target, and host checks
dnl SECTION 3: Command line arguments for the configure script.
dnl SECTION 4: Check for programs we need and that they are the right version
dnl SECTION 5: Check for libraries
dnl SECTION 6: Check for header files
dnl SECTION 7: Check for types and structures
dnl SECTION 8: Check for specific functions needed
dnl SECTION 9: Additional checks, variables, etc.
dnl SECTION 10: Specify the output files and generate it
dnl
dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 1: Initialization & Setup
dnl===
dnl===-----------------------------------------------------------------------===
dnl Initialize autoconf and define the package name, version number and
dnl email address for reporting bugs.
AC_INIT([[LLVM Support Module]],[[2.2svn]],[http://llvm.org/bugs],[llvm-support])
THIS_IS_LLVM_MODULE([support])

dnl Verify that the source directory is valid. This makes sure that we are
dnl configuring LLVM and not some other package (it validates --srcdir argument)
AC_CONFIG_SRCDIR([lib/System/Path.cpp])

dnl Quit if the source directory has already been configured.
dnl NOTE: This relies upon undocumented autoconf behavior.
if test ${srcdir} != "." ; then
  if test -f ${srcdir}/include/llvm/Config/config.h ; then
    AC_MSG_ERROR([Already configured in ${srcdir}])
  fi
fi

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 2: Architecture, target, and host checks
dnl===
dnl===-----------------------------------------------------------------------===

dnl Check the target for which we're compiling and the host that will do the
dnl compilations. This will tell us which LLVM compiler will be used for 
dnl compiling SSA into object code. This needs to be done early because 
dnl following tests depend on it.
AC_CANONICAL_TARGET

dnl Determine the platform type and cache its value. This helps us configure
dnl the System library to the correct build platform.
AC_CACHE_CHECK([type of operating system we're going to host on],
               [llvm_cv_os_type],
[case $host in
  *-*-aix*) 
    llvm_cv_link_all_option="-Wl,--whole-archive"
    llvm_cv_no_link_all_option="-Wl,--no-whole-archive"
    llvm_cv_os_type="AIX" 
    llvm_cv_platform_type="Unix" ;;
  *-*-irix*) 
    llvm_cv_link_all_option="-Wl,--whole-archive"
    llvm_cv_no_link_all_option="-Wl,--no-whole-archive"
    llvm_cv_os_type="IRIX" 
    llvm_cv_platform_type="Unix" ;;
  *-*-cygwin*) 
    llvm_cv_link_all_option="-Wl,--whole-archive"
    llvm_cv_no_link_all_option="-Wl,--no-whole-archive"
    llvm_cv_os_type="Cygwin" 
    llvm_cv_platform_type="Unix" ;;
  *-*-darwin*) 
    llvm_cv_link_all_option="-Wl,-all_load"
    llvm_cv_link_all_option="-Wl,-noall_load"
    llvm_cv_os_type="Darwin"
    llvm_cv_platform_type="Unix" ;;
  *-*-freebsd*) 
    llvm_cv_link_all_option="-Wl,--whole-archive"
    llvm_cv_no_link_all_option="-Wl,--no-whole-archive"
    llvm_cv_os_type="FreeBSD" 
    llvm_cv_platform_type="Unix" ;;
  *-*-openbsd*) 
    llvm_cv_link_all_option="-Wl,--whole-archive"
    llvm_cv_no_link_all_option="-Wl,--no-whole-archive"
    llvm_cv_os_type="OpenBSD" 
    llvm_cv_platform_type="Unix" ;;
  *-*-netbsd*) 
    llvm_cv_link_all_option="-Wl,--whole-archive"
    llvm_cv_no_link_all_option="-Wl,--no-whole-archive"
    llvm_cv_os_type="NetBSD" 
    llvm_cv_platform_type="Unix" ;;
  *-*-hpux*) 
    llvm_cv_link_all_option="-Wl,--whole-archive"
    llvm_cv_no_link_all_option="-Wl,--no-whole-archive"
    llvm_cv_os_type="HP-UX" 
    llvm_cv_platform_type="Unix" ;;
  *-*-interix*) 
    llvm_cv_link_all_option="-Wl,--whole-archive"
    llvm_cv_no_link_all_option="-Wl,--no-whole-archive"
    llvm_cv_os_type="Interix"
    llvm_cv_platform_type="Unix" ;;
  *-*-linux*) 
    llvm_cv_link_all_option="-Wl,--whole-archive"
    llvm_cv_no_link_all_option="-Wl,--no-whole-archive"
    llvm_cv_os_type="Linux"
    llvm_cv_platform_type="Unix" ;;
  *-*-solaris*) 
    llvm_cv_link_all_option="-Wl,-z,allextract"
    llvm_cv_no_link_all_option="-Wl,-z,defaultextract"
    llvm_cv_os_type="SunOS"
    llvm_cv_platform_type="Unix" ;;
  *-*-win32*) 
    llvm_cv_link_all_option="-Wl,--whole-archive"
    llvm_cv_no_link_all_option="-Wl,--no-whole-archive"
    llvm_cv_os_type="Win32"
    llvm_cv_platform_type="Win32" ;;
  *-*-mingw*) 
    llvm_cv_link_all_option="-Wl,--whole-archive"
    llvm_cv_no_link_all_option="-Wl,--no-whole-archive"
    llvm_cv_os_type="MingW" 
    llvm_cv_platform_type="Win32" ;;
  *) 
    llvm_cv_link_all_option="" 
    llvm_cv_no_link_all_option=""
    llvm_cv_os_type="Unknown" 
    llvm_cv_platform_type="Unknown" ;;
esac])

dnl Make sure we aren't attempting to configure for an unknown system
if test "$llvm_cv_os_type" = "Unknown" ; then
  AC_MSG_ERROR([Operating system is unknown, configure can't continue])
fi

dnl Set the "OS" Makefile variable based on the platform type so the
dnl makefile can configure itself to specific build hosts
AC_SUBST(OS,$llvm_cv_os_type)

dnl Set the LINKALL and NOLINKALL Makefile variables based on the platform
AC_SUBST(LINKALL,$llvm_cv_link_all_option)
AC_SUBST(NOLINKALL,$llvm_cv_no_link_all_option)

dnl Set the "LLVM_ON_*" variables based on llvm_cvs_platform_type
dnl This is used by lib/System to determine the basic kind of implementation
dnl to use.
case $llvm_cv_platform_type in
  Unix)  
    AC_DEFINE([LLVM_ON_UNIX],[1],[Define if this is Unixish platform]) 
    AC_SUBST(LLVM_ON_UNIX,[1])
    AC_SUBST(LLVM_ON_WIN32,[0])
    ;;
  Win32) 
    AC_DEFINE([LLVM_ON_WIN32],[1],[Define if this is Win32ish platform]) 
    AC_SUBST(LLVM_ON_UNIX,[0])
    AC_SUBST(LLVM_ON_WIN32,[1])
    ;;
esac

dnl Determine what our target architecture is and configure accordingly.
dnl This will allow Makefiles to make a distinction between the hardware and
dnl the OS.
AC_CACHE_CHECK([target architecture],[llvm_cv_target_arch],
[case $target in
  i?86-*)                 llvm_cv_target_arch="x86" ;;
  amd64-* | x86_64-*)     llvm_cv_target_arch="x86_64" ;;
  sparc*-*)               llvm_cv_target_arch="Sparc" ;;
  powerpc*-*)             llvm_cv_target_arch="PowerPC" ;;
  alpha*-*)               llvm_cv_target_arch="Alpha" ;;
  ia64-*)                 llvm_cv_target_arch="IA64" ;;
  arm-*)                  llvm_cv_target_arch="ARM" ;;
  *)                      llvm_cv_target_arch="Unknown" ;;
esac])

if test "$llvm_cv_target_arch" = "Unknown" ; then
  AC_MSG_WARN([Configuring LLVM for an unknown target archicture])
fi

dnl Define a substitution, ARCH, for the target architecture
AC_SUBST(ARCH,$llvm_cv_target_arch)

dnl Check for the endianness of the target
AC_C_BIGENDIAN(AC_SUBST([ENDIAN],[big]),AC_SUBST([ENDIAN],[little]))

dnl Check for build platform executable suffix if we're crosscompiling
if test "$cross_compiling" = yes; then
  AC_SUBST(LLVM_CROSS_COMPILING, [1])  
  AC_BUILD_EXEEXT
else
  AC_SUBST(LLVM_CROSS_COMPILING, [0])
fi

dnl Check to see if there's a "CVS" (or .svn) directory indicating that this 
dnl build is being done from a checkout. This sets up several defaults for the
dnl command line switches. When we build with a CVS directory, we get a 
dnl debug with assertions turned on. Without, we assume a source release and we
dnl get an optimized build without assertions. See --enable-optimized and
dnl --enable-assertions below
if test -d ".svn" -o -d "${srcdir}/.svn" -o -d "CVS" -o -d "${srcdir}/CVS"; then
  svnbuild="yes"
  optimize="no"
  AC_SUBST(SVNBUILD,[[SVNBUILD=1]])
else
  svnbuild="no"
  optimize="yes"
fi

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 3: Command line arguments for the configure script.
dnl===
dnl===-----------------------------------------------------------------------===

dnl Define the standard LLVM compilation features
LLVM_WANT_FEATURE(assertions,[Compile with assertion checks enabled],yes)
LLVM_WANT_FEATURE(debug,[Produce debug symbols in compilation objects],yes)
LLVM_WANT_FEATURE(doxygen,[Build doxygen documentation],no)
LLVM_WANT_FEATURE(checking,[Compile with expensive checking enabled],no)
LLVM_WANT_FEATURE(optimized,[Compile with optimizations enabled],no)
LLVM_WANT_FEATURE(opt-for-size,[Optimize for executable size not speed],no)
LLVM_WANT_FEATURE(profiling,[Compile with profiling enabled],yes)
LLVM_WANT_FEATURE(generate-pic,[Generate position independent code],no)
LLVM_WANT_FEATURE(stripped,[Strip symbols from libraries and executables],no)
LLVM_WANT_FEATURE(threads,[Use threads if available],yes)
LLVM_WANT_LEVEL(verbose,[Show each command executed while building],0,0,3)

dnl Allow configuration of the llvm-top directory from which all llvm software
dnl can be found. Usually this corresponds to .., but weird things can happen
dnl in build environments
AC_ARG_WITH(llvm-top,
  AS_HELP_STRING([--with-llvm-top],
                 [Specify where the llvm-top directory is]),,withval=default)
case "$withval" in
  default)  LLVM_TOP=`cd .. ; pwd` ;; 
  *)        LLVM_TOP=$withval ;;
esac
AC_SUBST(LLVM_TOP,$LLVM_TOP)

dnl Allow specification of the destination directory which is a meta-prefix for
dnl the installation. It allows package maintainers for operating systems to
dnl build an operating system image in a destination directory that uses the
dnl final paths, without damaging their own installation.
AC_ARG_WITH([destdir],
  AS_HELP_STRING([--with-destdir],
                 [Specify the install destination directory]),,withval=default)
case "$withval" in
  default)  DESTDIR='' ;;
  *)        DESTDIR=$withval ;;
esac
AC_SUBST(DESTDIR,$DESTDIR)

dnl Specify extra build options
AC_ARG_WITH(extra-options,
  AS_HELP_STRING([--with-extra-options],
                 [Specify addtional options to compile LLVM with]),,
                 withval=default)
case "$withval" in
  default) EXTRA_OPTIONS="" ;;
  *) EXTRA_OPTIONS=$withval ;;
esac
AC_SUBST(EXTRA_OPTIONS,$EXTRA_OPTIONS)

dnl Allow specific targets to be specified for building (or not)
TARGETS_TO_BUILD=""
AC_ARG_ENABLE([targets],AS_HELP_STRING([--enable-targets],
    [Build specific host targets: all,host-only,{target-name} (default=all)]),,
    enableval=all)
case "$enableval" in
  all) TARGETS_TO_BUILD="X86 Sparc PowerPC Alpha IA64 ARM MIPS" ;;
  host-only)
    case "$llvm_cv_target_arch" in
      x86)     TARGETS_TO_BUILD="X86" ;;
      x86_64)  TARGETS_TO_BUILD="X86" ;;
      Sparc)   TARGETS_TO_BUILD="Sparc" ;;
      PowerPC) TARGETS_TO_BUILD="PowerPC" ;;
      Alpha)   TARGETS_TO_BUILD="Alpha" ;;
      IA64)    TARGETS_TO_BUILD="IA64" ;;
      ARM)     TARGETS_TO_BUILD="ARM" ;;
      MIPS)    TARGETS_TO_BUILD="MIPS" ;;
      *)       AC_MSG_ERROR([Can not set target to build]) ;;
    esac 
    ;;
  *)for a_target in `echo $enableval|sed -e 's/,/ /g' ` ; do
      case "$a_target" in
        x86)     TARGETS_TO_BUILD="X86 $TARGETS_TO_BUILD" ;;
        x86_64)  TARGETS_TO_BUILD="X86 $TARGETS_TO_BUILD" ;;
        sparc)   TARGETS_TO_BUILD="Sparc $TARGETS_TO_BUILD" ;;
        powerpc) TARGETS_TO_BUILD="PowerPC $TARGETS_TO_BUILD" ;;
        alpha)   TARGETS_TO_BUILD="Alpha $TARGETS_TO_BUILD" ;;
        ia64)    TARGETS_TO_BUILD="IA64 $TARGETS_TO_BUILD" ;;
        arm)     TARGETS_TO_BUILD="ARM $TARGETS_TO_BUILD" ;;
        mips)    TARGETS_TO_BUILD="MIPS $TARGETS_TO_BUILD" ;;
        *) AC_MSG_ERROR([Unrecognized target $a_target]) ;;
      esac
  done 
  ;;
esac
TARGETS_TO_BUILD="CBackend MSIL $TARGETS_TO_BUILD"
AC_SUBST(TARGETS_TO_BUILD,$TARGETS_TO_BUILD)

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 4: Check for programs we need and that they are the right version
dnl===
dnl===-----------------------------------------------------------------------===

dnl Check for standard compilation tools
AC_PROG_INSTALL
AC_PROG_CPP
AC_PROG_CC(gcc)
AC_PROG_CXX(g++)
AC_PROG_FLEX
AC_PROG_BISON
AC_PROG_RANLIB
AC_PROG_NM
AC_PROG_LN_S
AC_SUBST(NM)

dnl Check for various build and documentation tools that could be used
AC_CHECK_GNU_MAKE
AC_PATH_PROG(BINPWD,[pwd],  [pwd])
AC_PATH_PROG(BZIP2,[bzip2],[echo "Skipped: bzip2 not found"])
AC_PATH_PROG(CMP, [cmp], [cmp])
AC_PATH_PROG(CP, [cp], [cp])
AC_PATH_PROG(DOXYGEN,[doxygen],[echo "Skipped: doxygen not found"])
AC_PATH_PROG(DATE, [date], [date])
AC_PATH_PROG(DOT, [dot], [echo dot])
AC_PATH_PROG(DOTTY, [dotty], [echo dotty])
AC_PATH_PROG(FIND, [find], [find])
AC_PATH_PROG(GREP, [grep], [grep])
AC_PATH_PROG(GRAPHVIZ, [Graphviz], [echo Graphviz])
AC_PATH_PROG(GROFF,[groff],[echo "Skipped: groff not found"])
AC_PATH_PROG(GV, [gv gsview32], [echo gv])
AC_PATH_PROG(GZIP,[gzip],[echo "Skipped: gzip not found"])
AC_PATH_PROG(MKDIR,[mkdir],[mkdir])
AC_PATH_PROG(MV,   [mv],   [mv])
AC_PATH_PROG(POD2HTML,[pod2html],[echo "Skipped: pod2html not found"])
AC_PATH_PROG(POD2MAN,[pod2man],[echo "Skipped: pod2man not found"])
AC_PATH_PROG(RM,   [rm],   [rm])
AC_PATH_PROG(RUNTEST,[runtest],[echo "Skipped: runtest not found"])
AC_PATH_PROG(SED,  [sed],  [sed])
AC_PATH_PROG(TAR,  [tar],  [gtar])
AC_PATH_PROG(ZIP,[zip],[echo "Skipped: zip not found"])
DJ_AC_PATH_TCLSH

dnl Adjust program paths for specific platforms so they can be used directly
dnl inside the program from config.h
if test "$GRAPHVIZ" != "echo Graphviz" ; then
  AC_DEFINE([HAVE_GRAPHVIZ],[1],[Define if the Graphviz program is available])
  dnl If we're targeting for mingw we should emit windows paths, not msys
  if test "$llvm_cv_os_type" = "MingW" ; then
    GRAPHVIZ=`echo $GRAPHVIZ | sed 's/^\/\([[A-Za-z]]\)\//\1:\//' `
  fi
  AC_DEFINE_UNQUOTED([LLVM_PATH_GRAPHVIZ],"$GRAPHVIZ${EXEEXT}",
   [Define path to Graphviz program if found or 'echo Graphviz' otherwise])
fi
if test "$DOT" != "echo dot" ; then
  AC_DEFINE([HAVE_DOT],[1],[Define if the dot program is available])
  dnl If we're targeting for mingw we should emit windows paths, not msys
  if test "$llvm_cv_os_type" = "MingW" ; then
    DOT=`echo $DOT | sed 's/^\/\([[A-Za-z]]\)\//\1:\//' `
  fi
  AC_DEFINE_UNQUOTED([LLVM_PATH_DOT],"$DOT${EXEEXT}",
   [Define path to dot program if found or 'echo dot' otherwise])
fi
if test "$GV" != "echo gv" ; then
  AC_DEFINE([HAVE_GV],[1],[Define if the gv program is available])
  dnl If we're targeting for mingw we should emit windows paths, not msys
  if test "$llvm_cv_os_type" = "MingW" ; then
    GV=`echo $GV | sed 's/^\/\([[A-Za-z]]\)\//\1:\//' `
  fi
  AC_DEFINE_UNQUOTED([LLVM_PATH_GV],"$GV${EXEEXT}",
   [Define path to gv program if found or 'echo gv' otherwise])
fi
if test "$DOTTY" != "echo dotty" ; then
  AC_DEFINE([HAVE_DOTTY],[1],[Define if the dotty program is available])
  dnl If we're targeting for mingw we should emit windows paths, not msys
  if test "$llvm_cv_os_type" = "MingW" ; then
    DOTTY=`echo $DOTTY | sed 's/^\/\([[A-Za-z]]\)\//\1:\//' `
  fi
  AC_DEFINE_UNQUOTED([LLVM_PATH_DOTTY],"$DOTTY${EXEEXT}",
   [Define to path to dotty program if found or 'echo dotty' otherwise])
fi

dnl Look for a sufficiently recent version of Perl.
LLVM_PROG_PERL([5.006])
AC_SUBST(PERL)
if test x"$PERL" = xnone; then
   AC_SUBST(HAVE_PERL,0)
   AC_MSG_ERROR([perl is required but was not found, please install it])
else
   AC_SUBST(HAVE_PERL,1)
fi

dnl Checks for documentation and testing tools that we can do without. If these
dnl are not found then they are set to "true" which always succeeds but does
dnl nothing. This just lets the build output show that we could have done 
dnl something if the tool was available. 

dnl Determine if the linker supports the -R option.
AC_LINK_USE_R

dnl Check for libtool and the library that has dlopen function (which must come
dnl before the AC_PROG_LIBTOOL check in order to enable dlopening libraries with
dnl libtool).
AC_LIBTOOL_DLOPEN
AC_LIB_LTDL
AC_PROG_LIBTOOL

if test "$lt_cv_dlopen_self" = "yes" ; then
  AC_DEFINE([CAN_DLOPEN_SELF],[1],
              [Define if dlopen(0) will open the symbols of the program])
fi

dnl Check if we know how to tell etags we are using C++:
etags_version=`$ETAGS --version 2>&1`
case "$etags_version" in
	*[Ee]xuberant*) ETAGSFLAGS="--language-force=c++" ;;
	*GNU\ Emacs*) ETAGSFLAGS="-l c++" ;;
	*) ETAGSFLAGS="" ;;
esac
AC_SUBST(ETAGSFLAGS,$ETAGSFLAGS)

if test "$WITH_LLVMGCCDIR" = "default" ; then
  LLVMGCC="llvm-gcc${EXEEXT}"
  LLVMGXX="llvm-g++${EXEEXT}"
  AC_PATH_PROG(LLVMGCC, $LLVMGCC, [])
  AC_PATH_PROG(LLVMGXX, $LLVMGXX, [])
else
  LLVMGCC="$WITH_LLVMGCCDIR/bin/llvm-gcc${EXEEXT}"
  LLVMGXX="$WITH_LLVMGCCDIR/bin/llvm-g++${EXEEXT}"
  AC_SUBST(LLVMGCC,$LLVMGCC)
  AC_SUBST(LLVMGXX,$LLVMGXX)
fi
  
AC_MSG_CHECKING([tool compatibility])

dnl Ensure that compilation tools are GCC or a GNU compatible compiler such as
dnl ICC; we use GCC specific options in the makefiles so the compiler needs
dnl to support those options.
dnl "icc" emits gcc signatures
dnl "icc -no-gcc" emits no gcc signature BUT is still compatible
ICC=no
IXX=no
case $CC in
  icc*|icpc*) 
    ICC=yes
    IXX=yes
    ;;
   *)
    ;;
esac

if test "$GCC" != "yes" && test "$ICC" != "yes"
then
  AC_MSG_ERROR([gcc|icc required but not found])
fi

dnl Ensure that compilation tools are GCC; we use GCC specific extensions
if test "$GXX" != "yes" && test "$IXX" != "yes"
then
  AC_MSG_ERROR([g++|icc required but not found])
fi

dnl Verify that GCC is version 3.0 or higher
if test "$GCC" = "yes"
then
  gccmajor=`$CC --version | head -n 1 | sed 's/[[^0-9]]*\([[0-9.]]\).*/\1/'` 
  if test "$gccmajor" -lt "3"
  then
    AC_MSG_ERROR([gcc 3.x required, but you have a lower version])
  fi
fi

dnl Check for GNU Make.  We use its extensions, so don't build without it
if test -z "$llvm_cv_gnu_make_command"
then
  AC_MSG_ERROR([GNU Make required but not found])
fi

dnl Tool compatibility is okay if we make it here.
AC_MSG_RESULT([ok])

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 5: Check for libraries
dnl===
dnl===-----------------------------------------------------------------------===

dnl libelf is for sparc only; we can ignore it if we don't have it
AC_CHECK_LIB(elf, elf_begin)
AC_CHECK_LIB(m,sin)
if test "$llvm_cv_os_type" = "MingW" ; then
  AC_CHECK_LIB(imagehlp, main)
  AC_CHECK_LIB(psapi, main)
fi

dnl lt_dlopen may be required for plugin support.
AC_SEARCH_LIBS(lt_dlopen,ltdl,AC_DEFINE([HAVE_LT_DLOPEN],[1],
              [Define if lt_dlopen() is available on this platform]),
              AC_MSG_WARN([lt_dlopen() not found - plugin support might 
                           not be available]))

dnl dlopen() is required for plugin support.
AC_SEARCH_LIBS(dlopen,dl,AC_DEFINE([HAVE_DLOPEN],[1],
               [Define if dlopen() is available on this platform.]),
               AC_MSG_WARN([dlopen() not found - disabling plugin support]))

dnl mallinfo is optional; the code can compile (minus features) without it
AC_SEARCH_LIBS(mallinfo,malloc,AC_DEFINE([HAVE_MALLINFO],[1],
               [Define if mallinfo() is available on this platform.]))

dnl pthread locking functions are optional - but llvm will not be thread-safe
dnl without locks.
if test "$WANT_THREADS" -eq 1 ; then
  AC_CHECK_LIB(pthread,pthread_mutex_init)
  AC_SEARCH_LIBS(pthread_mutex_lock,pthread,
                 AC_DEFINE([HAVE_PTHREAD_MUTEX_LOCK],[1],
                           [Have pthread_mutex_lock]))
fi

dnl Allow extra x86-disassembler library
AC_ARG_WITH(udis86,
  AS_HELP_STRING([--with-udis86=<path>],
    [Use udis86 external x86 disassembler library]),
    [
      AC_SUBST(USE_UDIS86, [1])
      case "$withval" in
        /usr/lib|yes) ;;
        *) LDFLAGS="$LDFLAGS -L${withval}" ;;
      esac
      AC_CHECK_LIB(udis86, ud_init, [], [
        echo "Error! You need to have libudis86 around."
        exit -1
      ])
    ],
    AC_SUBST(USE_UDIS86, [0]))
AC_DEFINE_UNQUOTED([USE_UDIS86],$USE_UDIS86,
                   [Define if use udis86 library])

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 6: Check for header files
dnl===
dnl===-----------------------------------------------------------------------===

dnl First, use autoconf provided macros for specific headers that we need
dnl We don't check for ancient stuff or things that are guaranteed to be there
dnl by the C++ standard. We always use the <cfoo> versions of <foo.h> C headers.
dnl Generally we're looking for POSIX headers.
AC_HEADER_DIRENT
AC_HEADER_MMAP_ANONYMOUS
AC_HEADER_STAT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME

AC_CHECK_HEADERS([dlfcn.h execinfo.h fcntl.h inttypes.h limits.h link.h])
AC_CHECK_HEADERS([malloc.h setjmp.h signal.h stdint.h unistd.h utime.h])
AC_CHECK_HEADERS([windows.h sys/mman.h sys/param.h sys/resource.h sys/time.h])
AC_CHECK_HEADERS([sys/types.h malloc/malloc.h mach/mach.h])
if test "$WANT_THREADS" -eq 1 ; then
  AC_CHECK_HEADERS([pthread.h])
else
  AC_SUBST(HAVE_PTHREAD_,0)
fi

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 7: Check for types and structures
dnl===
dnl===-----------------------------------------------------------------------===

AC_HUGE_VAL_CHECK
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL
AC_STRUCT_TM
AC_CHECK_TYPES([int64_t],,AC_MSG_ERROR([Type int64_t required but not found]))
AC_CHECK_TYPES([uint64_t],,
         AC_CHECK_TYPES([u_int64_t],,
         AC_MSG_ERROR([Type uint64_t or u_int64_t required but not found])))

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 8: Check for specific functions needed
dnl===
dnl===-----------------------------------------------------------------------===

AC_CHECK_FUNCS([backtrace ceilf floorf roundf rintf nearbyintf getcwd ])
AC_CHECK_FUNCS([powf fmodf strtof round ])
AC_CHECK_FUNCS([getpagesize getrusage getrlimit setrlimit gettimeofday ])
AC_CHECK_FUNCS([isatty mkdtemp mkstemp ])
AC_CHECK_FUNCS([mktemp realpath sbrk setrlimit strdup strerror strerror_r ])
AC_CHECK_FUNCS([strtoll strtoq sysconf malloc_zone_statistics ])
AC_CHECK_FUNCS([setjmp longjmp sigsetjmp siglongjmp])
AC_C_PRINTF_A
AC_FUNC_ALLOCA
AC_FUNC_RAND48

dnl Check for variations in the Standard C++ library and STL. These macros are
dnl provided by LLVM in the autoconf/m4 directory.
AC_CXX_HAVE_HASH_MAP
AC_CXX_HAVE_HASH_SET
AC_CXX_HAVE_STD_ITERATOR
AC_CXX_HAVE_BI_ITERATOR
AC_CXX_HAVE_FWD_ITERATOR
AC_FUNC_ISNAN
AC_FUNC_ISINF

dnl Check for mmap support.We also need to know if /dev/zero is required to 
dnl be opened for allocating RWX memory.  
dnl Make sure we aren't attempting to configure for an unknown system
if test "$llvm_cv_platform_type" = "Unix" ; then
  AC_FUNC_MMAP 
  AC_FUNC_MMAP_FILE
  AC_NEED_DEV_ZERO_FOR_MMAP

  if test "$ac_cv_func_mmap_fixed_mapped" = "no"
  then
    AC_MSG_WARN([mmap() of a fixed address required but not supported])
  fi
  if test "$ac_cv_func_mmap_file" = "no"
  then
    AC_MSG_WARN([mmap() of files required but not found])
  fi
fi

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 9: Additional checks, variables, etc.
dnl===
dnl===-----------------------------------------------------------------------===

dnl Propagate the shared library extension that the libltdl checks did to 
dnl the Makefiles so we can use it there too
AC_SUBST(SHLIBEXT,$libltdl_cv_shlibext)

dnl Get the time at which we were configured.
LLVM_CONFIGTIME=`date`
AC_SUBST(LLVM_CONFIGTIME)
AC_DEFINE_UNQUOTED([LLVM_CONFIGTIME],["$LLVM_CONFIGTIME"],
                   [Time at which LLVM was configured])

# Make sure we have a prefix and exec_prefix value that makes sense.
if test "${prefix}" = "NONE" ; then
  prefix="/usr/local"
fi
if test "${exec_prefix}" = "NONE" ; then
  exec_prefix="$prefix"
fi

dnl make sure we resolve shell variables for paths because silly autoconf 
dnl doesn't get this right.
eval bindir="$bindir"
eval sbindir="$sbindir"
eval libexecdir="$libexecdir"
eval sysconfdir="$sysconfdir"
eval sharedstatedir="$sharedstatedir"
eval localstatedir="$localstatedir"
eval libdir="$libdir"
eval includedir="$includedir"
eval oldincludedir="$oldincludedir"
eval datarootdir="$datarootdir"
eval datadir="$datadir"
eval infodir="$infodir"
eval localedir="$localedir"
eval mandir="$mandir"
eval docdir="$docdir"
eval htmldir="$htmldir"
eval dvidir="$dvidir"
eval pdfdir="$pdfdir"
eval psdir="$psdir"


dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 10: Specify the output files and generate it
dnl===
dnl===-----------------------------------------------------------------------===

dnl Configure header files
dnl WARNING: dnl If you add or remove any of the following config headers, then
dnl you MUST also update Makefile.rules so that the variable FilesToConfig
dnl contains the same list of files as AC_CONFIG_HEADERS below. This ensures the
dnl files can be updated automatically when their *.in sources change.
AC_CONFIG_HEADERS([include/llvm/Config/config.h])
AC_CONFIG_HEADERS([include/llvm/Support/DataTypes.h])
AC_CONFIG_HEADERS([include/llvm/ADT/hash_map])
AC_CONFIG_HEADERS([include/llvm/ADT/hash_set])
AC_CONFIG_HEADERS([include/llvm/ADT/iterator])

dnl Do special configuration of Makefiles
CONFIG_LLVM_MAKEFILE(Makefile)
CONFIG_LLVM_MAKEFILE(lib/Makefile)

dnl Finally, crank out the output
AC_OUTPUT
