# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -g

# Target executable name
TARGET = Cache_ex

# Source files
SRCS = Cache_ex.cpp

# Object files (same as source, but .o instead of .cpp)
OBJS = $(SRCS:.cpp=.o)

# Rule to build the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile the source file into an object file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove generated files
clean:
	rm -f $(OBJS) $(TARGET)
