#===- ./Makefile -------------------------------------------*- Makefile -*--===#
# 
#                     The LLVM Compiler Infrastructure
#
# This file was developed by the LLVM research group and is distributed under
# the University of Illinois Open Source License. See LICENSE.TXT for details.
# 
#===------------------------------------------------------------------------===#

# This is the top level Makefile for the LLVM support module.
LEVEL := .
DIRS  := lib
GOAL  := common

include $(LEVEL)/Makefile.config 

#------------------------------------------------------------------------
# Make sure the generated headers are up-to-date. This must be kept in
# sync with the AC_CONFIG_HEADER invocations in autoconf/configure.ac
#------------------------------------------------------------------------
FilesToConfig := \
  include/llvm/Config/config.h \
  include/llvm/Support/DataTypes.h \
  include/llvm/ADT/hash_map \
  include/llvm/ADT/hash_set \
  include/llvm/ADT/iterator
FilesToConfigPATH  := $(addprefix $(OBJ_ROOT)/,$(FilesToConfig))

all-local:: $(FilesToConfigPATH)
$(FilesToConfigPATH) : $(OBJ_ROOT)/% : $(SRC_ROOT)/%.in 
	$(Echo) Regenerating $*
	$(Verb) cd $(OBJ_ROOT) && $(ConfigStatusScript) $*

.PRECIOUS: $(FilesToConfigPATH)
