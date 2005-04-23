#!/bin/sh
die () {
	echo "$@" 1>&2
	exit 1
}
test -d autoconf && test -f autoconf/configure.ac && cd autoconf
test -f configure.ac || die "Can't find 'autoconf' dir; please cd into it first"
autoconf --version | egrep '2\.59' > /dev/null
if test $? -ne 0 ; then
  die "Your autoconf was not detected as being 2.59"
fi
aclocal-1.9 --version | egrep '1\.9\.4' > /dev/null
if test $? -ne 0 ; then
  die "Your aclocal was not detected as being 1.9.4"
fi
autoheader --version | egrep '2\.59' > /dev/null
if test $? -ne 0 ; then
  die "Your autoheader was not detected as being 2.59"
fi
libtool --version | grep '1.5.14' > /dev/null
if test $? -ne 0 ; then
  die "Your libtool was not detected as being 1.5.14"
fi
echo ""
echo "### NOTE: ############################################################"
echo "### If you get *any* warnings from autoconf below other than warnings"
echo "### about 'AC_CONFIG_SUBDIRS: you should use literals', you MUST fix"
echo "### the scripts in the m4 directory because there are future forward"
echo "### compatibility or platform support issues at risk. Please do NOT"
echo "### commit any configure.ac or configure script that was generated "
echo "### with warnings present."
echo "######################################################################"
echo ""
echo "Regenerating aclocal.m4 with aclocal"
save=`pwd`
cd ../../..
m4dir=`pwd`/autoconf/m4
cd $save
aclocal --force -I $m4dir || die "aclocal failed"
echo "Regenerating configure with autoconf 2.5x"
autoconf --force --warnings=all -o ../configure configure.ac || die "autoconf failed"
