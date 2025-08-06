# NSP Optimization Project Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -Isrc/core -Isrc/constraints -Isrc/metaheuristics -Isrc/utils

# Directories
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build
BIN_DIR = bin

# Source files
CORE_SOURCES = $(wildcard $(SRC_DIR)/core/*.cpp)
CONSTRAINT_SOURCES = $(wildcard $(SRC_DIR)/constraints/*.cpp)
META_SOURCES = $(wildcard $(SRC_DIR)/metaheuristics/*.cpp)
UTILS_SOURCES = $(wildcard $(SRC_DIR)/utils/*.cpp)
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)

# Object files
CORE_OBJECTS = $(CORE_SOURCES:$(SRC_DIR)/core/%.cpp=$(BUILD_DIR)/core/%.o)
CONSTRAINT_OBJECTS = $(CONSTRAINT_SOURCES:$(SRC_DIR)/constraints/%.cpp=$(BUILD_DIR)/constraints/%.o)
META_OBJECTS = $(META_SOURCES:$(SRC_DIR)/metaheuristics/%.cpp=$(BUILD_DIR)/metaheuristics/%.o)
UTILS_OBJECTS = $(UTILS_SOURCES:$(SRC_DIR)/utils/%.cpp=$(BUILD_DIR)/utils/%.o)
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/tests/%.o)

# Targets
.PHONY: all clean test directories original

all: directories $(BIN_DIR)/nsp_optimized $(BIN_DIR)/test_runner

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR)/core $(BUILD_DIR)/constraints $(BUILD_DIR)/metaheuristics $(BUILD_DIR)/utils $(BUILD_DIR)/tests $(BIN_DIR)

# Original main.cpp compilation (for comparison)
original: main.cpp
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/nsp_original main.cpp

# Test runner
$(BIN_DIR)/test_runner: $(CORE_OBJECTS) $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Main optimized NSP program (placeholder for now)
$(BIN_DIR)/nsp_optimized: $(CORE_OBJECTS) main_optimized.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ main_optimized.cpp $(CORE_OBJECTS)

# Core object files
$(BUILD_DIR)/core/%.o: $(SRC_DIR)/core/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Constraint object files
$(BUILD_DIR)/constraints/%.o: $(SRC_DIR)/constraints/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Metaheuristic object files
$(BUILD_DIR)/metaheuristics/%.o: $(SRC_DIR)/metaheuristics/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Utils object files
$(BUILD_DIR)/utils/%.o: $(SRC_DIR)/utils/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Test object files
$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Run tests
test: $(BIN_DIR)/test_runner
	./$(BIN_DIR)/test_runner

test-all: $(BIN_DIR)/test_runner
	./$(BIN_DIR)/test_runner --all

# Run original version with Instance1 for comparison
test-original: original
	./$(BIN_DIR)/nsp_original Instance1.txt 1000

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build optimized NSP and test runner"
	@echo "  original     - Build original main.cpp"
	@echo "  test         - Run basic tests"
	@echo "  test-all     - Run all tests"
	@echo "  test-original- Run original version with Instance1"
	@echo "  clean        - Remove build files"
	@echo "  help         - Show this help"