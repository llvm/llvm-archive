cmake_minimum_required(VERSION 2.8)
project(LLBROWSE)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

# Macros we'll need
include(FindwxWidgets)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
#include(CheckFunctionExists)
#include(CheckCXXSourceCompiles)
#include(CheckTypeSize)

# Check for the existence of include files.
check_include_file(assert.h HAVE_ASSERT_H)
#check_include_file(stdio.h HAVE_STDIO_H)
#check_include_file(stdint.h HAVE_STDINT_H)
#check_include_file(stddef.h HAVE_STDDEF_H)
#check_include_file(stdlib.h HAVE_STDLIB_H)
#check_include_file(string.h HAVE_STRING_H)
#check_include_file(stdbool.h HAVE_STDBOOL_H)
#check_include_file(stdarg.h HAVE_STDARG_H)
#check_include_file(malloc.h HAVE_MALLOC_H)
#check_include_file(unistd.h HAVE_UNISTD_H)
#check_include_file(inttypes.h HAVE_INTTYPES_H)
#check_include_file(errno.h HAVE_ERRNO_H)
#check_include_file(execinfo.h HAVE_EXECINFO_H)
#check_include_file(sys/time.h HAVE_SYS_TIME_H)
#check_include_file(sys/types.h HAVE_SYS_TYPES_H)
#check_include_file_cxx(new HAVE_NEW)

# Check for type sizes
#check_type_size(int SIZEOF_INT)
#check_type_size(long SIZEOF_LONG)
#check_type_size("long long" SIZEOF_LONG_LONG)
#check_type_size("void*" SIZEOF_VOID_PTR)

# Find WxWidgets
set(wxWidgets_USE_DEBUG true)
set(wxWidgets_USE_UNICODE true)
find_package(wxWidgets REQUIRED core base html)
include(${wxWidgets_USE_FILE})
     
# Find LLVM
find_package(LLVM REQUIRED)

# Definitions needed by LLVM headers
add_definitions(
    -D__STDC_LIMIT_MACROS
    -D__STDC_CONSTANT_MACROS
    -D_GNU_SOURCE)

# Include directory paths
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${LLBROWSE_SOURCE_DIR}/include)
include_directories(${LLVM_INCLUDE_DIR})

# Generate the config.h file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h)

file(GLOB LLBROWSE_SOURCES lib/*.cpp)
file(GLOB LLBROWSE_HEADERS include/llbrowse/*.h)

# Set up the resources target
# Set the list of resource to be the empty list initially.
# add_custom_target("${EXE_FILE}.run" COMMAND ./${EXE_FILE} DEPENDS ${EXE_FILE})
set(RESOURCES)

# Convert PNG image resources to embeddable header files.
file(GLOB PNG_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} resources/icons/*.png)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/resources/icons")
foreach(IN_FILE ${PNG_RESOURCES})
  string(REGEX REPLACE ".png\$" ".png.h" OUT_FILE "${IN_FILE}")
  set(ABS_IN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${IN_FILE}")
  set(ABS_OUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${OUT_FILE}")
  add_custom_command(
      OUTPUT ${ABS_OUT_FILE}
      MAIN_DEPENDENCY ${ABS_IN_FILE}
      COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/png2c" ${ABS_IN_FILE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources/icons
      COMMENT "Converting PNG image file ${IN_FILE}")
  set(RESOURCES ${RESOURCES} "${ABS_OUT_FILE}")
endforeach(IN_FILE)

# Declare the llbrowse executable
add_executable(llbrowse ${LLBROWSE_SOURCES} ${LLBROWSE_HEADERS} ${RESOURCES})
target_link_libraries(llbrowse ${wxWidgets_LIBRARIES} ${LLVM_MODULE_LIBS})
set_target_properties(llbrowse PROPERTIES
    COMPILE_FLAGS ${LLVM_CFLAGS}
    LINK_FLAGS ${LLVM_LFLAGS})
