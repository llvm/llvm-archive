LEVEL = ../..
TOOLNAME = llvm-tv
LLVMLIBS = analysis.a bcreader bcwriter datastructure ipa.a support.a \
           target.a vmcore transformutils.a

# note: delete when FileUtils is assimilated into libsupport
USEDLIBS = snapshot

CPPFLAGS = $(shell wx-config --cflags)
TOOLLINKOPTSB = $(shell wx-config --libs)
WXDIR = $(shell wx-config --prefix)
WXLIB = $(WXDIR)/lib

ifdef NOHTML
CPPFLAGS += -DNOHTML
endif

include $(LEVEL)/Makefile.common

all :: $(DESTTOOLCURRENT)/llvm-tv.exe \
       $(DESTTOOLCURRENT)/opt-snap

$(DESTTOOLCURRENT)/llvm-tv.exe: Makefile
	echo exec env LD_LIBRARY_PATH=$(WXLIB):\$$LD_LIBRARY_PATH llvm-tv > $@
	chmod u+x $@

$(DESTTOOLCURRENT)/opt-snap: Makefile
	echo exec env LD_LIBRARY_PATH=$(WXLIB):\$$LD_LIBRARY_PATH \
		OPTPASSES=\"$$\*\" opt -load=$(DESTLIBCURRENT)/libsnapshot.so $$\* > $@
	chmod u+x $@

ifeq ($(OS),Darwin)
all :: mac-app
clean :: clean-mac-app
endif

mac-app:
	@./MakeMacOSBundle llvm-tv "$(WXDIR)" "$(DESTTOOLCURRENT)/llvm-tv"

clean-mac-app:
	rm -rf llvm-tv.app
