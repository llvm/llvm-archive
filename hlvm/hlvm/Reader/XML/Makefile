#-------------------------------------------------------------------------------
# 
# Copyright (C) 2006 HLVM Group. All Rights Reserved
#
#-------------------------------------------------------------------------------

LEVEL 			:= ../../..
LIBRARYNAME 		:= HLVMXMLReader
DONT_BUILD_RELINKED 	:= 1
BUILD_ARCHIVE 		:= 1
EXTRA_DIST 		:= HLVM.rng
BUILT_SOURCES 		:= HLVMTokenizer.cpp HLVM.rng.inc
INSTALL_INCLUDES 	:= XMLReader.h

include $(LEVEL)/Makefile.hlvm

$(ObjDir)/HLVMTokenizer.o: \
	$(PROJ_OBJ_DIR)/HLVMTokenizer.h \
	$(PROJ_OBJ_DIR)/HLVMTokenHash.i \
	$(PROJ_OBJ_DIR)/HLVMTokenizer.cpp

$(PROJ_OBJ_DIR)/HLVMTokenizer.cpp $(PROJ_OBJ_DIR)/HLVMTokenizer.h \
  $(PROJ_OBJ_DIR)/HLVMTokenHash.i : $(PROJ_SRC_DIR)/HLVM.rng \
  $(HLVM_top_srcdir)/utils/tmplt/Tokenizer_Template.cpp \
  $(HLVM_top_srcdir)/utils/tmplt/Tokenizer_Template.h \
  $(HLVM_top_srcdir)/utils/bin/mkTokenizer
	$(Echo) Building Tokenizer For $*
	$(Verb) $(HLVM_top_srcdir)/utils/bin/mkTokenizer -f \
	  $(PROJ_SRC_DIR)/HLVM.rng

HLVM.rng.inc : $(PROJ_SRC_DIR)/HLVM.rng Makefile
	$(Echo) "Converting HLVM.rng -> HLVM.inc"
	$(Verb) sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' \
	            -e 's/^/"/' -e 's/$$/"/' $< > $@
