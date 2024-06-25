# g++ specifications etc
include ../Makefile.variables

# where we store .o files
OBJDIR := ../bin

# list of .cpp files in this directory
SOURCES := $(wildcard *.cpp)

# list of .o files in $(OBJDIR) from the .cpp files
OBJECTS := $(SOURCES:%.cpp=$(OBJDIR)/%.o)

# target to build all object files
all: $(OBJECTS)

# rule to compile each .cpp file into .o file
# $< represents the first dependency (%.cpp)
# $@ represents the target (%.o)
$(OBJDIR)/%.o: %.cpp
	clang++ $(CXXFLAGS) -c $< -o $@