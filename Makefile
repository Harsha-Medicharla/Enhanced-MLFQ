# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Target executable
TARGET = mlfq
SOURCE = MLFQ_2PROCESSOR.cpp

# Build rule
$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

# Clean rule
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: clean