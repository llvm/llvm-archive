README FOR llvm-top MODULE
==========================

You are reading the README for the llvm-top module of the LLVM project. This
module should be the first thing you check out when accessing the LLVM
project's subversion repository. From here all other modules are accessible
via small scripts.

You should check this module out with the following subversion command:

   svn co http://llvm.org/svn/llvm-project/llvm-top/trunk llvm-top

Alternatively, if you have commit access, use this form:

   svn co https://username@llvm.org/svn/llvm-project/llvm-top/trunk llvm-top

Once you've checked out llvm-top, you can then check out a module (and all its
dependencies) with the "get" script located here. For example:

  ./get llvm-gcc-4-0

which will check out both llvm and llvm-gcc-4-0 because the latter depends on
the former. 


In addition to checking out software, there are several more scripts in 
llvm-top.  In all the scripts, the dependency checking behavior is the same as
for the get script. That is, the script operates on the modules you ask for as
well as all the modules they depend on.

The scripts available are:

  get      - check out modules and their dependencies from subversion
  info     - get subversion information about one or more modules
  update   - update one or more modules

  build    - configure, compile and link one or more modules
  install  - install one or more modules (presumes build already done)
  clean    - clean (remove build products) one or more modules

The first three scripts just work with subversion.  The last three scripts do 
not dictate how to build, install or clean the modules; that is up to the 
modules themselves.  The only thing these scripts depend on is a file named 
ModuleInfo.txt located in each module's top directory. This file can contain 
the following definitions:

  DepModule:  - the list of modules this module depends on 
  BuildCmd:   - a command to build (and configure) the module
  InstallCmd: - a command to install the module
  CleanCmd:   - a command to clean the module

The scripts in llvm-top determine dependencies based on the DepModule lines of
the various modules. This is how it knows which modules need to be checked out
and what order to build them in. The three definitions ending in "Cmd" specify
commands to be run. They are used by the build, install and clean scripts,
respectively. Modules are free to specify whatever command is appropriate to
build, install or clean that module.

Each of these scripts uses a common library of shell functions (library.sh) to
ensure their function is regular. In particular, they all accept the same set 
of arguments. The arguments recognized are listed below in the order they
are recognized by the scripts:

  VEROBSE={verbosity_level}
     This controls how verbose the scripts are in their output. The default
     level is 0 which produces no output unless there's an error. At level 1
     you'll get basic confirmation of the action taken. At level 2 you'll get
     a dialogue of the individual steps taken by the script and verbose 
     output from anything it runs.  At level 3 you'll get full diagnostics
     messages (generally only useful for implementers of these scripts).

  PREFIX=/path/to/install/directory
     This is the prefix directory for installation. It is the expected final
     location for installation of the software.

  DESTDIR=/path/to/destination/directory
     Specify the directory above where the install prefix will install. This
     is handy for package maintainers. You can set PREFIX=/usr/bin but then
     you don't actually want it installed there! So, specify DESTDIR=/tmp and
     it would actually get installed in /tmp/usr/bin.

  LLVM_TOP=/path/to/llvm-top
     This allows you to override the location of the llvm-top directory.
     
  -*
  --*
  *=*
     Any options matching these patterns are collected and passed down to the
     build, install or clean commands.

  all
     This is equivalent to specify all modules in the LLVM subversion 
     repository. Careful! All the scripts will check out EVERYTHING in the
     repository. 

  [a-zA-Z]*
     Any option not matching something above and starting with a letter 
     specifies a module name to work on.

  *
     Anything else is an error.

All the scripts need some (minimal) set of modules to work on. You have three
choices on the command line:

  1. Don't specify any modules - the script will work with the currently
     checked out set of modules.

  2. Specify the modules you want, by name - generally you only have to 
     specify the one or two at the top of the dependency graph.

  3. Specify "all" - all modules will be checked out (careful!)

So, for example:

  ./build llvm-gcc-4.0 ENABLE_OPTIMIZED=1 PREFIX=/my/install/dir VERBOSE=1

As you might guess, this will do the following:

  1. Check out the llvm-gcc-4.0 module
  2. Check out the core module because llvm-gcc-4.0 depends on core
  3. Check out the support module because core depends on support
  4. Build the support module in optimized mode and configured to install
     into /my/install/dir
  5. Build the core module the same way.
  6. Build the llvm-gcc-4.0 module the same way.
  7. Do all of the above with some simple progress messages.

The modules available are:

  llvm-top     - This directory
  sample       - A sample module you can use as a template for your own
  support      - The support libraries, makefile system, etc.
  core         - The core llvm software (currently "llvm")
  llvm-gcc-4.0 - The C/C++/Obj-C front end for llvm, based on GCC 4.0
  llvm-gcc-4.2 - The C/C++/Obj-C front end for llvm, based on GCC 4.2
  cfe          - The new C/C++/Obj-C front end for llvm
  test-suite   - The llvm test suite
  stacker      - The stacker front end (a 'Forth-like' language)
  hlvm         - High Level Virtual Machine (nascent)
  java         - Java Front End (unfinished, out of date)
  poolalloc    - The pooled allocator from Chris Lattner's thesis


You can check out any number of modules using the "get" script, for example,
like this:

  ./get llvm-gcc-4.0 test-suite stacker

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
