LEVEL = ../..
TOOLNAME = llvm-tv
LLVMLIBS = LLVMAnalysis.a LLVMBitReader.a LLVMBitWriter.a \
           LLVMipa.a LLVMSupport.a LLVMTarget.a LLVMCore.a \
           LLVMTransformUtils.a LLVMSupport.a LLVMSystem.a

USEDLIBS = LLVMTVSnapshot.a

# LLVMDataStructure.a comes from PoolAlloc, not LLVM proper.
PALIBS = LLVMDataStructure.a

CPPFLAGS = $(shell wx-config --cxxflags) -fno-strict-aliasing
TOOLLINKOPTSB = $(shell wx-config --debug=yes --libs)
WXDIR = $(shell wx-config --prefix)
WXLIB = $(WXDIR)/lib

include $(LEVEL)/Makefile.common

all :: $(LLVMToolDir)/llvm-tv.exe \
       $(LLVMToolDir)/opt-snap

LLVMTV = $(PROJ_OBJ_ROOT)/$(BuildMode)/bin/llvm-tv

$(LLVMToolDir)/llvm-tv.exe: Makefile
	echo "Creating llvm-tv.exe script"
	echo exec env \
		LD_LIBRARY_PATH=$(WXLIB):$(LibDir):$(PALibDir):\$$LD_LIBRARY_PATH \
		$(LLVMTV) > $@
	chmod u+x $@

$(LLVMToolDir)/opt-snap: Makefile
	echo "Creating opt-snap script"
	echo exec env LD_LIBRARY_PATH=$(WXLIB):\$$LD_LIBRARY_PATH \
		OPTPASSES=\"$$\*\" opt -load=$(LibDir)/libLLVMTVSnapshot.so $$\* > $@
	chmod u+x $@

ifeq ($(OS),Darwin)
all :: mac-app
clean :: clean-mac-app
endif

mac-app:
	@./MakeMacOSBundle llvm-tv "$(WXDIR)" "$(LLVMToolDir)/llvm-tv"

clean-mac-app:
	rm -rf llvm-tv.app
