##===- test/Programs/SingleSource/UnitTests/Makefile -------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file was developed by the LLVM research group and is distributed under
# the University of Illinois Open Source License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##
LEVEL := ../../../..

CLASSFILES := $(wildcard *.class)

BUILD_JAVA_SOURCE=1

all:: $(foreach ext,linked.dis-ll,$(CLASSFILES:%.class=Output/%.$(ext)))

include ../Makefile.singlesrc
