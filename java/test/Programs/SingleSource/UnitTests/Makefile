##===- test/Programs/SingleSource/UnitTests/Makefile -------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file was developed by the LLVM research group and is distributed under
# the University of Illinois Open Source License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##
LEVEL := ../../../..

CPPFLAGS+=-I$(PROJ_SRC_ROOT)/include/llvm/Java
BYTECODE_LIBRARY=1
SHARED_LIBRARY=1
LIBRARYNAME=test
BUILT_SOURCES = Test.h

include $(LEVEL)/Makefile.common

EXTRA_OBJS = $(LibName.BCA)

BUILD_JAVA_SOURCES=1

include ../Makefile.singlesrc

Test.h: Output/Test.class
	$(Verb)$(Echo) Creating JNI header for class $(basename $(notdir $<))
	$(Verb)$(RM) -f $@
	$(Verb)$(JAVAH) -classpath Output Test

clean::
	$(Verb)rm -f Test.h
