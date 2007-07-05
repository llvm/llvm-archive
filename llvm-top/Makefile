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
SVNROOT := $(shell $(SVN) info . | grep 'Repository Root:' | \
                   sed -e 's/^Repository Root: //')

.PHONY: checkout

checkout:
	$(SVN) co $(SVNROOT)/$(MODULE)/trunk $(MODULE)




get-llvm: llvm
llvm:
	@$(SVN) co $(SVNROOT)/llvm/trunk llvm

build-llvm: llvm
	@root=`pwd` ; cd llvm ; \
	  ./configure --prefix="$$root/install" \
	    --with-llvm-gcc="$$root/llvm-gcc" ; \
	  make tools-only ; 
