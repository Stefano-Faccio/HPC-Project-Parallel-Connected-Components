#pragma once

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
vector<int> calculate_edges_per_processor(int group_size, const vector<Edge>& edges);
// Function to get the displacements for the scatterv / gatherv functions
vector<int> calculate_displacements(int group_size, const vector<int>& edges_per_processor);
// Function to choose the hook direction
void choose_hook_direction(const vector<Edge>& edges, vector<uint32_t>& labels);
// Function to find the roots for every node
void find_roots(uint32_t nNodes, vector<uint32_t>& labels);
// Function to compute the next edges
vector<Edge> compute_next_edges(const vector<Edge>& edges, const vector<uint32_t>& labels);