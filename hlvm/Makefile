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

MYMODE := Debug
MYPATH := /proj/llvm/cfe/install:/proj/install

.PHONY: all debug Debug optimized Optimized release Release check clean \
	install hlvm tools

all:
	scons -Q mode=$(MYMODE)

debug Debug:
	scons -Q mode=Debug debug=1 assertions=1 optimized=0 inline=0 \
	small=0 confpath=$(MYPATH)

optimized Optimized:
	scons -Q mode=Optimized debug=0 assertions=1 optimized=1 inline=1 \
	small=0 strip=0 confpath=$(MYPATH)

release Release:
	scons -Q mode=Release debug=0 assertions=0 optimized=1 inline=1 \
	small=1 strip=1 confpath=$(MYPATH)

profile Profile:
	scons -Q mode=Profile debug=0 assertions=0 optimized=1 inline=1 \
	small=0 strip=0 profile=1 confpath=$(MYPATH)

check:  all
	scons -Q check mode=$(MYMODE)

clean:
	scons -Q --clean mode=$(MYMODE)

install:
	scons -Q install mode=$(MYMODE)

doc:
	scons -Q docs mode=$(MYMODE)

hlvm:
	cd hlvm ; scons -Q -u mode=$(MYMODE)

tools:
	cd tools ; scons -Q -u mode=$(MYMODE)
