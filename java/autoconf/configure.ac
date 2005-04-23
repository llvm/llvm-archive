dnl **************************************************************************
dnl * Initialize
dnl **************************************************************************
AC_INIT([[Java]], [[0.0]], [llvmbugs@cs.uiuc.edu])

dnl Tell autoconf that the auxilliary files are actually located in
dnl the LLVM autoconf directory, not here.
AC_CONFIG_AUX_DIR([../../autoconf])

dnl Tell autoconf that this is an LLVM project being configured
dnl This provides the --with-llvmsrc and --with-llvmobj options
LLVM_CONFIG_PROJECT

dnl Verify that the source directory is valid
AC_CONFIG_SRCDIR([Makefile.common.in])

dnl Configure a common Makefile
AC_CONFIG_FILES(Makefile.common)

dnl Configure project makefiles
dnl List every Makefile that exists within your source tree
AC_CONFIG_MAKEFILE([Makefile])
AC_CONFIG_MAKEFILE([lib/Makefile])
AC_CONFIG_MAKEFILE([lib/ClassFile/Makefile])
AC_CONFIG_MAKEFILE([test/Makefile])
AC_CONFIG_MAKEFILE([test/Makefile.test])
AC_CONFIG_MAKEFILE([test/Programs/SingleSource/UnitTests/Makefile])
AC_CONFIG_MAKEFILE([test/Programs/SingleSource/Makefile])
AC_CONFIG_MAKEFILE([test/Programs/SingleSource/Makefile.singlesrc])
AC_CONFIG_MAKEFILE([test/Programs/Makefile])
AC_CONFIG_MAKEFILE([test/Regression/ClassFile/Makefile])
AC_CONFIG_MAKEFILE([test/Regression/Makefile])
AC_CONFIG_MAKEFILE([tools/Makefile])
AC_CONFIG_MAKEFILE([tools/classdump/Makefile])

dnl **************************************************************************
dnl * Determine which system we are building on
dnl **************************************************************************

dnl **************************************************************************
dnl * Check for programs.
dnl **************************************************************************
AC_ARG_VAR(JAVA, [The Java Virtual Machine (used for testing)])
AC_CHECK_PROG(JAVA, java, java)
AC_ARG_VAR(JAVAC, [The java compiler (used for testing)])
AC_CHECK_PROG(JAVAC, javac, javac)
AC_ARG_VAR(JAVAH, [The java JNI header generator (used for testing)])
AC_CHECK_PROG(JAVAH, javah, javah)

if test -z "$JAVA"
then
  AC_MSG_WARN([A Java Virtual Machine is required for the test suite, but it was not found])
fi

if test -z "$JAVAC"
then
  AC_MSG_WARN([A Java compiler is required for the test suite, but it was not found])
fi

if test -z "$JAVAC"
then
  AC_MSG_WARN([A Java JNI header generator is required for the test suite, but it was not found])
fi

dnl Verify that the source directory is valid
AC_CONFIG_SRCDIR(Makefile.common.in)

dnl **************************************************************************
dnl * Set the location of various third-party software packages
dnl **************************************************************************
AC_ARG_WITH([classpathdir],
  AS_HELP_STRING([--with-classpathdir],
                 [location of GNU classpath install dir (default /usr/local)]),
                 [ac_cv_classpathdir=$withval],
                 [ac_cv_classpathdir=/usr/local])

CLASSPATH_JAVA_LIBRARY_PATH=$ac_cv_classpathdir/share/classpath
AC_SUBST(CLASSPATH_JAVA_LIBRARY_PATH)

CLASSPATH_NATIVE_LIBRARY_PATH=$ac_cv_classpathdir/lib/classpath
AC_SUBST(CLASSPATH_NATIVE_LIBRARY_PATH)

CLASSPATH_NATIVE_LIBS="gtkpeer javaawt javaio javalang javalangreflect javanet javanio javautil"
AC_SUBST(CLASSPATH_NATIVE_LIBS)

dnl **************************************************************************
dnl * Check for libraries.
dnl **************************************************************************
AC_MSG_CHECKING([for GNU classpath libraries])
if test ! -f "$CLASSPATH_JAVA_LIBRARY_PATH/java/lang/Object.class"
then
  AC_MSG_ERROR([A flat installation of GNU classpath is required])
fi
AC_MSG_RESULT([found at $CLASSPATH_JAVA_LIBRARY_PATH])

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
AC_OUTPUT
