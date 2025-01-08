#include <mpi.h>
//Standard libraries
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <utility>
//Custom libraries
#include "Edge.hpp"
#include "MPIEdge.hpp"

// Function to get the number of edges to send to every processor
vector<int> calculate_edges_per_processor(int rank, int group_size, const vector<Edge>& edges);
// Function to get the displacements for the scatterv function
vector<int> calculate_displacements(int rank, int group_size, const vector<int>& edges_per_processor);
// Function to get the number of edges to send to every processor and the displacements for the scatterv function
pair<vector<int>, vector<int>> calculate_edges_per_processor_and_displacements(int rank, int group_size, const vector<Edge>& edges);

