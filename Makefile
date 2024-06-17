# g++ specifications etc
include ./Makefile.variables

# where we store .o files
OBJDIR := ./bin

# name of the exe 
EXEC := crisp

# phony targets (targets that don't represent actual files)
.PHONY: all clean

# compile all .cpp files into .o files and put them in $(OBJDIR)
all:
	$(MAKE) -C src all
	$(MAKE) -C scan all
	$(MAKE) -C parse all
	$(MAKE) -C error all
	$(MAKE) crisp

# link all object files together into executable $(EXEC)
crisp: $(wildcard $(OBJDIR)/*.o)
	$(CXX) $^ -o $(EXEC)

# remove all .o files and the executable
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(EXEC)