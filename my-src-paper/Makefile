CXX = g++
CXXFLAGS = -std=c++11 -Wall -Werror -O2 -fopenmp
SRC = $(wildcard utils/*.cpp) serial_cc.cpp 
OBJ = $(SRC:.cpp=.o)
TARGET = connected_components.out

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean