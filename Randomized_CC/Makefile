#CXX = g++-14
# Compiler for HPC
CXX = /apps/gcc-9.1.0/local/bin/g++-9.1.0

BOOST_PATH = ../boost_1_83_0/
CXXFLAGS = -std=c++11 -O2 -fopenmp -Wall -g -pedantic -I./$(BOOST_PATH) 

#Serial version
FILENAME_SERIAL = randomized_cc.cpp
#FILENAME_SERIAL = test.cpp
SRC_SERIAL = $(FILENAME_SERIAL) 
TARGET_SERIAL = $(basename $(FILENAME_SERIAL)).out

#Parallel version

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

all: $(TARGET_SERIAL)

$(TARGET_SERIAL): $(OBJ_SERIAL)
	@echo "Linking $(PURPLE)$@$(NC)"
	$(CXX) $(CXXFLAGS) $(OBJ_SERIAL) -o $(TARGET_SERIAL)
	@echo "$(GREEN)[ DONE ]$(NC)"

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)/utils
	@echo "Compiling $(YELLOW)$@$(NC)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(RED)Cleaning old compiled files$(NC)"
	rm -f $(OBJ_SERIAL) $(TARGET_SERIAL)

.PHONY: all clean