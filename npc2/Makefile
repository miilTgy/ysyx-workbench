
SHELL := /bin/bash

BUILD_FILES := build.sc
CHISEL_SRCS := $(shell find chisel -type f)
RVDECODER_SRCS := $(shell find rvdecoderdb/rvdecoderdb -type f)
GENV_DEPS := $(BUILD_FILES) $(CHISEL_SRCS) $(RVDECODER_SRCS)

.PHONY: genv
genv: $(GENV_DEPS)
	mill -i CPU.runMain Main
