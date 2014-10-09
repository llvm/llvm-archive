
ifdef DEBUG
    BUILD_MODE := Debug+Asserts
else
    BUILD_MODE := Release+Asserts
endif

ifndef BENCHMARK_REPEATS
    BENCHMARK_REPEATS := 12
endif

ifndef VERBOSE
    QUIET := @
endif

DACAPO_JAR    := $(abspath $(PWD)/$(LEVEL)/jars/dacapo-$(DACAPO_VERSION).jar)
JVM_PATH      := $(abspath $(PWD)/$(LEVEL)/../../$(BUILD_MODE)/bin/j3)
WORK_DIR      := $(HOME)/dacapo_benchmarks
TMP_DIR       := $(shell mkdir -p $(WORK_DIR) 2>/dev/null ; mktemp -d --suffix=.$(DACAPO_VERSION) --tmpdir=$(WORK_DIR))
DEPS          := $(shell for i in $(DECAPO_BENCHMARKS); do seq -f "dacapoBenchmark_%g_$$i" $(BENCHMARK_REPEATS); done)

all: _PRINT_TMP_DIR $(DEPS)

_PRINT_TMP_DIR:
	$(QUIET) echo "Target dir: " $(TMP_DIR)

$(DEPS):: $(DACAPO_JAR)

$(DACAPO_JAR):
	$(QUIET) wget "http://sourceforge.net/projects/dacapobench/files/archive/2006-10-MR2/dacapo-2006-10-MR2.jar/download" -O "$(DACAPO_JAR)"

dacapoBenchmark_%:
	$(eval BENCH_COUNT_$@ := $$(shell echo $@ | cut -d '_' -f 2))
	$(eval BENCH_NAME_$@ := $$(shell echo $@ | cut -d '_' -f 3-))
	$(eval TEMP_DIR_$@ := $(TMP_DIR)/$(BENCH_NAME_$@)/$(BENCH_COUNT_$@))
	$(QUIET) mkdir -p "$(TEMP_DIR_$@)" 2>/dev/null
	$(QUIET) cd "$(TEMP_DIR_$@)" && "$(JVM_PATH)" -jar "$(DACAPO_JAR)" $(DACAPO_ARGS) "$(BENCH_NAME_$@)" > "$(TMP_DIR)/$(BENCH_NAME_$@).$(BENCH_COUNT_$@).log" 2>&1

clean:
	$(QUIET) -rm -rf $(WORK_DIR)/tmp.* $(WORK_DIR)/*.log 2>/dev/null
