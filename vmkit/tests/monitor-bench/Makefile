ifndef J3_ROOT_PATH
    J3_ROOT_PATH := /home/koutheir/phd/vmkit/vmkit-monitor
endif

ifndef BENCHMARK_NAMES
    BENCHMARK_NAMES := MethodCallBenchmark SmallObjectsBenchmark
endif

ifdef DEBUG
    BUILD_MODE := Debug+Asserts
else
    BUILD_MODE := Release+Asserts
endif

ifndef BENCHMARK_REPEATS
    BENCHMARK_REPEATS := 4
endif

ifndef VERBOSE
    QUIET := @
endif

BENCHMARK_CLASSPATH := $(abspath $(PWD)/bin)
J3_PATH      := $(abspath $(J3_ROOT_PATH)/$(BUILD_MODE)/bin/j3)
WORK_DIR      := $(HOME)/vmkit_monitor_benchmarks
TMP_DIR       := $(shell mkdir -p $(WORK_DIR) 2>/dev/null ; mktemp -d --suffix=.monitor-bench --tmpdir=$(WORK_DIR))
DEPS          := $(shell for i in $(BENCHMARK_NAMES); do seq -f "instancesOfBenchmark_%g_$$i" $(BENCHMARK_REPEATS); done)

all: _PRINT_TMP_DIR $(DEPS)

_PRINT_TMP_DIR:
	$(QUIET) echo "Target dir: " $(TMP_DIR)

$(DEPS):: $(BENCHMARK_CLASSPATH)/MonitorBench/Main.class

instancesOfBenchmark_%:
	$(eval BENCH_COUNT_$@ := $$(shell echo $@ | cut -d '_' -f 2))
	$(eval BENCH_NAME_$@ := $$(shell echo $@ | cut -d '_' -f 3-))
	$(eval TEMP_DIR_$@ := $(TMP_DIR)/$(BENCH_NAME_$@)/$(BENCH_COUNT_$@))
	$(QUIET) mkdir -p "$(TEMP_DIR_$@)" 2>/dev/null
	$(QUIET) cd "$(TEMP_DIR_$@)" && "$(J3_PATH)" -cp "$(BENCHMARK_CLASSPATH)" "MonitorBench.Main" "$(BENCH_NAME_$@)" > "$(TMP_DIR)/$(BENCH_NAME_$@).$(BENCH_COUNT_$@).log" 2>&1

clean:
	$(QUIET) -rm -rf $(WORK_DIR)/tmp.* $(WORK_DIR)/*.log 2>/dev/null
