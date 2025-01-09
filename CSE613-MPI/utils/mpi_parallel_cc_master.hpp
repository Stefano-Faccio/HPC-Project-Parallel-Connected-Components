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

vector<uint32_t>& par_MPI_master_deterministic_cc(int rank, int group_size, uint32_t nNodes, uint32_t nEdges, const vector<Edge>& edges, vector<uint32_t>& labels, int* iteration);