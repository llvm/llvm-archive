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
	ClassTest \
	FieldAccess \
	FloatCompare \
	ForLoop \
	If \
	InstanceOf \
	LongCompare \
	LookupSwitch \
	Return \
	StaticInitializers \
	TableSwitch \
	VTable

CPPFLAGS+=-I$(BUILD_SRC_ROOT)/include/llvm/Java
BYTECODE_LIBRARY=1
SHARED_LIBRARY=1
LIBRARYNAME=test

EXTRA_OBJS = $(LibDir)/libtest.bca

include ../Makefile.singlesrc
