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
	Array \
	BigConstants \
	FieldAccess \
	FloatCompare \
	ForLoop \
	If \
	InstanceOf \
	InterfaceCall \
	LongCompare \
	LookupSwitch \
	StaticInitializers \
	TableSwitch \
	VirtualCall \
	VTable \
#	MultiArray \


CPPFLAGS+=-I$(BUILD_SRC_ROOT)/include/llvm/Java
BYTECODE_LIBRARY=1
SHARED_LIBRARY=1
LIBRARYNAME=test

all:: Test.h

Test.h: Output/.compile-java
	$(Verb)$(JAVAH) -classpath Output Test

EXTRA_OBJS = $(LibDir)/libtest.bca

include ../Makefile.singlesrc
