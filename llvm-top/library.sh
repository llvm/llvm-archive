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
INSTALL_PREFIX="$LLVM_TOP/install"

# A command to figure out the root of the SVN repository by asking for it from
# the 'svn info' command. To use, execute it in a script with something like
SVNROOT=`$SVN info . | grep 'pository Root:' | sed -e 's/^Repository Root: //'`

# Set this to true (after sourcing this library) if you want verbose
# output from the library
VERBOSE=0

# Generate an informative message to the user based on the verbosity level
msg() {
  level=$1
  shift
  if test "$level" -le "$VERBOSE" ; then
    echo "INFO-$level: $*"
  fi
}

# Die with an error message
die() {
  EXIT_CODE=$1
  shift
  echo "ERROR-$EXIT_CODE: $*"
  exit $EXIT_CODE
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
  if test "$module" == "llvm-gcc-4.0" ; then
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
         die $? "get_dependencies failed"
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

process_builder_args() {
  for arg in "$@" ; do
    case "$arg" in
      LLVM_TOP=*) LLVM_TOP=`echo "$arg" | sed -e 's/LLVM_TOP=//'` ;;
      PREFIX=*)   PREFIX=`echo "$arg" | sed -e 's/PREFIX=//'` ;;
      MODULE=*)   MODULE=`echo "$arg" | sed -e 's/MODULE=//'` ;;
      ENABLE_PIC=*) 
        ENABLE_PIC=`echo "$arg" | sed -e 's/ENABLE_PIC=//'`
        OPTIONS_ASSIGN="$OPTIONS_ASSIGN $arg" 
        ;;
      ENABLE_OPTIMIZED=*) 
        ENABLE_OPTIMIZED=`echo "$arg" | sed -e 's/ENABLE_OPTIMIZED=//'`
        OPTIONS_ASSIGN="$OPTIONS_ASSIGN $arg" 
        ;;
      ENABLE_PROFILING=*) 
        ENABLE_PROFILING=`echo "$arg" | sed -e 's/ENABLE_PROFILING=//'`
        OPTIONS_ASSIGN="$OPTIONS_ASSIGN $arg" 
        ;;
      ENABLE_EXPENSIVE_CHECKS=*) 
        ENABLE_EXPENSIVE_CHECKS=`echo "$arg" | sed -e 's/ENABLE_EXPENSIVE_CHECKS=//'`
        OPTIONS_ASSIGN="$OPTIONS_ASSIGN $arg" 
        ;;
      --*) OPTIONS_DASH_DASH="$OPTIONS_DASH_DASH $arg" ;;
       -*) OPTIONS_DASH="$OPTIONS_DASH $arg" ;;
      *=*) OPTIONS_ASSIGN="$OPTIONS_ASSIGN $arg" ;;
        *) die 1 "Unrecognized option: $arg" ;;
    esac
  done
}
