PLUGIN_SOURCE_FILES=llvm-convert.cpp llvm-backend.cpp llvm-debug.cpp llvm-types.cpp bits_and_bobs.cpp
PLUGIN_OBJECT_FILES=$(patsubst %.cpp,%.o,$(PLUGIN_SOURCE_FILES))

#GCCPLUGIN_DIR:=$(shell $(GCC) -print-file-name=plugin)
GCCSOURCE_DIR=/home/duncan/tmp/gcc.fsf.master
GCCOBJECT_DIR=/home/duncan/tmp/gcc.fsf.master-objects

#CXXFLAGS+=-I$(GCCPLUGINS_DIR)/include -fPIC -O2

CXXFLAGS+=-Werror -fPIC -g -O2
CXXFLAGS+=$(shell llvm-config --cppflags)
CXXFLAGS+=\
	-I${GCCOBJECT_DIR}/gcc -I${GCCOBJECT_DIR}/gcc/include \
	-I${GCCSOURCE_DIR}/gcc -I${GCCSOURCE_DIR}/include \
	-I${GCCSOURCE_DIR}/libcpp/include -I${GCCSOURCE_DIR}/libdecnumber \
	-I${GCCOBJECT_DIR}/libdecnumber

LDFLAGS+=$(shell llvm-config --ldflags) $(shell llvm-config --libs analysis core target)

llvm.so: $(PLUGIN_OBJECT_FILES)
	$(CXX) -shared $^ -o $@ ${LDFLAGS}

clean::
	rm -f *.o *.so
