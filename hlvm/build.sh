#!/bin/sh
#
# build.sh
#
# This shell script runs the LLVM configure script and "make" with the standard 
# configuration arguments for building HLVM. How HLVM gets built depends on the
# the single argument passed to the script. The argument forms a "word" with 
# the letters "PESTADIO".  Each letter stands for a configuration option. 
# Capitalization means the feature is on, lower case means the feature 
# is off.  By default this script builds the pestADio configuration.
#
# The letters have the following meanings:
#
# FLAG		DEFAULT SETTING
# ###########   ############################################
# p = profiling (no)  -- build with profiling enabled
# e = efence	(no)  -- use Electric Fence for malloc/free
# s = small	(no)  -- favors speed over size
# t = trace	(no) -- code tracing is enabled
# A = assert	(yes) -- assertions are compiled in
# D = debug 	(yes) -- compiled with debug flags
# i = inline	(no)  -- all potential inlines are functions
# o = optimize	(no)  -- non-optimized build (easier debugging)
#
# In addition to the "PESTADIO" option, you can also pass additional 
# "configure" script options to this script. They will be passed on to 
# configure, as is.  If you'd rather have a different configuration of HLVM, 
# change any of the "--enable" flags to configure (see below) to indicate 
# "yes" you want the feature or "no" you don't want the feature.
#
# The first time this script is run, it will ask you for general configuration
# information, mostly relating to locations of software in your file system.
# This information is then stored in config.opts. Subsequent runs of build.sh
# will load the values from config.opts and use those instead. Note that you
# only need to run build.sh when you want to *change* or *initialize* the
# configuration of HLVM. Subsequent builds can be done using "make".
#

function getDirectory() 
{
  DIR=""
  if test -z "$2" ; then
    DEFAULT="<none>"
  else
    DEFAULT="$2"
  fi
  read -e -p "$1 ($DEFAULT): " DIR
  if test -z "$DIR" ; then
    echo "Defaulting to $DEFAULT"
    DIR="$DEFAULT"
  elif test \! -d "$DIR" ; then
    echo "The value entered is not a valid directory. Try again."
    getDirectory "$1" "$2"
  fi
}

function getBoolean()
{
  BOOL=""
  read -e -p "$1 " BOOL
  case "$BOOL" in
    [yY]*) BOOL="yes" ;;
    [nN]*) BOOL="no" ;;
    *)
      echo "Please enter 'yes' or 'no'. Try again."
      getBoolean "$1"
      ;;
  esac
}

if test -f config.opts ; then
  . config.opts
else
  getDirectory "Enter path to HLVM source directory" "../hlvm"
  HLVM_SRC_DIR="$DIR"

  getDirectory "Enter path where HLVM should be installed" "/usr/local/hlvm" 
  INSTALL_PREFIX="$DIR"

  getDirectory "Enter path to root of HLVM workspaces" "/usr/local/hlvm/wkspc"
  WORKSPACE="$DIR"

  getDirectory "Enter path to LLVM source root" "/proj/llvm/llvm" 
  LLVM_SRC="$DIR"

  getDirectory "Enter path to LLVM object root" "/proj/llvm/build" 
  LLVM_OBJ="$DIR"
  
  getDirectory "Enter path to expat libraries" "/proj/install/lib" 
  EXPAT_OBJ="$DIR"

  getDirectory "Enter path for additional header files" "/proj/install/include" 
  HEADERS="$DIR"

  getBoolean "Do you want to build verbosely (lots of output)?"
  if test "$BOOL" = "yes" ; then
    VERB="VERBOSE=1"
  else
    VERB=""
  fi

  getBoolean "Do you want to enable profiling?" 
  enable_PROFILING="$BOOL"
  getBoolean "Do you want to enable Electric Fence memory protection?" 
  enable_EFENCE="$BOOL"
  getBoolean "Do you want to build a small version of HLVM?" 
  enable_SMALL="$BOOL"
  getBoolean "Do you want to build a tracing version of HLVM?" 
  enable_TRACE="$BOOL"
  getBoolean "Do you want to compile assertions into HLVM?" 
  enable_ASSERT="$BOOL"
  getBoolean "Do you want to build a debug version of HLVM?" 
  enable_DEBUG="$BOOL"
  getBoolean "Do you want to build with inline functions?" 
  enable_INLINE="$BOOL"
  getBoolean "Do you want to optimize generated code for a release?" 
  enable_OPTIMIZE="$BOOL"

  echo
  echo "You have entered the following information:"
  echo "HLVM_SRC_DIR=$HLVM_SRC_DIR"
  echo "INSTALL_PREFIX=$INSTALL_PREFIX"
  echo "WORKSPACE=$INSTALL_PREFIX"
  echo "LLVM_SRC=$LLVM_SRC"
  echo "LLVM_OBJ=$LLVM_OBJ"
  echo "EXPAT_OBJ=$EXPAT_OBJ"
  echo "HEADERS=$HEADERS"
  echo "LIBRARIES=$LIBRARIES"
  echo "PROGRAMS=$PROGRAMS"
  echo "enable_PROFILING=$enable_PROFILING"
  echo "enable_EFENCE=$enable_EFENCE"
  echo "enable_SMALL=$enable_SMALL"
  echo "enable_TRACE=$enable_TRACE"
  echo "enable_ASSERT=$enable_ASSERT"
  echo "enable_DEBUG=$enable_DEBUG"
  echo "enable_INLINE=$enable_INLINE"
  echo "enable_OPTIMIZE=$enable_OPTIMIZE"

  echo
  getBoolean "Is this what you want?"
  if test "$BOOL" = "no" ; then
    exit
  fi

  echo "# HLVM Configuration Options - automatically generated" > config.opts
  echo "HLVM_SRC_DIR=$HLVM_SRC_DIR" >> config.opts
  echo "INSTALL_PREFIX=$INSTALL_PREFIX" >> config.opts
  echo "WORKSPACE=$INSTALL_PREFIX" >> config.opts
  echo "LLVM_SRC=$LLVM_SRC" >> config.opts
  echo "LLVM_OBJ=$LLVM_OBJ" >> config.opts
  echo "EXPAT_OBJ=$EXPAT_OBJ" >> config.opts
  echo "HEADERS=$HEADERS" >> config.opts
  echo "LIBRARIES=$LIBRARIES" >> config.opts
  echo "PROGRAMS=$PROGRAMS" >> config.opts
  echo "enable_PROFILING=$enable_PROFILING" >> config.opts
  echo "enable_EFENCE=$enable_EFENCE" >> config.opts
  echo "enable_SMALL=$enable_SMALL" >> config.opts
  echo "enable_TRACE=$enable_TRACE" >> config.opts
  echo "enable_ASSERT=$enable_ASSERT" >> config.opts
  echo "enable_DEBUG=$enable_DEBUG" >> config.opts
  echo "enable_INLINE=$enable_INLINE" >> config.opts
  echo "enable_OPTIMIZE=$enable_OPTIMIZE" >> config.opts
