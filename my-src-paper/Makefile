CXX = mpiCC # g++
CXXFLAGS = -std=c++11 -O2 -fopenmp -Wall -pedantic
#-Werror: every warning is treated as an error.
#-Wextra 
SRC = parallel_cc.cpp $(wildcard utils/*.cpp)   #serial_cc.cpp
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