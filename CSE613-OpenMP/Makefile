HP_CLUSTER = 1

ifeq ($(HP_CLUSTER), 1)
# Compiler for HPC
CXX = /apps/gcc-9.1.0/local/bin/g++-9.1.0
else
# Compiler for local machine
CXX = g++
endif

BOOST_PATH = ../boost_1_83_0/
CXXFLAGS = -std=c++11 -O2 -fopenmp -Wall -g -pedantic -I./$(BOOST_PATH) 

#Deterministic and randomized version
FILENAME_DETE = deterministic_cc.cpp
FILENAME_RAND = randomized_cc.cpp
SRC_DETE =  $(wildcard utils/*.cpp) $(FILENAME_DETE) 
SRC_RAND =  $(wildcard utils/*.cpp) $(FILENAME_RAND) 
TARGET_DETE = $(basename $(FILENAME_DETE)).out
TARGET_RAND = $(basename $(FILENAME_RAND)).out

#Object files
OBJDIR = obj
OBJ_DETE = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC_DETE))
OBJ_RAND = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC_RAND))

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

all: $(TARGET_DETE) $(TARGET_RAND)

$(TARGET_DETE): $(OBJ_DETE)
	@echo "Linking $(PURPLE)$@$(NC)"
	$(CXX) $(CXXFLAGS) $(OBJ_DETE) -o $(TARGET_DETE)
	@echo "$(GREEN)[ DONE ]$(NC)"

$(TARGET_RAND): $(OBJ_RAND)
	@echo "Linking $(PURPLE)$@$(NC)"
	$(CXX) $(CXXFLAGS) $(OBJ_RAND) -o $(TARGET_RAND)
	@echo "$(GREEN)[ DONE ]$(NC)"

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)/utils
	@echo "Compiling $(YELLOW)$@$(NC)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(RED)Cleaning old compiled files$(NC)"
	rm -f $(OBJ_DETE) $(OBJ_RAND) $(TARGET_DETE) $(TARGET_RAND)

.PHONY: all clean