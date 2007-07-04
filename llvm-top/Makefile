#===- ./Makefile -------------------------------------------*- Makefile -*--===#
# 
#                     llvm-top Makefile
#
# This file was developed by Reid Spencer and is distributed under the
# University of Illinois Open Source License. See LICENSE.TXT for details.
# 
#===------------------------------------------------------------------------===#

# This makefile is used for handling "top level" tasks. The "llvm-top" module
# should be the first one checked out by an llvm developer. Once it is checked
# out you have, via this Makefile, a set of tools to assist with building a
# development environment, including checking out other software, configuration,
# etc.
#
SVN = svn
SVNROOT = http://llvm.org/svn/llvm-project

.PHONY: get-llvm get-hlvm get-stacker get-test-suite get-llvm-gcc

get-llvm: llvm
llvm:
	@$(SVN) co $(SVNROOT)/llvm/trunk llvm

get-test-suite: test-suite
	@$(SVN) co $(SVNROOT)/test-suite/trunk test-suite

get-stacker: stacker
	@$(SVN) co $(SVNROOT)/stacker/trunk stacker

get-hlvm: hlvm
	@echo hlvm: Not implemented yet.
	#@$(SVN) co $(SVNROOT)/hlvm/trunk hlvm

get-llvm-gcc: llvm-gcc
	@echo llvm-gcc: Not implemented yet.
	#@$(SVN) co $(SVNROOT)/llvm-gcc/trunk llvm-gcc

