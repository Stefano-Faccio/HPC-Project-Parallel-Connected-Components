#Serial local compile 
#CXX = mpiCC 
#BOOST_PATH = ../boost_1_86_0/
#CXXFLAGS = -std=c++11 -O2 -fopenmp -Wall -g -pedantic -I./$(BOOST_PATH)
#FILENAME = serial_cc.cpp
#SRC = $(wildcard utils/DisjointSets.cpp) $(FILENAME) 

#Parallel local compile 
#CXX = mpiCC 
BOOST_PATH = ../boost_1_86_0/
CXXFLAGS = -std=c++11 -O2 -fopenmp -Wall -g -pedantic -I./$(BOOST_PATH) 
FILENAME = parallel_cc.cpp
SRC = $(wildcard utils/*.cpp) $(FILENAME)
#-Werror: every warning is treated as an error. -Wextra

#Parallel compile on cluster
CXX = /apps/mpich-3.2/bin/mpicxx

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