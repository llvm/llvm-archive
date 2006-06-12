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

.PHONY: all check install hlvm tools
all:
	scons -Q

debug:
	scons -Q mode=debug debug=1 assertions=1 optimized=0 inline=0 small=0

optimized:
	scons -Q mode=optimized debug=0 assertions=1 optimized=1 inline=1 \
	small=0

release:
	scons -Q mode=release debug=0 assertions=0 optimized=1 inline=1 small=1

check:  all
	scons -Q check

clean:
	scons -Q --clean 

install:
	scons -Q install

doc:
	scons -Q docs

hlvm:
	cd hlvm ; scons -Q -u

tools:
	cd tools ; scons -Q -u
