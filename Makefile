# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Wextra

# Source files
SRCS = token.cpp scanner.cpp parser.cpp main.cpp 

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable
EXEC = crisp

# Default target
all: $(EXEC)

# Link object files to create executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXEC)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target to remove object files and executable
clean:
	rm -f $(OBJS) $(EXEC)
