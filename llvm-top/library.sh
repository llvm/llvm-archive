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
  local level=$1
  shift
  if test "$level" -le "$VERBOSE" ; then
    echo "TOP-$level: $*"
  fi
}

# Die with an error message
die() {
  local code=$1
  shift
  echo "TOP-$code: Error: $*"
  exit $code
}

process_arguments() {
  msg 2 "Processing arguments"
  local arg=""
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
    local modinfo=""
    for modinfo in */ModuleInfo.txt ; do
      mod=`dirname $modinfo`
      mod=`basename $modinfo`
      MODULES="$mod $MODULES"
    done
  fi
}

# Check out a single module. 
checkout_a_module() {
  local module=$1
  msg 1 "Checking out module '$module'"
  if test -d "$module" ; then
    msg 2 "Module '$module' is already checked out."
    return 0
  fi
  if test -e "$module" ; then
    die 2 "Module '$module' is not a directory!"
    return 0
  fi
  local quiet=""
  if test "$VERBOSE" -le 1 ; then
    quiet="-q"
  fi
  msg 3 "Running svn checkout for '$module'"
  if test "$module" = "llvm-gcc-4.0" ; then
    $SVN checkout $quiet svn://anonsvn.opensource.apple.com/svn/llvm/trunk \
      $module || die $? "Checkout of module $module failed."
  else
    $SVN checkout $quiet $SVNROOT/$module/trunk $module || \
      die $? "Checkout of module '$module' failed."
  fi
  return 0
}

# This function extracts a module info item from a ModuleInfo.txt file. If
# the module isn't already checked out, then it gets checked out. The value
# of the info item is returned in the MODULE_INFO_VALUE variable.
get_module_info() {
  local module="$1"
  local item_name="$2"
  msg 2 "Getting '$item_name' module info for '$module'"
  if test ! -d "$module" ; then
    checkout_a_module "$module" || die $? "Checkout failed."
  fi
  local module_info="$module/ModuleInfo.txt"
  if test -f "$module_info" ; then
    local item_value=`grep -i "$item_name:" $module_info | \
                sed -e "s/$item_name: *//g"`
    if test "$?" -ne 0 ; then 
      die $? "Searching file '$module_info for $item_name' failed."
    fi
  else
    msg 1 "Module $module has no ModuleInfo.txt file (ignored)."
  fi
  MODULE_INFO_VALUE="$item_value"
}

# This function gets the dependencies of all the dependent modules of the
# list of modules passed as arguments. If a module is not checked out, it will
# be checked out. This process repeats recursively until all the dependencies
# have been satisfied. Upon exit the MODULE_DEPENDENCIES variable contains
# the list of module names from least dependent to most dependent in the
# correct configure/build order.
add_module_if_not_duplicate() {
  local mod_to_add="$1"
  local mod=""
  for mod in $MODULE_DEPENDENCIES ; do
    local has_mod=`echo "$mod" | grep "$mod_to_add"`
    if test ! -z "$has_mod" ; then
      return 0
    fi
    msg 3 "Looping in add_module_if_not_duplicate"
  done
  msg 2 "Adding module '$mod_to_add' to list"
  MODULE_DEPENDENCIES="$MODULE_DEPENDENCIES $mod_to_add"
}

get_a_modules_dependencies() {
  if test "$RECURSION_DEPTH" -gt 10 ; then
    die 2 "Recursing too deeply on module dependencies"
  fi
  let RECURSION_DEPTH="$RECURSION_DEPTH + 1"
  local module="$1"
  msg 2 "Getting dependencies for module '$module'"
  get_module_info $module DepModule
  if test ! -z "$MODULE_INFO_VALUE" ; then
    local my_deps="$MODULE_INFO_VALUE"
    msg 2 "Module '$module' depends on '$my_deps'"
    local has_me=`echo "$my_deps" | grep "$module"`
    if test -z "$has_me" ; then
      local dep=""
      for dep in $my_deps ; do
        get_a_modules_dependencies "$dep"
        msg 3 "Looping in get_a_modules_dependencies"
      done
    else
      die 1 "Module '$module' has a dependency on itself!"
    fi
  fi
  add_module_if_not_duplicate "$1"
}

get_module_dependencies() {
  msg 2 "Getting module dependencies: $*"
  local module=""
  for module in "$@" ; do
    RECURSION_DEPTH=0
    get_a_modules_dependencies $module
    msg 3 "Looping in get_module_dependencies"
  done
}

build_a_module() {
  local module="$1"
  msg 1 "Building module '$module'"   
  get_module_info $module BuildCmd
  if test -z "$MODULE_INFO_VALUE" ; then
    msg 2 "Module $module has no BuildCmd entry so it will not be built."
    return 0
  fi
  local build_cmd="$MODULE_INFO_VALUE MODULE=$module $build_args"
  msg 1 "Building Module '$module'"
  msg 2 "Build Command: $build_cmd"
  cd $module
  $build_cmd || die $? "Build of module '$module' failed."
  cd $LLVM_TOP
}

