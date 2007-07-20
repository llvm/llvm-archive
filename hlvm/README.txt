High Level Virtual Machine (HLVM)
=================================

This directory and its subdirectories contain source code for the High Level 
Virtual Machine (HLVM). HLVM is a toolkit and a set of executables to assist 
with the rapid construction of compilers and virtual machines for various
languages.  It aims to foster domain specific languages, rapid compiler 
development, and a common infrastructure upon which those languages can 
be compiled, execute and inter-operate.

HLVM is a module in the LLVM-based group of projects. LLVM provides HLVM with
an optimizing compiler, native code generation, bytecode, and a basic 
infrastructure on which to build.

Licensing
=========
HLVM is open source software. You may freely distribute it under the terms of
the license agreements found in the LICENSE.txt files, wherever they may be 
found in the source tree. See the top level file, LICENSE.txt, for full details.

Documentation
=============
HLVM comes with documentation in HTML format. These are provided in the docs 
directory. Please visit http://llvm.org/docs/hlvm/ to get an index of the 
documentation that is available. You will find conceptual guides as well as
build and installation instructions there.

Quick Build
===========
HLVM uses the SCons tool for software construction. However a compatible
Makefile is also provided. One slight difference, there is no "configure" step
for SCons. Your first build will configure HLVM for your environment, mostly
automatically unless you have dependent packages installed in non-standard
locations.

The usual should work:
    make 
    make check
    make install
