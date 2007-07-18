#=== autoconf/Makefile.module - autoconf makefile for modules-*- Makefile -*===#
# 
#                     The LLVM Support Autoconf Makefile
#
# This file was developed by the Reid Spencer and is distributed under the
# University of Illinois Open Source License. See LICENSE.TXT for details.
# 
#===------------------------------------------------------------------------===#

# See if we can't intuit some paths that we need, based on the assumption that
# this thing is checked out into an llvm-top directory and the things we need
# are in llvm-top
# be set if LLVM_TOP.
ifndef LLVM_TOP
  LLVM_TOP := $(shell cd ../.. ; /bin/pwd)
endif
ifndef MODULE_DIR
  MODULE_DIR := $(shell cd .. ; /bin/pwd)
endif

AUTOREGEN=$(LLVM_TOP)/support/autoconf/AutoRegen.sh
CONFIGURE=$(MODULE_DIR)/configure
CONFIGURE_AC=$(MODULE_DIR)/autoconf/configure.ac
M4_FILES := $(wildcard $(LLVM_TOP)/support/autoconf/m4/*.m4)
M4_FILES += $(wildcard $(MODULE_DIR)/autoconf/m4/*.m4)
MODULE_INFO=$(MODULE_DIR)/ModuleInfo.txt

all: $(CONFIGURE)

$(CONFIGURE) : $(AUTOREGEN) $(CONFIGURE_AC) $(M4_FILES) $(MODULE_INFO)
	@LLVM_TOP="$(LLVM_TOP)" $(AUTOREGEN)

$(CONFIGURE_AC):
	@echo "Your module ($(MODULE_DIR)) needs a configure.ac file"

print:
	@echo "LLVM_TOP=$(LLVM_TOP)"
	@echo "MODULE_DIR=$(MODULE_DIR)"
	@echo "CONFIGURE=$(CONFIGURE)"
	@echo "CONFIGURE_AC=$(CONFIGURE_AC)"
	@echo "M4_FILES=$(M4_FILES)"
