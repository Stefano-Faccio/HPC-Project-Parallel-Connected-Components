CXX = mpiCC # g++
CXXFLAGS = -std=c++11 -O2 -fopenmp -Wall -pedantic -Wunused
#
#-Werror: every warning is treated as an error.
#-Wextra 
FILENAME = parallel_cc.cpp #serial_cc.cpp
SRC = $(FILENAME) $(wildcard utils/*.cpp)   
OBJ = $(SRC:.cpp=.o)
TARGET = connected_components.out

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
#bear -- make

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean