# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11 -g

# Directories
INCLUDE_DIR = include
SRC_DIR = src

# Source files
SRCS = $(SRC_DIR)/HTTPObject.cpp $(SRC_DIR)/Server.cpp $(SRC_DIR)/Connection.cpp $(SRC_DIR)/utils.cpp main.cpp 

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = output

# Default target
all: $(EXEC)

# Link the executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDE_DIR)/%.h
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets
.PHONY: all clean


