LEVEL = ../..
TOOLNAME = llvm-tv
USEDLIBS =
CPPFLAGS = -I$(LEVEL)/lib/wxwindows/lib/wx/include/gtk-2.4 -I$(LEVEL)/lib/wxwindows/include/ -DGTK_NO_CHECK_CASTS -D__WXGTK__ -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES

TOOLLINKOPTSB =  -L$(LEVEL)/lib/wxwindows/lib/ -pthread -lwx_gtkd-2.4

include $(LEVEL)/Makefile.common
