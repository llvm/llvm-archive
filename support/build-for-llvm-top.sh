#!/bin/sh

# This includes the Bourne shell library from llvm-top. Since this file is
# generally only used when building from llvm-top, it is safe to assume that
# llvm is checked out into llvm-top in which case .. just works.
. $LLVM_TOP/library.sh

# Call the library function to process the arguments
process_builder_args "$@"

# See if we have previously been configured by sensing the presense
# of the config.status scripts
if test ! -x "config.status" ; then
  # We must configure so build a list of configure options
  config_options="--prefix=$PREFIX --with-llvm-top=$LLVM_TOP "
  config_options="$config_options $OPTIONS_DASH_DASH"
  msg 0 Configuring $module with:
  msg 0 "  ./configure" $config_options
  ./configure $config_options || (echo "Can't configure llvm" ; exit 1)
fi

msg 0 Building $MODULE with:
msg 0 "  make" $build_opts
make $build_opts
