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
	Lists1 \
	Lists2 \
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
#	Strings \
#	MultipleInterfaces \
#	MultiArray \


CPPFLAGS+=-I$(BUILD_SRC_ROOT)/include/llvm/Java
BYTECODE_LIBRARY=1
SHARED_LIBRARY=1
LIBRARYNAME=test

EXTRA_OBJS = $(LibDir)/libtest.bca

Test.h: Output/Test.class
	$(Verb)$(Echo) Creating JNI header for class $(basename $(notdir $<))
	$(Verb)$(RM) -f $@
	$(Verb)$(JAVAH) -classpath Output Test

clean::
	$(Verb)rm -f Test.h

include ../Makefile.singlesrc

$(EXTRA_OBJS): Test.h
