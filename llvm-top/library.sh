#                        llvm-top common script
# 
# This file was developed by Reid Spencer and is distributed under the
# University of Illinois Open Source License. See LICENSE.TXT for details.
# 
#===------------------------------------------------------------------------===#

# This script provides the script fragments and functions that are common to
# the scripts in the llvm-top module.

# The arguments to all scripts are
# Define where subversion is. We assume by default its in the path.
SVN=svn

# A command to figure out the root of the SVN repository by asking for it from
# the 'svn info' command. To use, execute it in a script with something like
SVNROOT=`$SVN info . | grep 'pository Root:' | sed -e 's/^Repository Root: //'`

# Set this to true (after sourcing this library) if you want verbose
# output from the library
VERBOSE=0

# Rule to get the modules that $(MODULE) depends on.
#MODULEINFO = $(MODULE)/ModuleInfo.txt
#DEPMODULES = grep -i DepModule: $(MODULEINFO) | sed 's/DepModule: *//g'
#BUILDTARGET = grep -i BuildTarget: $(MODULEINFO) | sed 's/BuildTarget: *//g'
#CONFIGTARGET = grep -i ConfigTarget: $(MODULEINFO) | sed 's/ConfigTarget: *//g'

# Figure out the root of the SVN repository by asking for it from 'svn info'
#SVNROOT = $(shell $(SVN) info . | grep 'Repository Root:' | \
#                   sed -e 's/^Repository Root: //')


# Check out a module and all its dependencies. Note that this arrangement
# depends on each module having a file named ModuleInfo.txt that explicitly
# indicates the other LLVM modules it depends on. See one of those files for
# examples.

msg() {
  level=$1
  shift
  if test "$level" -le "$VERBOSE" ; then
    echo "INFO-$level: $*"
  fi
}

die() {
  EXIT_CODE=$1
  shift
  echo "ERROR-$EXIT_CODE: $*"
  exit $EXIT_CODE
}

checkout() {
  module=$1
  msg 1 "Checking out module $module"
  $SVN checkout $SVNROOT/$module/trunk $module || \
    die $? "Checkout of module $module failed."
  return 0
}

get_dependencies() {
  for module in $* ; do
    if test ! -d "$module" ; then
      checkout "$module" || die $? "Checkout failed."
    fi
    mi="$module/ModuleInfo.txt"
    dep_modules=""
    if test -f "$mi" ; then
      dep_modules=`grep -i DepModule: $mi | sed 's/DepModule: *//g'`
      if test "$?" -ne 0 ; then 
        die $? "Searching file '$mi' failed."
      fi
    fi
    if test ! -z "$dep_modules" ; then
      msg 1 "Module '$module' depends on $dep_modules"
      deps=`get_dependencies $dep_modules` || die $? "get_dependencies failed"
      for dep in $dep_modules ; do
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
