##===- test/Programs/SingleSource/UnitTests/Makefile -------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file was developed by the LLVM research group and is distributed under
# the University of Illinois Open Source License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##
LEVEL := ../../../..

JAVA_TESTS := AbstractCall \
	Arithm \
	Array \
	BigConstants \
	Call \
	FieldAccess \
	FloatCompare \
	ForLoop \
	If \
	InnerClass \
	InstanceOf \
	InterfaceCall \
	Lists \
	LongCompare \
	LookupSwitch \
	Numbers \
	StaticInitializers \
	TableSwitch \
	Test \
	VirtualCall \
	VTable \
	ackermann \
	ary \
	fibo \
	heapsort \
	methcall \
	nestedloop \
	objinst \
	sieve \
	random \
	takfp \
#	Lists1 \
#	Strings \
#	MultipleInterfaces \
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
