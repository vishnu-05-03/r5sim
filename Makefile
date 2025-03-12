#-----------------------------------------------------
# R5SIM Project Makefile
#-----------------------------------------------------

# Compiler settings
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -Werror -O2 -Wno-type-limits -Wno-maybe-uninitialized -Wno-format -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-label -Wno-unused-value -Wno-unused-local-typedefs
CXXFLAGS = $(CFLAGS) -std=c++11
DEBUG_FLAGS = -g -DDEBUG

# Directory structure
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
TEST_DIR = tests
BIN_DIR = bin

# Source files (automatically find all .c and .cpp files in src directory)
C_SOURCES := $(shell find $(SRC_DIR) -name "*.c")
CXX_SOURCES := $(shell find $(SRC_DIR) -name "*.cpp")
TEST_SOURCES := $(shell find $(TEST_DIR) -name "*.c" -o -name "*.cpp")

# Object files
C_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
CXX_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CXX_SOURCES))

C_SOURCES += main.c
C_OBJECTS += $(BUILD_DIR)/main.o


OBJECTS := $(C_OBJECTS) $(CXX_OBJECTS)

# Test object files
TEST_OBJECTS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/test_%.o,$(filter %.c,$(TEST_SOURCES))) \
                $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/test_%.o,$(filter %.cpp,$(TEST_SOURCES)))

# Main executable
TARGET = $(BIN_DIR)/r5sim

# Include paths
INCLUDES = -I$(INCLUDE_DIR) -L/usr/lib/x86_64-linux-gnu

# Default target
all: makedirs $(TARGET)

# Create necessary directories
makedirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR) $(dir $(OBJECTS))

# Main executable build
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lyaml

# Compile C source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile C++ source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile test source files
$(BUILD_DIR)/test_%.o: $(TEST_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/test_%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

build/main.o: main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Build test executables
tests: makedirs $(TEST_OBJECTS) $(OBJECTS)
	@mkdir -p $(BIN_DIR)/tests
	@for test_obj in $(TEST_OBJECTS); do \
		test_name=$$(basename $$test_obj .o | sed 's/^test_//'); \
		$(CXX) $(CXXFLAGS) $$test_obj $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS)) -o $(BIN_DIR)/tests/$$test_name; \
	done

# Run all tests
test: tests
	@for test in $(BIN_DIR)/tests/*; do \
		echo "Running $$test"; \
		$$test; \
	done

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: all

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Install (adjust as needed)
install: all
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin

.PHONY: all clean test tests debug install makedirs