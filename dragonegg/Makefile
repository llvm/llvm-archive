GCCSOURCE_DIR=$(HOME)/GCC/src/
GCCOBJECT_DIR=$(HOME)/GCC/objects/
#GCCPLUGIN_DIR:=$(shell $(GCC) -print-file-name=plugin)

# Replace with an informative string when doing a release.
REVISION:=$(shell svnversion -n .)

TARGET_TRIPLE:=$(shell $(GCCOBJECT_DIR)/gcc/xgcc -v 2>&1 | grep "^Target:" | sed -e "s/^Target: *//")
ARCH_DIR:=$(shell TARGET_TRIPLE=$(TARGET_TRIPLE) GCCSOURCE_DIR=$(GCCSOURCE_DIR) $(SHELL) ./get_arch_dir)

C_SOURCE_FILES=llvm-cache.c
CPP_SOURCE_FILES=llvm-convert.cpp llvm-backend.cpp llvm-debug.cpp \
		 $(ARCH_DIR)/llvm-target.cpp llvm-types.cpp bits_and_bobs.cpp
PLUGIN_OBJECT_FILES=$(C_SOURCE_FILES:.c=.o) $(CPP_SOURCE_FILES:.cpp=.o)

GENGTYPE_INPUT=$(PWD)/llvm-cache.c
GENGTYPE_OUTPUT=$(PWD)/gt-llvm-cache.h

CFLAGS+=-Wall -Werror -fPIC -g -O2
CFLAGS+=-DIN_GCC -DREVISION=\"$(REVISION)\" \
	-DTARGET_NAME=\"$(TARGET_TRIPLE)\" -I$(ARCH_DIR)
CFLAGS+=-I$(GCCOBJECT_DIR)/gcc -I$(GCCOBJECT_DIR)/gcc/include \
	-I$(GCCSOURCE_DIR)/gcc -I$(GCCSOURCE_DIR)/include \
	-I$(GCCSOURCE_DIR)/libcpp/include -I$(GCCSOURCE_DIR)/libdecnumber \
	-I$(GCCOBJECT_DIR)/libdecnumber -I.
CXXFLAGS+=$(CFLAGS) $(shell llvm-config --cppflags)

LDFLAGS+=$(shell llvm-config --ldflags) $(shell llvm-config --libs analysis core ipo scalaropts target x86)

llvm.so: $(PLUGIN_OBJECT_FILES)
	$(CXX) -shared $^ -o $@ $(LDFLAGS)

llvm-cache.o: gt-llvm-cache.h

gt-llvm-cache.h:
	cd $(GCCOBJECT_DIR)/gcc && ./build/gengtype \
	  -P $(GENGTYPE_OUTPUT) $(GCCSOURCE_DIR) gtyp-input.list \
	    $(GENGTYPE_INPUT)
	sed -i "s/ggc_cache_tab .*\[\]/ggc_cache_tab gt_ggc_rc__gt_llvm_cache_h[]/" $(GENGTYPE_OUTPUT)
	sed -i "s/ggc_root_tab .*\[\]/ggc_root_tab gt_pch_rc__gt_llvm_cache_h[]/" $(GENGTYPE_OUTPUT)

clean::
	rm -f *.o *.so $(ARCH_DIR)/*.o
