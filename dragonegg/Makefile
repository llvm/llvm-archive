C_SOURCE_FILES=llvm-cache.c
CPP_SOURCE_FILES=llvm-convert.cpp llvm-backend.cpp llvm-debug.cpp llvm-target.cpp llvm-types.cpp bits_and_bobs.cpp
PLUGIN_OBJECT_FILES=$(C_SOURCE_FILES:.c=.o) $(CPP_SOURCE_FILES:.cpp=.o)

#GCCPLUGIN_DIR:=$(shell $(GCC) -print-file-name=plugin)
GCCSOURCE_DIR=/home/duncan/tmp/gcc.fsf.master
GCCOBJECT_DIR=/home/duncan/tmp/gcc.fsf.master-objects

#CFLAGS+=-I$(GCCPLUGINS_DIR)/include -fPIC -O2

CFLAGS+=-Werror -fPIC -g -O2
CFLAGS+= -I${GCCOBJECT_DIR}/gcc -I${GCCOBJECT_DIR}/gcc/include \
	 -I${GCCSOURCE_DIR}/gcc -I${GCCSOURCE_DIR}/include \
	 -I${GCCSOURCE_DIR}/libcpp/include -I${GCCSOURCE_DIR}/libdecnumber \
	 -I${GCCOBJECT_DIR}/libdecnumber
CXXFLAGS+=$(CFLAGS) $(shell llvm-config --cppflags)

LDFLAGS+=$(shell llvm-config --ldflags) $(shell llvm-config --libs analysis core target x86)

llvm.so: $(PLUGIN_OBJECT_FILES)
	$(CXX) -shared $^ -o $@ ${LDFLAGS}

clean::
	rm -f *.o *.so
