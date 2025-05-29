# Makefile for MLFQ Scheduler
# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g

# Target and source files
TARGET = mlfq
SOURCE = MLFQ_2PROCESSOR.cpp
HEADER = MLFQ_2PROCESSOR.h

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCE) $(HEADER)
	@echo "Compiling MLFQ Scheduler..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)
	@echo "Build successful! Executable: $(TARGET)"

# Run the program
run: $(TARGET)
	@echo "Running MLFQ Scheduler..."
	./$(TARGET)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGET) output.txt
	@echo "Clean complete!"

# Debug build
debug: CXXFLAGS += -DDEBUG -g3
debug: $(TARGET)
	@echo "Debug build complete!"

# Release build
release: CXXFLAGS = -std=c++17 -O3 -DNDEBUG
release: $(TARGET)
	@echo "Release build complete!"

# Install (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/"

# Uninstall (optional)
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled from /usr/local/bin/"

# Help
help:
	@echo "Available targets:"
	@echo "  all     - Build the program (default)"
	@echo "  run     - Build and run the program"
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Build with debug symbols"
	@echo "  release - Build optimized release version"
	@echo "  help    - Show this help message"

# Check if compiler exists
check:
	@which $(CXX) > /dev/null || (echo "Error: $(CXX) not found!" && exit 1)
	@echo "Compiler check passed: $(CXX) found"

# Phony targets
.PHONY: all clean run debug release install uninstall help check