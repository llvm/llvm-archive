##===- tests/Compiler/Makefile -----------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file was developed by the LLVM research group and is distributed under
# the University of Illinois Open Source License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##
LEVEL := ../..

CLASSFILES := $(wildcard *.class)

all:: $(foreach ext,rbc ll,$(CLASSFILES:%.class=Output/%.$(ext)))

include $(LEVEL)/test/Makefile.test
