LEVEL = ../..
TOOLNAME = llvm-tv
USEDLIBS = 
LLVMLIBS = bcreader vmcore support.a
CPPFLAGS = $(shell $(LEVEL)/lib/wxwindows/wx-config --cflags)
TOOLLINKOPTSB = $(shell $(LEVEL)/lib/wxwindows/wx-config --libs)

include $(LEVEL)/Makefile.common

all :: $(DESTTOOLCURRENT)/llvm-tv.exe \
       $(DESTTOOLCURRENT)/opt-snap

$(DESTTOOLCURRENT)/llvm-tv.exe: Makefile
	echo exec env LD_LIBRARY_PATH=$(shell cd $(LEVEL)/lib/wxwindows/lib; pwd) \
		llvm-tv  > $@
	chmod u+x $@

$(DESTTOOLCURRENT)/opt-snap: Makefile
	echo exec env LD_LIBRARY_PATH=$(shell cd $(LEVEL)/lib/wxwindows/lib; pwd) \
		OPTPASSES=\"$$\*\" opt -load=$(DESTLIBCURRENT)/libsnapshot.so $$\* > $@
	chmod u+x $@

