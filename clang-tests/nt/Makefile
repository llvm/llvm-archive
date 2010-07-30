srcdir := $(shell pwd)/src

CC_UNDER_TEST := /Developer/usr/bin/clang
CXX_UNDER_TEST := /Developer/usr/bin/clang++

check-%:
	rm -rf obj-$*
	lnt runtest nt \
		--sandbox obj-$* \
		--cc $(CC_UNDER_TEST) \
		--cxx $(CXX_UNDER_TEST) \
		--llvm-src ~/llvm \
		--llvm-obj ~/llvm.obj.64 \
		--test-suite ~/llvm.ref/test-suite.src \
		--test-externals ~/llvm/projects/test-suite-externals \
		lc-clang_qa-$* \
		--arch $* --small --simple -j 6 \
		--no-timestamp --no-configure
.PHONY: check-%

check:
	$(MAKE) -k check-i386
	$(MAKE) -k check-x86_64
.PHONY: check

save-results:
	if [ -z "$(VERSION)" ]; then \
	  echo "usage: make $@ VERSION=107"; \
	  exit 1; \
	fi
	rm -f results/*/clang-$(VERSION).*
	cp obj-i386/build/report.json results/i386/clang-$(VERSION).json
	cp obj-x86_64/build/report.json results/x86_64/clang-$(VERSION).json
.PHONY: save-results
