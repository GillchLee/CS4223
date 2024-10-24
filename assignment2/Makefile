# Makefile

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -g

# Source and target names
SRC = coherence.cpp
TARGET = coherence

# Default rule to build the executable
all: $(TARGET)

# Compile the source file into an executable
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# Run the program with 5 command-line arguments
run: $(TARGET)
	./$(TARGET) $(arg1) $(arg2) $(arg3) $(arg4) $(arg5)

# Clean rule to remove the compiled executable
clean:
	rm -f $(TARGET)
