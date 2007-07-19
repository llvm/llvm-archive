#===-- Makefile.tool - Rules for building tools ------------*- Makefile -*--===#
#
#                     The LLVM Compiler Infrastructure
#
# This file was developed by Reid Spencer and is distributed under the
# University of Illinois Open Source License. See LICENSE.TXT for details.
# 
#===------------------------------------------------------------------------===#
#
# This file provides rules for compiling code and linking executable tools.
#
#===-----------------------------------------------------------------------====#

include $(LLVM_TOP)/support/make/Makefile.compile

ifndef TOOLNAME
  $(error You must specify TOOLNAME for the "tool" goal)
endif

#---------------------------------------------------------
# Define various command line options pertaining to the
# libraries needed when linking. There are "Proj" libs 
# (defined by the user's project) and "LLVM" libs (defined 
# by the LLVM project).
#---------------------------------------------------------

ifdef USEDLIBS
ProjLibsOptions := $(patsubst %.a.o, -l%, $(addsuffix .o, $(USEDLIBS)))
ProjLibsOptions := $(patsubst %.o, $(LibDir)/%.o,  $(ProjLibsOptions))
ProjUsedLibs    := $(patsubst %.a.o, lib%.a, $(addsuffix .o, $(USEDLIBS)))
ProjLibsPaths   := $(addprefix $(LibDir)/,$(ProjUsedLibs))
endif

ifdef LLVMLIBS
LLVMLibsOptions := $(patsubst %.a.o, -l%, $(addsuffix .o, $(LLVMLIBS)))
LLVMLibsOptions := $(patsubst %.o, $(LLVMLibDir)/%.o, $(LLVMLibsOptions))
LLVMUsedLibs    := $(patsubst %.a.o, lib%.a, $(addsuffix .o, $(LLVMLIBS)))
LLVMLibsPaths   := $(addprefix $(LLVMLibDir)/,$(LLVMUsedLibs))
endif

ifeq ($(strip $(filter clean clean-local dist-clean,$(MAKECMDGOALS))),)
ifdef LINK_COMPONENTS

# If LLVM_CONFIG doesn't exist, build it.  This can happen if you do a make
# clean in tools, then do a make in tools (instead of at the top level).
$(LLVM_CONFIG):
	@echo "*** llvm-config doesn't exist - rebuilding it."
	@$(MAKE) -C $(OBJ_ROOT)/tools/llvm-config
        
$(ToolDir)/$(strip $(TOOLNAME))$(EXEEXT): $(LLVM_CONFIG)

ProjLibsOptions += $(shell $(LLVM_CONFIG) --libs     $(LINK_COMPONENTS))
ProjLibsPaths   += $(LLVM_CONFIG) \
                  $(shell $(LLVM_CONFIG) --libfiles $(LINK_COMPONENTS))
endif
endif

###############################################################################
# Tool Build Rules: Build executable tool based on TOOLNAME option
###############################################################################

#---------------------------------------------------------
# Set up variables for building a tool.
#---------------------------------------------------------
ifdef EXAMPLE_TOOL
ToolBuildPath   := $(ExmplDir)/$(strip $(TOOLNAME))$(EXEEXT)
else
ToolBuildPath   := $(ToolDir)/$(strip $(TOOLNAME))$(EXEEXT)
endif

#---------------------------------------------------------
# Provide targets for building the tools
#---------------------------------------------------------
all-local:: $(ToolBuildPath)

clean-local::
ifneq ($(strip $(ToolBuildPath)),)
	-$(Verb) $(RM) -f $(ToolBuildPath)
endif

ifdef EXAMPLE_TOOL
$(ToolBuildPath): $(ExmplDir)/.dir
else
$(ToolBuildPath): $(ToolDir)/.dir
endif

$(ToolBuildPath): $(ObjectsO) $(ProjLibsPaths) $(LLVMLibsPaths)
	$(Echo) Linking $(BuildMode) executable $(TOOLNAME) $(StripWarnMsg)
	$(Verb) $(LTLink) -o $@ $(TOOLLINKOPTS) $(ObjectsO) $(ProjLibsOptions) \
	$(LLVMLibsOptions) $(ExtraLibs) $(TOOLLINKOPTSB) $(LIBS)
	$(Echo) ======= Finished Linking $(BuildMode) Executable $(TOOLNAME) \
          $(StripWarnMsg) 

ifdef NO_INSTALL
install-local::
	$(Echo) Install circumvented with NO_INSTALL
uninstall-local::
	$(Echo) Uninstall circumvented with NO_INSTALL
else
DestTool = $(PROJ_bindir)/$(TOOLNAME)

install-local:: $(DestTool)

$(DestTool): $(PROJ_bindir) $(ToolBuildPath)
	$(Echo) Installing $(BuildMode) $(DestTool)
	$(Verb) $(ProgInstall) $(ToolBuildPath) $(DestTool)

uninstall-local::
	$(Echo) Uninstalling $(BuildMode) $(DestTool)
	-$(Verb) $(RM) -f $(DestTool)
endif

