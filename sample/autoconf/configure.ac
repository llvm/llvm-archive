dnl === configure.ac --------------------------------------------------------===
dnl                     LLVM Sample Module Configuration
dnl
dnl This file was developed by Reid Spencer and is distributed under the 
dnl University of Illinois Open Source License. See LICENSE.TXT for details.
dnl 
dnl===-----------------------------------------------------------------------===
dnl This is the sample module configuration script. It is processed by the 
dnl autoconf program to produce a script named configure. This script contains 
dnl the configuration checks that the sample module needs in order to support 
dnl multiple platforms.
dnl
dnl This file is composed of 10 sections per the recommended organization of
dnl autoconf input defined in the autoconf documentation. As this file evolves,
dnl please keep the various types of checks within their sections. The sections
dnl are as follows:
dnl
dnl SECTION  1: Initialization & Setup
dnl SECTION  2: Architecture, target, and host checks
dnl SECTION  3: Command line arguments for the configure script.
dnl SECTION  4: Check for programs we need and that they are the right version
dnl SECTION  5: Check for libraries
dnl SECTION  6: Check for header files
dnl SECTION  7: Check for types and structures
dnl SECTION  8: Check for specific functions needed
dnl SECTION  9: Additional checks, variables, etc.
dnl SECTION 10: Specify the output files and generate it
dnl
dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 1: Initialization & Setup
dnl===
dnl===-----------------------------------------------------------------------===
dnl Initialize autoconf and define the package name, version number and
dnl email address for reporting bugs.
AC_INIT([[LLVM Sample Module]],[[1.0]],[bugs@yourdomain.tld],[sample])
THIS_IS_LLVM_MODULE([sample])

dnl Verify that the source directory is valid. This makes sure that we are
dnl configuring LLVM and not some other package (it validates --srcdir argument)
AC_CONFIG_SRCDIR([lib/sample/sample.c])

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 2: Architecture, target, and host checks
dnl===
dnl===-----------------------------------------------------------------------===

dnl NOTES FOR MODULES:
dnl   1. You don't need AC_CANONICAL_TARGET here, its already been done.
dnl   2. Most everything you'll need is already in the support/Makefile.config
dnl      which should be included into your module's Makefile.config
dnl   3. I.e. probably this section is empty for your module.

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 3: Command line arguments for the configure script.
dnl===
dnl===-----------------------------------------------------------------------===

dnl NOTES FOR MODULES:
dnl   1. This is where you can put AC_ARG_WITH and AC_ARG_ENABLE macros to 
dnl      define the --with-* and --enable-* options.
dnl      For example:
dnl        AC_ARG_WITH(my_thing, AS_HELP_STRING([--with-my-thing,
dnl          [Specify where my-thing is]),,withval=default)
dnl        case "$withval" in
dnl          default)  MY_THING=/default/path/to/my/thing ;;
dnl          *) MY_THING=$withval ;;
dnl        esac
dnl        AC_SUBST(MY_THING,$MY_THING)
dnl   2. Also put anything else here that is general and not a feature test

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 4: Check for programs we need and that they are the right version
dnl===
dnl===-----------------------------------------------------------------------===

dnl NOTES FOR MODULES:
dnl   1. Most of the typical programs are already checked for by support and
dnl      made available through the makefile. Its more efficient to use those
dnl      than re-check them in the module, but this isn't a hard and fast rule.
dnl   2. This is where you use AC_PROG_* and AC_PATH_PROG macros
dnl   3. You should also check sanity and version for these programs in this 
dnl      section.
dnl   4. Its useful to default the programs to "echo ProgName" so that if they
dnl      get used in a makefile rule, they simply print out the command line
dnl      that was supposed to be run.
dnl   5. If tools have dependencies then check their compatibility here too.
dnl   6. If you can use a set of alternate tools, pick one here and configure
dnl      for it.

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 5: Check for libraries
dnl===
dnl===-----------------------------------------------------------------------===

dnl NOTES FOR MODULES:
dnl   1. This section is for AC_CHECK_LIB and AC_SEARCH_LIBS macros.

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 6: Check for header files
dnl===
dnl===-----------------------------------------------------------------------===

dnl NOTES FOR MODULES:
dnl   1. This section is for AC_HEADER_* and AC_CHECK_HEADERS macros.
dnl   2. Note that support/autoconf checks for many of the standard operating
dnl      system headers and provides a lib/System to provide a consistent 
dnl      interface to the operating system. Chances are you don't need any
dnl      but could instead include support/include/llvm/Config/config.h
dnl   3. Also, use the C++ variant (e.g. <cstdio>) as this has some platform
dnl      neutrality benefits as well.

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 7: Check for types and structures
dnl===
dnl===-----------------------------------------------------------------------===

dnl NOTES FOR MODULES:
dnl   1. Most checks for integer type sizes are done by support module.
dnl   2. See the support/include/llvm/Support/DataTypes.h file
dnl  

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 8: Check for specific functions needed
dnl===
dnl===-----------------------------------------------------------------------===

dnl NOTES FOR MODULES:
dnl   1. This section is for AC_CHECK_FUNCS macros and similar ones that
dnl      check for the availability of specific functions.
dnl   2. Again, many of the operating system specific functions are already
dnl      handled by the lib/System library in the support module. Use its
dnl      interface (support/include/llvm/System/*.h) instead of relying on
dnl      system-specific behavior. If it lacks something, add an abstraction.

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 9: Additional checks, variables, etc.
dnl===
dnl===-----------------------------------------------------------------------===

dnl NOTES_FOR_MODULES:
dnl   1. This is for anything else that doesn't fit any of the other sections.
dnl   2. If you need to compute substitution variables, this is the place to
dnl      do it.

dnl===-----------------------------------------------------------------------===
dnl===
dnl=== SECTION 10: Specify the output files and generate it
dnl===
dnl===-----------------------------------------------------------------------===

dnl NOTES FOR MODULES:
dnl   1. This is *only* for AC_CONFIG_FILES, AC_CONFIG_HEADER, AC_CONFIG_LINKS,
dnl      AC_CONFIG_COMMANDS, CONFIG_LLVM_MAKEFILE, and AC_OUTPUT macros.
dnl   2. Many of the configured header files you'll need are already in the
dnl      support/include/llvm/Config/config.h and other support headers. No
dnl      need to do them again in your module.
dnl   3. You don't need to do CONFIG_LLVM_MAKEFILE on your Makefile.config or
dnl      Makefile.common, it was done by THIS_IS_LLVM_MODULE
dnl   4. The only makefiles that need to be specified are the ones that are in
dnl      the first level directories below the module's directory (e.g. lib,
dnl      tools, etc.) The makefile system knows how to propagate makefiles to
dnl      your build directory at lower levels.
dnl   5. The AC_OUTPUT macro *must* be last. Its the one that does everything

dnl Do special configuration of Makefiles
CONFIG_LLVM_MAKEFILE(Makefile)
CONFIG_LLVM_MAKEFILE(lib/Makefile)
CONFIG_LLVM_MAKEFILE(lib/sample/Makefile)
CONFIG_LLVM_MAKEFILE(tools/Makefile)
CONFIG_LLVM_MAKEFILE(tools/sample/Makefile)

dnl Finally, crank out the output
AC_OUTPUT
