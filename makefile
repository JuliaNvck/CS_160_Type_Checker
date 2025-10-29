CXX = g++
# Use C++17 standard for features like std::optional
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-unused-parameter -pedantic -g
# Add AddressSanitizer flags
# CXXFLAGS += -fsanitize=address
# LDFLAGS = -fsanitize=address

# Name of the final executable
TARGET = type

# Source files
SRCS = typechecker.cpp ast.cpp
# Object files derived from source files
OBJS = $(SRCS:.cpp=.o)

# Default target: build the executable
all: $(TARGET)

# Rule to link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile .cpp files into .o files
# Added json.hpp as a dependency for ast.o as well, just in case
%.o: %.cpp ast.hpp json.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to clean up generated files
clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean