#include <mpi.h>
//Standard libraries
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <unordered_set>
//Custom libraries
#include "Edge.hpp"
#include "MPIEdge.hpp"
#include "GraphInputIterator.hpp"
#include "mpi_parallel_cc_utils.hpp"

void par_MPI_slave_deterministic_cc(int rank, int group_size, uint32_t nNodes);