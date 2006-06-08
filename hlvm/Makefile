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

check:  all
	scons -Q check

clean:
	scons -Q --clean 

install:
	scons -Q install

hlvm:
	cd hlvm ; scons -Q -u

tools:
	cd tools ; scons -Q -u
