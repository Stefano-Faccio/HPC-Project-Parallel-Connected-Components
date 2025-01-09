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
#include <numeric>
//Custom libraries
#include "utils/Edge.hpp"
#include "utils/MPIEdge.hpp"
#include "utils/GraphInputIterator.hpp"
#include "utils/mpi_parallel_cc_utils.hpp"

using namespace std;

vector<uint32_t>& par_MPI_master_deterministic_cc(int rank, int group_size, uint32_t nNodes, uint32_t nEdges, const vector<Edge>& edges, vector<uint32_t>& labels, int* iteration);