# command to build, run & clean riscv core executable

CXXFLAGS += -DDEBUG_ENABLE=0

build:
	g++ main.cpp 3_stage_pipeline.cpp
clean:
	rm *.out
run:
	./a.out $(assembly_file)
