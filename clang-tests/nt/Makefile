CC_UNDER_TEST := /Developer/usr/bin/clang
CXX_UNDER_TEST := /Developer/usr/bin/clang++

###
# General Config

# FIXME: We shouldn't require an LLVM directory.
LLVM_SRCDIR := $(shell echo ~/llvm)
LLVM_OBJDIR := $(shell echo ~/llvm.obj.64)
LLVM_TEST_SUITE := $(shell echo ~/llvm-test-suite)
LLVM_EXTERNALS := $(shell echo ~/llvm/projects/test-suite-externals)

###
# Nightly Test Options

# Use 'make NT_SERVER=http://server.example.com/perf/submitRun ...' to
# auto submit results.
NT_SERVER :=
NT_NICKNAME := clang_qa-$(shell uname -n)
NT_BASE_DIR := $(shell pwd)

# Use 'make FAST=1 ...' to run tests in parallel.
ifneq ($(FAST),)
NT_THREADS := $(shell sysctl -n hw.ncpu)
else
NT_THREADS := 1
endif

# Use 'make ONLY_TEST=SingleSource ...' to run a subset of tests.
NT.COMMON_FLAGS := --simple
ifneq ($(ONLY_TEST),)
NT.COMMON_FLAGS += --only-test $(ONLY_TEST)
endif

###
# Predefined Nightly Test Configurations

NT.FLAGS.i386		:= --arch i386 --small
NT.FLAGS.x86_64		:= --arch x86_64 --small

NT.FLAGS.i386.O0.g        := --arch i386 --small \
                             --optimize-option -O0 --cflag -g
NT.FLAGS.x86_64.O0.g      := --arch x86_64 --small \
                             --optimize-option -O0 --cflag -g

NT.FLAGS.i386.Os.flto     := --arch i386 --small \
                             --optimize-option -Os --cflag -flto
NT.FLAGS.x86_64.Os.flto   := --arch x86_64 --small \
                             --optimize-option -Os --cflag -flto

NT.FLAGS.i386.O3.g.flto     := --arch i386 --small \
                               --optimize-option -O3 --cflag -g --cflag -flto
NT.FLAGS.x86_64.O3.g.flto   := --arch x86_64 --small \
                               --optimize-option -O3 --cflag -g --cflag -flto

NT_CONFIGS = \
	i386 x86_64 \
	i386.O0.g x86_64.O0.g \
	i386.Os.flto x86_64.Os.flto \
	i386.O3.g.flto x86_64.O3.g.flto

###
# Derived Options

ifeq ($(NT_SERVER),)
NT_SUBMIT_FLAGS :=
else
NT_SUBMIT_FLAGS := --submit $(NT_SERVER)
endif

###

check-%:
	rm -rf $(NT_BASE_DIR)/obj.$*
	lnt runtest $(NT_SUBMIT_FLAGS) nt \
	  --sandbox $(NT_BASE_DIR)/obj.$* \
	  --cc $(CC_UNDER_TEST) \
	  --cxx $(CXX_UNDER_TEST) \
	  --llvm-src $(LLVM_SRCDIR) \
	  --llvm-obj $(LLVM_OBJDIR) \
	  --test-suite $(LLVM_TEST_SUITE) \
	  --test-externals $(LLVM_EXTERNALS) \
	  -j $(NT_THREADS) --no-auto-name $(NT_NICKNAME)-$* \
	  $(NT.COMMON_FLAGS) $(NT.FLAGS.$*) \
	  --no-timestamp --no-configure
.PHONY: check-%

check: $(NT_CONFIGS:%=check-%)
	true
.PHONY: check

clean:
	rm -rf obj.*
.PHONY: clean

