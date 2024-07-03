# g++ specifications etc
include ./Makefile.variables

# where we store .o files
OBJDIR := ./bin

# name of the exe 
EXEC := crisp

# tells clang++ to use the lld linker instead of the default linker 
USELLD := -fuse-ld=lld

# specifies all llvm directories llvm libs reside in w flag -L 
# -L: specify additional directories where the linker should look libs with -l flag
LLVMLINKFLAG := $(shell llvm-config --ldflags)

# links all llvm libs used w flag -l
# -l: used during the linking phase to specify a lib to link against
# added -lz bc linker was crying ab missing zlib1g-dev symbols
LLVMLIBS := $(shell llvm-config --libs) -lz

# phony targets (targets that don't represent actual files)
.PHONY: all clean

# compile all .cpp files into .o files and put them in $(OBJDIR)
all:
	$(MAKE) -C src all
	$(MAKE) -C scan all
	$(MAKE) -C parse all
	$(MAKE) -C error all
	$(MAKE) -C emitIR all
	$(MAKE) crisp

# link all object files together into executable $(EXEC)
# $^ expands to these .o files
crisp: $(wildcard $(OBJDIR)/*.o)
	$(CXX) $(CXXFLAGS) $(LLVMLINKFLAG) $(USELLD) $^ -o $(EXEC) $(LLVMLIBS)

# remove all .o files and the executable
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(EXEC)