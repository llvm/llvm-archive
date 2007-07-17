#!/bin/sh

# This includes the Bourne shell library from llvm-top. Since this file is
# generally only used when building from llvm-top, it is safe to assume that
# llvm is checked out into llvm-top in which case .. just works.
. ../library.sh

is_debug=1
for arg in "$@" ; do
  case "$arg" in
    LLVM_TOP=*)
      LLVM_TOP=`echo "$arg" | sed -e 's/LLVM_TOP=//'`
      ;;
    PREFIX=*)
      PREFIX=`echo "$arg" | sed -e 's/PREFIX=//'`
      ;;
    *=*)
      build_opts="$build_opts $arg"
      ;;
    --*)
      config_opts="$config_opts $arg"
      ;;
  esac
done

# See if we have previously been configured by sensing the presense
# of the config.status scripts
if test ! -x "config.status" ; then
  # We must configure so build a list of configure options
  config_options="--prefix=$PREFIX --with-llvm-top=$LLVM_TOP "
  config_options="$config_options $config_opts"
  msg 0 Configuring $module with:
  msg 0 "  ./configure" $config_options
  ./configure $config_options || (echo "Can't configure llvm" ; exit 1)
fi

msg 0 Building $module with:
msg 0 "  make" $build_opts tools-only
make $build_opts tools-only
