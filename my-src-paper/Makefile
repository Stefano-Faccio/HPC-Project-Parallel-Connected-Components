#Possible compiler: g++ / clang 
#CXX = mpiCC 
CXX = /apps/mpich-3.2/bin/mpicc
BOOST_PATH = ../boost_1_87_0/
CXXFLAGS = -std=c++11 -O2 -fopenmp -Wall -pedantic -I./$(BOOST_PATH) 
#-Werror: every warning is treated as an error. -Wextra

#FILENAME = serial_cc.cpp
FILENAME = parallel_cc.cpp
SRC = $(FILENAME) $(wildcard utils/*.cpp)   
OBJDIR = obj
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))
#$(SRC:.cpp=.o)
TARGET = $(basename $(FILENAME)).out

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)/utils
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean