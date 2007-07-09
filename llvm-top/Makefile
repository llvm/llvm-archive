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

# Figure out the root of the SVN repository by asking for it from 'svn info'
SVNROOT = $(shell $(SVN) info . | grep 'Repository Root:' | \
                   sed -e 's/^Repository Root: //')


# Rule to get the modules that $(MODULE) depends on.
MODULEINFO = $(MODULE)/ModuleInfo.txt
DEPMODULES = grep -i DepModule: $(MODULEINFO) | sed 's/DepModule: *//g'
BUILDTARGET = grep -i BuildTarget: $(MODULEINFO) | sed 's/BuildTarget: *//g'


.PHONY: checkout

ifndef MODULE
checkout:
	@echo ERROR: you must specify a MODULE value to 'make checkout'.
	@echo ERROR: for example, use: 'make checkout MODULE=llvm'.

build:
	@echo ERROR: you must specify a MODULE value to 'make build'.
	@echo ERROR: for example, use: 'make build MODULE=llvm'.
else

# Check out a module and all its dependencies. Note that this arrangement
# depends on each module having a file named ModuleInfo.txt that explicitly
# indicates the other LLVM modules it depends on. See one of those files for
# examples.
checkout: $(MODULE)

$(MODULE):
	$(SVN) co $(SVNROOT)/$(MODULE)/trunk $(MODULE)
	@if test -f $(MODULEINFO); then \
	  for mod in `$(DEPMODULES)`; do \
	    echo -n "NOTE: $(MODULE) module depends on $$mod" ; \
	    if test -d $$mod ; then \
	      echo ", but its already checked out." ; \
	    else \
	      echo ", checking it out." ; \
	      $(MAKE) checkout MODULE=$$mod ; \
	    fi ; \
          done; \
        fi

build: $(MODULE) install
	@if test -f $(MODULEINFO) ; then \
	  BuildTarget=`$(BUILDTARGET)` ; \
	  for mod in `$(DEPMODULES)`; do \
	    echo -n "NOTE: $(MODULE) module depends on $$mod" ; \
	    if test -d $$mod ; then \
	      echo ", building it first." ; \
	    else \
	      echo ", checking it out now." ; \
	      $(MAKE) checkout MODULE=$$mod ; \
	      if test "$$?" -ne 0 ; then exit 1 ; fi \
	    fi ; \
	    $(MAKE) build MODULE=$$mod ; \
	    if test "$$?" -ne 0 ; then exit 1 ; fi \
          done; \
        fi ; \
	echo "Building MODULE $(MODULE)" ; \
	root=`pwd` ; cd $(MODULE) ; $(MAKE) $$BuildTarget LLVM_TOP=$$root 
endif

install:
	@mkdir install
