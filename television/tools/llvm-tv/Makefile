LEVEL = ../..
TOOLNAME = llvm-tv
USEDLIBS = 
LLVMLIBS = analysis.a ipa.a bcreader vmcore support.a
CPPFLAGS = $(shell $(LEVEL)/lib/wxwindows/wx-config --cflags)
TOOLLINKOPTSB = $(shell $(LEVEL)/lib/wxwindows/wx-config --libs)

ifdef NOHTML
CPPFLAGS += -DNOHTML
endif

include $(LEVEL)/Makefile.common

all :: $(DESTTOOLCURRENT)/llvm-tv.exe \
       $(DESTTOOLCURRENT)/opt-snap

$(DESTTOOLCURRENT)/llvm-tv.exe: Makefile
	echo exec env LD_LIBRARY_PATH=$(shell cd $(LEVEL)/lib/wxwindows/lib; pwd):\$$LD_LIBRARY_PATH \
		llvm-tv  > $@
	chmod u+x $@

$(DESTTOOLCURRENT)/opt-snap: Makefile
	echo exec env LD_LIBRARY_PATH=$(shell cd $(LEVEL)/lib/wxwindows/lib; pwd):\$$LD_LIBRARY_PATH \
		OPTPASSES=\"$$\*\" opt -load=$(DESTLIBCURRENT)/libsnapshot.so $$\* > $@
	chmod u+x $@

ifeq ($(OS),Darwin)
all :: mac-app
clean :: clean-mac-app
endif

mac-app:
	@./MakeMacOSBundle llvm-tv "$(LEVEL)/lib/wxwindows" "$(DESTTOOLCURRENT)/llvm-tv"

clean-mac-app:
	rm -rf llvm-tv.app
