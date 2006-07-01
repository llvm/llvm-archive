#------------------------------------------------------------------------------
# Makefile.hlvm - Make -> scons conversion
#
# Copyright (C) 2006 Reid Spencer. All Rights Reserved
#
#------------------------------------------------------------------------------
# This makefile simply converts standard make targets into the equivalent 
# scons invocation in order to retain compatibility for those who wish to use
# make.
#------------------------------------------------------------------------------

# Select the name of the build mode you want to build by default.
MYMODE := Debug

# Specify a path for where the 3rd party software is installed
MYPATH := /proj/install:/proj/llvm/cfe/install

# Specify where you want HLVM to be installed
MYPREFIX := /proj/install/hlvm

SCONSOPTS := -Q -j 2

SUITES := all

.PHONY: all debug Debug optimized Optimized release Release check clean \
	install hlvm tools

all:
	scons $(SCONSOPTS) mode=$(MYMODE)

debug Debug:
	scons $(SCONSOPTS)  mode=Debug debug=1 assertions=1 optimized=0 \
	inline=0 small=0 confpath=$(MYPATH) prefix=$(MYPREFIX)

optimized Optimized:
	scons $(SCONSOPTS)  mode=Optimized debug=0 assertions=1 optimized=1 \
	inline=1 small=0 strip=0 confpath=$(MYPATH) prefix=$(MYPREFIX)

release Release:
	scons $(SCONSOPTS) mode=Release debug=0 assertions=0 optimized=1 \
	inline=1 small=1 strip=1 confpath=$(MYPATH) prefix=$(MYPREFIX)

profile Profile:
	scons $(SCONSOPTS) mode=Profile debug=0 assertions=0 optimized=1  \
	inline=1 small=0 strip=0 profile=1 confpath=$(MYPATH) prefix=$(MYPREFIX)

check: 
	scons -Q check mode=$(MYMODE) suites=$(SUITES)

clean:
	scons $(SCONSOPTS) --clean mode=$(MYMODE)

install:
	scons $(SCONSOPTS) install mode=$(MYMODE)

doc:
	scons $(SCONSOPTS) docs mode=$(MYMODE)

hlvm:
	cd hlvm ; scons $(SCONSOPTS) -u mode=$(MYMODE)

tools:
	cd tools ; scons $(SCONSOPTS) -u mode=$(MYMODE)

dist: exclusions
	tar jcf hlvm-0.1-tar.bz2 -C .. --exclude-from exclusions hlvm

exclusions: Makefile
	echo exclusions > exclusions
	echo .svn >> exclusions
	echo '.*_options' >> exclusions
	echo '*.pyc' >> exclusions
	echo '*.tmp' >> exclusions
	echo Debug >> exclusions
	echo Optimized >> exclusions
	echo Release >> exclusions
	echo Profile >> exclusions
	echo default >> exclusions
	echo Library >> exclusions
	echo hlvm-0.1-tar.bz2 >> exclusions
	echo '*.swp' >> exclusions
	echo 'config.log' >> exclusions
