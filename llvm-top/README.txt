README FOR llvm-top MODULE
==========================

You are reading the README for the llvm-top module of the LLVM project. This
module should be the first thing you check out when accessing the LLVM
project's subversion repository. From here all other modules are accessible
via the makefile.

You should check this module out with the following subversion command:

   svn co http://llvm.org/svn/llvm-project/llvm-top/trunk llvm-top

Once you've done that, you can then check out a module (and all its
dependencies) with the 'make checkout MODULE=modulename' command.  For example:

  make checkout MODULE=llvm-gcc-4.0

Checks out llvm-gcc-4.0 (the llvm C/C++/ObjC compiler built with the GCC 4.0
front-end) and all the things it depends on.  Other modules available are:

  test-suite - The llvm test suite
  stacker    - The stacker front end (a 'Forth-like' language)
  hlvm       - High Level Virtual Machine

You can check out any number of modules.


Once you check out the modules, use the "make" command to automatically
configure and build the projects you checked out.

...


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
