LEVEL = ../..
TOOLNAME = llvm-tv
LLVMLIBS = LLVMAnalysis.a LLVMBCReader LLVMBCWriter LLVMDataStructure \
           LLVMipa.a LLVMSupport.a LLVMTarget.a LLVMCore LLVMTransformUtils.a \
           LLVMSystem.a LLVMbzip2

# note: delete when FileUtils is assimilated into libsupport
USEDLIBS = LLVMTVSnapshot

CPPFLAGS = $(shell wx-config --cflags)
TOOLLINKOPTSB = $(shell wx-config --libs)
WXDIR = $(shell wx-config --prefix)
WXLIB = $(WXDIR)/lib

include $(LEVEL)/Makefile.common

all :: $(LLVMToolDir)/llvm-tv.exe \
       $(LLVMToolDir)/opt-snap

LLVMTV = $(PROJ_OBJ_ROOT)/Debug/bin/llvm-tv

$(LLVMToolDir)/llvm-tv.exe: Makefile
	echo exec env LD_LIBRARY_PATH=$(WXLIB):\$$LD_LIBRARY_PATH $(LLVMTV) \
		"2&>1 > /dev/null" > $@
	chmod u+x $@

$(LLVMToolDir)/opt-snap: Makefile
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
