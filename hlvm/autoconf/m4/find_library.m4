dnl Avoid duplicating INCLUDE and LDFLAG settings with this macro
dnl Parameters:
dnl  $1 - Variable to set
dnl  $2 - Value to add, if not present
AC_DEFUN([HLVM_ADD_TO_VAR],[
 echo ["$]$1["] | grep -- "$2" >/dev/null 2>/dev/null
 if test $? -ne 0 ; then
   $1=["$]$1[ ]$2["]
 fi
])
dnl Check for a package and its libraries and include files
dnl 
dnl Parameters:
dnl   $1 - prefix directory to check
dnl   $2 - package name
dnl   $3 - header file to check 
AC_DEFUN([HLVM_CHECK_INCLUDE],[
hlvm_found_inc=0
if test -d "$1" ; then
  if test -n "$3" ; then
    for dir in "$1/include" "$1" "$1/include/$2" ; do
      if test -d "$dir" ; then
        if test -f "$dir/$3" ; then
          AC_SUBST($2[_INC],["$dir"])
          hlvm_found_inc=1
        fi
      fi
    done
  fi
fi
])
dnl Parameters:
dnl   $1 - prefix directory to check
dnl   $2 - package name
dnl   $3 - library file to check
AC_DEFUN([HLVM_CHECK_LIBRARY],[
hlvm_found_lib=0
if test -d "$1" ; then
  if test -n "$3" ; then
    for dir in "$1" "$1/lib" ; do
      if test -d "$dir" ; then
        if test -f "$dir/lib$3.so" ; then
          AC_SUBST($2[_LIB],[$dir])
          hlvm_found_lib=1
        elif test -f "$dir/lib$3.a" ; then
          AC_SUBST($2[_LIB],[$dir])
          hlvm_found_lib=1
        elif test -f "$dir/lib$3.la" ; then
          AC_SUBST($2[_LIB],[$dir])
          hlvm_found_lib=1
        fi
      fi
    done
  fi
fi
])

dnl Find a program via --with options, in the path, or well known places
dnl
dnl Parameters:
dnl   $1 - short name for the package
dnl   $2 - header file name to check (optional)
dnl   $3 - library file name to check (optional)
dnl   $4 - library symbol to test
dnl   $5 - alternate (long) name for the program
AC_DEFUN([HLVM_FIND_LIBRARY],
[m4_define([allcapsname],translit($1,a-z,A-Z))
AC_ARG_WITH(allcapsname(),
  AS_HELP_STRING([--with-]allcapsname()[=DIR],
  [Specify that the ]$5[ install prefix is DIR]),
  $1[pfxdir=$withval],$1[pfxdir=nada])
AC_ARG_WITH(allcapsname()[-lib],
  AS_HELP_STRING([--with-]allcapsname()[-lib=DIR],
  [Specify that ]$5[ libraries are in DIR]),
  $1[libdir=$withval],$1[libdir=nada])
AC_ARG_WITH(allcapsname()[-inc],
  AS_HELP_STRING([--with-]allcapsname()[-inc=DIR],
  [Specify that the ]$5[ includes are in DIR]),
  $1[incdir=$withval],$1[incdir=nada])
pfxval="${]$1[pfxdir}"
incval="${]$1[incdir}"
libval="${]$1[libdir}"
AC_MSG_CHECKING([for ]$5[ library and header])
hlvm_found_lib=0
hlvm_found_inc=0
if test "${pfxval}" != "nada" ; then
  if test -d "${pfxval}" ; then
    HLVM_CHECK_INCLUDE(${pfxval},$1,$2)
    HLVM_CHECK_LIBRARY(${pfxval},$1,$3)
  else
    AC_MSG_RESULT([failed]);
    AC_MSG_ERROR([The --with-]$1[ value must be a directory])
  fi
else 
  if test "${libval}" != "nada" ; then
    HLVM_CHECK_LIBRARY(${libval},$1,$3)
  fi
  if test "${incval}" != "nada" ; then
    HLVM_CHECK_INCLUDE(${incval},$1,$2)
  fi
fi
if test "$hlvm_found_lib" != 1 ; then
  HLVM_CHECK_LIBRARY([/usr],$1,$3)
fi
if test "$hlvm_found_lib" != 1 ; then
  HLVM_CHECK_LIBRARY([/usr/local],$1,$3)
fi
if test "$hlvm_found_lib" != 1 ; then
  HLVM_CHECK_LIBRARY([/proj/install],$1,$3)
fi
if test "$hlvm_found_lib" != 1 ; then
  HLVM_CHECK_LIBRARY([/sw],$1,$3)
fi
if test "$hlvm_found_lib" != 1 ; then
  HLVM_CHECK_LIBRARY([/opt],$1,$3)
fi
if test "$hlvm_found_inc" != 1 ; then
  HLVM_CHECK_INCLUDE([/usr],$1,$2)
fi
if test "$hlvm_found_inc" != 1 ; then
  HLVM_CHECK_INCLUDE([/usr/local],$1,$2)
fi
if test "$hlvm_found_inc" != 1 ; then
  HLVM_CHECK_INCLUDE([/proj/install],$1,$2)
fi
if test "$hlvm_found_inc" != 1 ; then
  HLVM_CHECK_INCLUDE([/sw],$1,$2)
fi
if test "$hlvm_found_inc" != 1 ; then
  HLVM_CHECK_INCLUDE([/opt],$1,$2)
fi
if test "$hlvm_found_inc" == 1 ; then
  if test "$hlvm_found_lib" == 1 ; then
    AC_MSG_RESULT([found])
    incdir=[${]$1[_INC}]
    libdir=[${]$1[_LIB}]
    HLVM_ADD_TO_VAR(LDFLAGS,-L"${libdir}")
    AC_SEARCH_LIBS($4,$3,[found_sym=1],[found_sym=0])
    if test "$found_sym" == "1" ; then
      AC_MSG_NOTICE([Found $1 in ${incdir} and ${libdir}])
      HLVM_ADD_TO_VAR(CPPFLAGS,-I"${incdir}")
    else
      AC_MSG_NOTICE([Symbol ']$4[' not found in library $3 in ${libdir}])
    fi
  else
    AC_MSG_RESULT([failed to find library file])
    AC_MSG_ERROR([The --with-]allcapsname()[-lib option must be used])
  fi
else
  AC_MSG_RESULT([failed to find header file])
  AC_MSG_ERROR([The --with-]allcapsname()[-inc option must be used])
fi
])
