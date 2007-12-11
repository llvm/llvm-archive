dnl Make it easier to use the AC_ARG_ENABLE macro for certain numeric switches
dnl that return levels of support as integer values. The arguments are:
dnl   1 - feature name
dnl   2 - feature description
dnl   3 - default value
dnl   4 - min value (default 0)
dnl   5 - max value (default 100)
AC_DEFUN([LLVM_WANT_LEVEL],[
  m4_define([allcapsname],translit($1,a-z-,A-Z_))
  AC_ARG_ENABLE([$1],
    AS_HELP_STRING([--enable-$1],[$2 ($3)]),,enableval="$3")
  case "$enableval" in
    yes) enableval="1" ;;
    no)  enableval="0" ;;
    [0-9]*) ;;
    *)   enableval="0" ;;
  esac
  digits=`echo "$enableval" | sed 's/[^0-9]//'`
  if test -z "$digits" ; then
    AC_MSG_ERROR([Expected numeric value for --enable-$1.])
  fi
  min="$4"
  max="$5"
  if test -z "$min" ; then min="0" ; fi
  if test -z "$max" ; then max="100" ; fi
  if test "$enableval" -lt "$min" ; then
    AC_MSG_ERROR(
      [Value for --enable-$1 ($enableval) is less than minimum ($min)])
  fi
  if test "$enableval" -gt "$max" ; then
    AC_MSG_ERROR(
      [Value for --enable-$1 ($enableval) is greater than maximum ($max)])
  fi
  AC_SUBST([WANT_]allcapsname(),$enableval)
  want_var=[WANT_]allcapsname()
  AC_DEFINE_UNQUOTED($want_var,$enableval,[$2])
])
