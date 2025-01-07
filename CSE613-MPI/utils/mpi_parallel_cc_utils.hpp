#include <mpi.h>
//Standard libraries
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>
//Custom libraries
#include "Edge.hpp"
#include "MPIEdge.hpp"

// Function to get the number of edges to send to every processor
vector<int> calculate_edges_per_processor(int rank, int group_size, const vector<Edge>& edges);

