README FOR llvm-top MODULE
==========================

You are reading the README for the llvm-top module of the LLVM project. This
module should be the first thing you check out when accessing the LLVM
project's subversion repository. From here all other modules are accessible
via the makefile.

You should check this module out with the following subversion command:

   svn co http://llvm.org/svn/llvm-project/llvm-top/trunk llvm-top

Once you've done that, you can then use the following "make" targets to check
out the various modules of the project:

make checkout MODULE=llvm       - Checks out the llvm (core) module
make checkout MODULE=llvm-gcc   - Checks out the GCC based C/C++/Obj-C compiler
make checkout MODULE=test-suite - Checks out the llvm test suite
make checkout MODULE=stacker    - Checks out the stacker front end
make checkout MODULE=hlvm       - Checks out the High Level Virtual Machine

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
