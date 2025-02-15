HP_CLUSTER = 1

ifeq ($(HP_CLUSTER), 1)
# Compiler for HPC
CXX = /apps/mpich-3.2/bin/mpicxx
else
# Compiler for local machine
CXX = mpiCC
endif

BOOST_PATH = ../boost_1_83_0/
CXXFLAGS = -std=c++11 -O2 -fopenmp -Wall -g -pedantic -I./$(BOOST_PATH) 

#Parallel version
FILENAME_PARALLEL = my_MPI_cc.cpp
SRC_PARALLEL = $(wildcard utils/*.cpp) $(FILENAME_PARALLEL)
TARGET_PARALLEL = $(basename $(FILENAME_PARALLEL)).out

#Object files
OBJDIR = obj
OBJ_PARALLEL = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC_PARALLEL))

# Color codes
BLACK=\033[0;30m# Black
RED=\033[0;31m# Red
GREEN=\033[1;32m# Green
YELLOW=\033[0;33m# Yellow
BLUE=\033[0;34m# Blue
PURPLE=\033[1;35m# Purple
CYAN=\033[0;36m# Cyan
WHITE=\033[0;37m# White
NC = \033[0m# No Color

# If HP_CLUSTER is 1, then clear all the color codes
ifeq ($(HP_CLUSTER), 1)
BLACK=
RED=
GREEN=
YELLOW=
BLUE=
PURPLE=
CYAN=
WHITE=
NC=
endif

all: $(TARGET_PARALLEL)

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
	rm -f $(OBJ_PARALLEL) $(TARGET_PARALLEL)

.PHONY: all clean