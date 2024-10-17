# Compiler settings
CC = gcc
CXX = g++
CFLAGS = -I../include -Wall
CXXFLAGS = -std=c++14 -I../include -Wall -I/usr/local/include

# Linker settings
LDFLAGS = -L/usr/local/lib -lgtest -pthread

# Directories
SRC_DIR = ../src
OBJ_DIR = obj
BIN_DIR = bin

# Explicit source files
SOURCES = $(SRC_DIR)/flash.c $(SRC_DIR)/gpNvm.c $(SRC_DIR)/hamming.c
TEST_SOURCES = $(wildcard *.cpp) $(wildcard *.c)

# Object files
OBJS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) $(TEST_SOURCES:%.cpp=$(OBJ_DIR)/%.o)

# Target executable
TARGET = $(BIN_DIR)/run_tests

# Default target
all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
