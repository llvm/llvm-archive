#!/bin/sh
die () {
	echo "$@" 1>&2
	exit 1
}
test -d autoconf && test -f autoconf/configure.ac && cd autoconf
test -f configure.ac || die "Can't find 'autoconf' dir; please cd into it first"
autoconf --version | egrep '2\.5[0-9]' > /dev/null
if test $? -ne 0 ; then
  die "Your autoconf was not detected as being 2.5x"
fi
cwd=`pwd`
echo "Regenerating aclocal.m4 with aclocal"
rm -f aclocal.m4
aclocal -I $cwd/m4 || die "aclocal failed"
echo "Regenerating configure with autoconf 2.5x"
autoconf --warnings=all -o ../configure configure.ac || die "autoconf failed"
cd ..
exit 0
