##===- test/Programs/SingleSource/UnitTests/Makefile -------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file was developed by the LLVM research group and is distributed under
# the University of Illinois Open Source License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##
LEVEL := ../../../..

JAVA_TESTS := Arithm \
	BigConstants \
	FieldAccess \
	FloatCompare \
	ForLoop \
	If \
	LongCompare \
	LookupSwitch \
	Return \
	StaticInitializers \
	TableSwitch \
#	VTable \
#	ClassTest \
#	InstanceOf \

CPPFLAGS+=-I$(BUILD_SRC_ROOT)/include/llvm/Java
BYTECODE_LIBRARY=1
LIBRARYNAME=test

EXTRA_OBJS = $(LibDir)/libtest.bca

all:: Output/.compile-java $(addprefix Output/, $(addsuffix .llvm.bc, $(JAVA_TESTS)))

include ../Makefile.singlesrc
