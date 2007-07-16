README FOR llvm-top MODULE
==========================

You are reading the README for the llvm-top module of the LLVM project. This
module should be the first thing you check out when accessing the LLVM
project's subversion repository. From here all other modules are accessible
via the makefile.

You should check this module out with the following subversion command:

   svn co http://llvm.org/svn/llvm-project/llvm-top/trunk llvm-top

Once you've done that, you can then check out a module (and all its
dependencies) with the get script located here. For example:

  ./get llvm-gcc-4-0

which will check out both llvm and llvm-gcc-4-0 because the latter depends on
the former.

Similarly you can build any module just by using the build script located
here. For example:

  ./build llvm-gcc-4-0 ENABLE_OPTIMIZED=1 PREFIX=/my/install/dir VERBOSE=1

As you might guess, this will check out both llvm and llvm-gcc-4-0 if they
haven't been and then configure and build them according to the arguments 
that you specified. 

The modules available are:

  llvm-top     - This directory
  llvm         - The core llvm software
  llvm-gcc-4-0 - The C/C++/Obj-C front end for llvm, based on GCC 4.0
  llvm-gcc-4-2 - The C/C++/Obj-C front end for llvm, based on GCC 4.2
  test-suite   - The llvm test suite
  stacker      - The stacker front end (a 'Forth-like' language)
  hlvm         - High Level Virtual Machine (nascent)
  java         - Java Front End (unfinished, out of date)

You can check out any number of modules using the "get" script.

-----------------------------------------------------------------------------

Some Other Useful URLS
======================

Please use the following URLs to discover more about the LLVM project and its
software modules. You can copy and paste these URLs into your browser.

  * http://llvm.org/
    Main web site for the project with access to each module's documentation.

  * http://llvm.org/docs/ (http://llvm.org/svn/llvm-project/llvm/trunk/docs/)
    Documentation for the main llvm sub-project.

  * http://llvm.org/svn/
    Browse the latest revision of the source code in plain text (no frills).

  * http://llvm.org/viewvc/llvm-project/
    Browse any revision of the source code with lots of frills provided by
    ViewVC.
