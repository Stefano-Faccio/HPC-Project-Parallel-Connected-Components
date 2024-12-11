CXX = mpiCC # g++ / clang #OMPI_CC=clang 
CXXFLAGS = -std=c++11 -O2 -fopenmp -Wall -pedantic
#-Werror: every warning is treated as an error. -Wextra

FILENAME = parallel_cc.cpp #serial_cc.cpp
SRC = $(FILENAME) $(wildcard utils/*.cpp)   
OBJDIR = obj
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))
#$(SRC:.cpp=.o)
TARGET = connected_components.out

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)/utils
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean