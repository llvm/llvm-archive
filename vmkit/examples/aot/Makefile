##===- tools/j3/Makefile -----------------------------------*- Makefile -*-===##
# 
#                     The VMKit project
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##
LEVEL=../..

BC_FILES=$(BUILD_DIR)/HelloWorld.class.bc
TOOL=aot-test
LINK_USE=Classpath J3 J3Compiler Vmkit VmkitCompiler CommonThread FinalMMTk Precompiled

include $(LEVEL)/Makefile.common

%.class.o: %.class.bc
	$(LLC) -load $(STATIC_GC_PRINTER_LIB) -j3-aot -filetype=obj -o $@ $<

%.class.bc: %.class
	cd $(BUILD_DIR) && $(VMJC) $(notdir $<)

$(BUILD_DIR)/%.class: %.java $(BUILD_DIR)/.dir
	$(JAVAC) -d $(BUILD_DIR) $<

check:
	echo $(MODULE_A_DEP)

