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
	ForLoop \
	If \
	LookupSwitch \
	Return \
	StaticInitializers \
	TableSwitch \
#	ClassTest \
#	FieldAccess \
#	FloatCompare \
#	LongCompare \
#	InstanceOf \
#	VTable

all:: Output/.compile-java $(addprefix Output/, $(addsuffix .llvm.bc, $(JAVA_TESTS)))

include ../Makefile.singlesrc