fi

# In its default state, the script assumes you want to install into /usr/local 
# and that HLVM software is in ../xps-<version>. You can change these 
# assumptions by changing the INSTALL_PREFIX and SOURCE_DIR variables below to 
# suit your needs. 
#

# Set up defaults

# If a parameter was set
if  [ -n "$1" ] ; then
  if [ "${1:0:1}" != '-' ] ; then
    CONFIG_NAME=$1
    shift
    case $CONFIG_NAME  in
      (dbg) CONFIG_NAME="pestADio" ;;
      (opt) CONFIG_NAME="pestADiO" ;;
      (prf) CONFIG_NAME="PestadIO" ;;
      (rls) CONFIG_NAME="pemdatIO" ;;
    esac
  fi 
fi

# While there are no more configuration items to process
while [ -n "$CONFIG_NAME"  ] ; do
  OPTION=${CONFIG_NAME:0:1}
  CONFIG_NAME=${CONFIG_NAME:1}
  case $OPTION in
    (P) enable_PROFILING="yes" ;;
    (p) enable_profiling="no" ;;
    (E) enable_EFENCE="yes" ;;
    (e) enable_EFENCE="no" ;;
    (D) enable_DEBUG="yes" ;;
    (d) enable_DEBUG="no" ;;
    (A) enable_ASSERT="yes" ;;
    (a) enable_ASSERT="no" ;;
    (T) enable_TRACE="yes" ;;
    (t) enable_TRACE="no" ;;
    (I) enable_INLINE="yes" ;;
    (i) enable_INLINE="no" ;;
    (O) enable_OPTIMIZE="yes" ;;
    (o) enable_OPTIMIZE="no" ;;
    (S) enable_SMALL="yes" ;;
    (s) enable_SMALL="no" ;;
    (*) echo "Invalid configuration letter: $OPTION" ; exit ;;
  esac
done

set -x 
LD_LIBRARY_PATH="${LIBRARIES}:${LD_LIBRARY_PATH}" PATH="${PROGRAMS}:${PATH}" \
CPPFLAGS="-I${HEADERS}" ${HLVM_SRC_DIR}/configure \
    --prefix="${INSTALL_PREFIX}" \
    --srcdir=${HLVM_SRC_DIR} \
    --enable-maintainer-mode=no \
    --enable-debug=${enable_DEBUG} \
    --enable-assert=${enable_ASSERT} \
    --enable-trace=${enable_TRACE} \
    --enable-inline=${enable_INLINE} \
    --enable-optimize=${enable_OPTIMIZE} \
    --enable-small=${enable_SMALL} \
    --enable-efence=${enable_EFENCE} \
    --enable-profiling=${enable_PROFILING} \
    --with-expat=${EXPAT_OBJ} \
    --with-llvm-src=${LLVM_SRC} \
    --with-llvm-obj=${LLVM_OBJ} \
    --with-workspace=${WORKSPACE} \
    $* && \
    make $VERB && \
    make $VERB check && \
    make $VERB install 
