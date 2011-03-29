CC_UNDER_TEST := /Developer/usr/bin/clang
CXX_UNDER_TEST := /Developer/usr/bin/clang++

###
# General Config

LLVM_TEST_SUITE := $(shell echo ~/llvm-test-suite)
LLVM_EXTERNALS := $(shell echo ~/test-suite-externals)

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

ifneq ($(MULTISAMPLE),)
NT.COMMON_FLAGS += --multisample=$(MULTISAMPLE)
endif

###
# Predefined Nightly Test Configurations

# X86 Configurations
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
# ARM Configurations

ARM_SDK_NAME := iphoneos
ARM_PLATFORM = $(shell /Developer/usr/bin/xcodebuild -version -sdk $(ARM_SDK_NAME) PlatformPath)
ARM_SYSROOT = $(shell /Developer/usr/bin/xcodebuild -version -sdk $(ARM_SDK_NAME) Path)

NT.ARM_COMMON_FLAGS = \
	--isysroot $(ARM_SYSROOT) \
	--cflag -miphoneos-version-min=4.0 --cflag -Wl,--no-demangle \
	--remote --remote-host=localhost --remote-user=mobile --remote-port=3022 \
	--disable-cxx

NT.FLAGS.armv7.thumb = \
	$(NT.ARM_COMMON_FLAGS) \
	--arch armv7 --small \
	--cflag -mthumb
NT.FLAGS.armv6.thumb	 = \
	$(NT.ARM_COMMON_FLAGS) \
	--arch armv6 --small \
	--cflag -mthumb

NT.FLAGS.armv7.no-thumb	 = \
	$(NT.ARM_COMMON_FLAGS) \
	--arch armv7 --small \
	--cflag -mno-thumb
NT.FLAGS.armv6.no-thumb	 = \
	$(NT.ARM_COMMON_FLAGS) \
	--arch armv6 --small \
	--cflag -mno-thumb

NT.FLAGS.armv7.O0.g	 = \
	$(NT.ARM_COMMON_FLAGS) \
	--arch armv7 --small \
	--optimize-option -O0 --cflag -g
NT.FLAGS.armv6.O0.g	 = \
	$(NT.ARM_COMMON_FLAGS) \
	--arch armv6 --small \
	--optimize-option -O0 --cflag -g

NT.FLAGS.armv7.Os.flto = \
	$(NT.ARM_COMMON_FLAGS) \
	--arch armv7 --small \
	--optimize-option -Os --cflag -flto
NT.FLAGS.armv6.Os.flto	 = \
	$(NT.ARM_COMMON_FLAGS) \
	--arch armv6 --small \
	--optimize-option -Os --cflag -flto

NT_ARM_CONFIGS = \
	armv7.thumb armv6.no-thumb \
	armv7.O0.g armv6.O0.g \
	armv7.no-thumb armv6.thumb \
	armv7.Os.flto armv6.Os.flto

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
	  --without-llvm \
	  --test-suite $(LLVM_TEST_SUITE) \
	  --test-externals $(LLVM_EXTERNALS) \
	  -j $(NT_THREADS) --no-auto-name $(NT_NICKNAME)-$* \
	  $(NT.COMMON_FLAGS) $(NT.FLAGS.$*) \
	  --no-timestamp --no-configure
.PHONY: check-%

check: $(NT_CONFIGS:%=check-%)
	true
.PHONY: check

check-arm: $(NT_ARM_CONFIGS:%=check-%)
	true
.PHONY: check-arm

clean:
	rm -rf obj.*
.PHONY: clean

