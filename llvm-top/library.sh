#!/bin/sh
#                   llvm-top common function library
# 
# This file was developed by Reid Spencer and is distributed under the
# University of Illinois Open Source License. See LICENSE.TXT for details.
# 
#===------------------------------------------------------------------------===#

# This script provides the script fragments and functions that are common to
# the scripts in the llvm-top module.

# The arguments to all scripts are
# Define where subversion is. We assume by default its in the path.
SVN=`which svn`

# Get the llvm-top directory before anyone has a chance to cd out of it
LLVM_TOP=`pwd`
PREFIX="$LLVM_TOP/installed"
DESTDIR=

# A command to figure out the root of the SVN repository by asking for it from
# the 'svn info' command. To use, execute it in a script with something like
SVNROOT=`$SVN info . | grep 'Repository Root:' |sed -e 's/^Repository Root: //'`

# Set this to non-zero (after sourcing this library) if you want verbose
# output from the library. The higher the value, the more output you get
VERBOSE=0

# Generate an informative message to the user based on the verbosity level
msg() {
  level=$1
  shift
  if test "$level" -le "$VERBOSE" ; then
    echo "TOP-$level: $*"
  fi
}

# Die with an error message
die() {
  EXIT_CODE=$1
  shift
  echo "TOP-$EXIT_CODE: Error: $*"
  exit $EXIT_CODE
}

process_arguments() {
  for arg in "$@" ; do
    case "$arg" in
      LLVM_TOP=*) LLVM_TOP=`echo "$arg" | sed -e 's/LLVM_TOP=//'` ;;
      PREFIX=*)   PREFIX=`echo "$arg" | sed -e 's/PREFIX=//'` ;;
      DESTDIR=*)  DESTDIR=`echo "$arg" | sed -e 's/DESTDIR=//'` ;;
      MODULE=*)   MODULE=`echo "$arg" | sed -e 's/MODULE=//'` ;;
      VERBOSE=*)  VERBOSE=`echo "$arg" | sed -e 's/VERBOSE=//'` ;;
      --*)        OPTIONS_DASH_DASH="$OPTIONS_DASH_DASH $arg" ;;
       -*)        OPTIONS_DASH="$OPTIONS_DASH $arg" ;;
      *=*)        OPTIONS_ASSIGN="$OPTIONS_ASSIGN $arg" ;;
      [a-zA-Z]*)  MODULES="$MODULES $arg" ;;
        *)        die 1 "Unrecognized option: $arg" ;;
    esac
  done

  # An empty modules list means "the checked out modules" so gather that list
  # now if we didn't get any modules on the command line.
  if test -z "$MODULES" ; then
    MODULES=""
    for modinfo in */ModuleInfo.txt ; do
      mod=`dirname $modinfo`
      mod=`basename $modinfo`
      MODULES="$mod $MODULES"
    done
  fi
}

# Check out a single module. 
checkout_a_module() {
  module=$1
  if test -d "$module" ; then
    msg 0 "Module $module is already checked out."
    return 0
  fi
  if test -e "$module" ; then
    msg 0 "Module $module is not a directory! Checkout skipped."
    return 0
  fi
  msg 1 "Checking out module $module"
  if test "$module" = "llvm-gcc-4.0" ; then
    $SVN checkout svn://anonsvn.opensource.apple.com/svn/llvm/trunk $module ||
      die $? "Checkout of module $module failed."
  else
    $SVN checkout $SVNROOT/$module/trunk $module || \
      die $? "Checkout of module $module failed."
  fi
  return 0
}

# This function extracts a module info item from a ModuleInfo.txt file. If
# the module isn't already checked out, then it gets checked out. The value
# of the info item is returned in the MODULE_INFO_VALUE variable.
get_module_info() {
  module="$1"
  item_name="$2"
  if test ! -d "$module" ; then
    checkout_a_module "$module" || die $? "Checkout failed."
  fi
  module_info="$module/ModuleInfo.txt"
  if test -f "$module_info" ; then
    item_value=`grep -i "$item_name:" $module_info | \
                sed -e "s/$item_name: *//g"`
    if test "$?" -ne 0 ; then 
      die $? "Searching file '$module_info for $item_name' failed."
    fi
  else
    msg 0 "Module $module has no ModuleInfo.txt file (ignored)."
  fi
  MODULE_INFO_VALUE="$item_value"
}

# This function gets the dependencies of all the dependent modules of the
# list of modules passed as arguments. If a module is not checked out, it will
# be checked out. This process repeats recursively until all the dependencies
# have been satisfied. Upon exit the MODULE_DEPENDENCIES variable contains
# the list of module names from least dependent to most dependent in the
# correct configure/build order.
get_module_dependencies() {
  for module in $* ; do
    get_module_info $module DepModule
    if test ! -z "$MODULE_INFO_VALUE" ; then
      msg 1 "Module '$module' depends on $MODULE_INFO_VALUE"
      deps=`get_module_dependencies $MODULE_INFO_VALUE` || \
         die $? "get_module_dependencies failed"
      for dep in $MODULE_INFO_VALUE ; do
        matching=`echo "$MODULE_DEPENDENCIES" | grep "$dep"`
        if test -z "$matching" ; then
          MODULE_DEPENDENCIES="$MODULE_DEPENDENCIES $dep"
        fi
      done
    fi
    matching=`echo "$MODULE_DEPENDENCIES" | grep "$module"`
    if test -z "$matching" ; then
      MODULE_DEPENDENCIES="$MODULE_DEPENDENCIES $module"
    fi
  done
  return 0
}

build_a_module() {
  module="$1"
  msg 2 "Getting module info for $module"
  get_module_info $module BuildCmd
  if test -z "$MODULE_INFO_VALUE" ; then
    msg 0 "Module $module has no BuildCmd entry so it will not be built."
    return 0
  fi
  build_cmd="$MODULE_INFO_VALUE MODULE=$module $build_args"
  msg 1 "Building Module $module with this command:"
  msg 1 "  $build_cmd"
  cd $module
  $build_cmd || die $? "Build of $module failed."
  cd $LLVM_TOP
}

