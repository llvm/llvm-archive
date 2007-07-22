#!/bin/sh

# This includes the Bourne shell library from llvm-top. Since this file is
# generally only used when building from llvm-top, it is safe to assume that
# llvm is checked out into llvm-top in which case .. just works.
if test -z "$LLVM_TOP" ; then
  . ../library.sh
else
  . "$LLVM_TOP/library.sh"
fi

# Call the library function to process the arguments
process_arguments "$@"

function add_config_option() {
  local name="$1"
  lc=`echo ${name} | tr 'A-Z_' 'a-z-'`
  if test -z "${!name}" -o "${!name}" -eq 0 ; then
    config_options="$config_options --disable-$lc"
  else
    config_options="$config_options --enable-$lc"
  fi
}

# See if we have previously been configured by sensing the presense
# of the config.status scripts
if test ! -x "config.status" ; then
  # We must configure, so build a list of configure options
  config_options="--config-cache --prefix=$PREFIX --with-llvm-top=$LLVM_TOP"
  config_options="$config_options --with-destdir='$DESTDIR'"
  config_options="$config_options $OPTIONS_DASH_DASH"
  add_config_option ASSERTIONS 
  add_config_option CHECKING
  add_config_option DEBUG 
  add_config_option DOXYGEN
  add_config_option OPTIMIZED
  add_config_option OPT_FOR_SIZE
  add_config_option PROFILING
  add_config_option STRIPPED
  add_config_option THREADS
  add_config_option VERBOSE
  msg 0 Configuring $module with:
  msg 0 "  ./configure" $config_options
  if test "$VERBOSE" -eq 0 ; then
    config_options="$config_options --quiet"
  fi
  ./configure $config_options || (echo "Can't configure llvm" ; exit 1)
fi

msg 0 Building $MODULE with:
msg 0 "  make" $build_opts
make $build_opts
