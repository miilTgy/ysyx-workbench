VSRCS = "./vsrc/example.v" 
CSRCS = "./csrc/main.cpp"
VERILATOR = verilator
all:
	@echo "Write this Makefile by your self."
build:
	$(VERILATOR) --cc --exe --build -j 0 -Wall --trace $(CSRCS) $(VSRCS)
sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	#@echo "Write this Makefile by your self."
	echo "Building..."
	echo $(VERILATOR) --cc --exe --build -j 0 -Wall --trace $(CSRCS) $(VSRCS)
	echo "Starting Simulation..."
	./obj_dir/Vexample
	echo "Simulation Done."
include ../Makefile
