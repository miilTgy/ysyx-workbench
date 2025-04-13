TOP ?= top
PREFIX ?= ysyx_25010001_

BUILD_PATH = obj_dir/
VERILATOR_CFLAGS += -MMD -O3 --x-assign 1 --x-initial unique -cc --trace -j 16
VERILATOR_TB += $(shell find $(abspath ./csrc) -name "tb_$(TOP).cpp")
VERILATOR_TOP = --top-module $(PREFIX)$(TOP)
VERILATOR_EXE = obj_dir/V$(PREFIX)$(TOP)

IMG ?=
DIFF ?= -d /home/miil/ysyx-workbench/nemu/build/riscv64-nemu-interpreter-so 

CSRC += $(shell find $(abspath ./csrc) -name "*.cpp")
VSRC += $(filter-out $(abspath ./vsrc/chisel)/%.v, $(shell find $(abspath ./vsrc) -name "*.v"))
VSRC += $(shell find $(abspath ./vsrc) -name "*.sv")

.PHONY: sim genc clean

all:
	@echo "Write this Makefile by your self."

sim: $(VERILATOR_TB)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "sim vsrc ="
	@$(foreach file, $(VSRC), echo "	$(file)";)
	@echo "exe ="
	@$(foreach file, $(VERILATOR_TB), echo "	$(file)";)
	verilator $(VERILATOR_CFLAGS) $(VERILATOR_TOP) $(VSRC) \
		-exe --build $(VERILATOR_TB)
	time $(VERILATOR_EXE) $(DIFF) $(IMG) +verilator+rand+reset+1

genc: $(shell find $(abspath ./vsrc) -name "$(PREFIX)$(TOP).v") clean
	verilator $(VERILATOR_CFLAGS) $(VERILATOR_TOP) $(VSRC)

clean:
	rm -rf ./obj_dir
include ../Makefile
