#CXX = /apps/mpich-3.2/bin/mpicxx #Compile on cluster
CXX = mpiCC #Compile on local machine

BOOST_PATH = ../boost_1_83_0/
#BOOST_PATH = ../boost_1_86_0/ #Having trouble with this version
CXXFLAGS = -std=c++11 -O2 -fopenmp -Wall -g -pedantic -I./$(BOOST_PATH) 

#Serial version
FILENAME_SERIAL = serial_cc.cpp
SRC_SERIAL = $(wildcard utils/DisjointSets.cpp) $(FILENAME_SERIAL) 
TARGET_SERIAL = $(basename $(FILENAME_SERIAL)).out

#Parallel version
FILENAME_PARALLEL = parallel_cc.cpp
SRC_PARALLEL = $(wildcard utils/*.cpp) $(FILENAME_PARALLEL)
TARGET_PARALLEL = $(basename $(FILENAME_PARALLEL)).out

#Object files
OBJDIR = obj
OBJ_SERIAL = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC_SERIAL))
OBJ_PARALLEL = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC_PARALLEL))

# olor codes
BLACK=\033[0;30m# Black
RED=\033[0;31m# Red
GREEN=\033[1;32m# Green
YELLOW=\033[0;33m# Yellow
BLUE=\033[0;34m# Blue
PURPLE=\033[1;35m# Purple
CYAN=\033[0;36m# Cyan
WHITE=\033[0;37m# White
NC = \033[0m# No Color

all: $(TARGET_SERIAL) $(TARGET_PARALLEL)

$(TARGET_SERIAL): $(OBJ_SERIAL)
	@echo "Linking $(PURPLE)$@$(NC)"
	$(CXX) $(CXXFLAGS) $(OBJ_SERIAL) -o $(TARGET_SERIAL)
	@echo "$(GREEN)[ DONE ]$(NC)"

$(TARGET_PARALLEL): $(OBJ_PARALLEL)
	@echo "Linking $(PURPLE)$@$(NC)"
	$(CXX) $(CXXFLAGS) $(OBJ_PARALLEL) -o $(TARGET_PARALLEL)
	@echo "$(GREEN)[ DONE ]$(NC)"

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)/utils
	@echo "Compiling $(YELLOW)$@$(NC)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(RED)Cleaning old compiled files$(NC)"
	rm -f $(OBJ_SERIAL) $(OBJ_PARALLEL) $(TARGET_SERIAL) $(TARGET_PARALLEL)

.PHONY: all clean