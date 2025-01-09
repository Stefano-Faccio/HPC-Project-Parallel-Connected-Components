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
// Function to count the number of hooks
pair<uint32_t, uint32_t> count_hooks(const vector<Edge>& edges);
// Function to choose the hook direction
void choose_hook_direction(const vector<Edge>& edges, uint32_t hooks_small_2_large, uint32_t hooks_large_2_small, vector<uint32_t>& labels);
// Function to find the roots for every node
void find_roots(uint32_t nNodes, vector<uint32_t>& labels);
// Function to mark new edges
vector<uint32_t> mark_new_edges(const vector<Edge>& edges, const vector<uint32_t>& labels);
// Function to compute the prefix sum of the marked edges
vector<uint32_t> compute_prefix_sum(int rank, int group_size, uint32_t nEdges, uint32_t nEdges_local, const vector<uint32_t>& marked_edges);
// Function to compute the next edges
vector<Edge> compute_next_edges(const vector<Edge>& edges, const vector<uint32_t>& labels, const vector<uint32_t>& prefix_sum)