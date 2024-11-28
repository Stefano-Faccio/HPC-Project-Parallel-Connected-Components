# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -Werror -O2 -fopenmp

# Target executable
TARGET = parallel_connected_components.out

# Source files
SRCS = main.cpp input/parse_input.cpp serial_approaches/bfs.cpp serial_approaches/dfs.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean