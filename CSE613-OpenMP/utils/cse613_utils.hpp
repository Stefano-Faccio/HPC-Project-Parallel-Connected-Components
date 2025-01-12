#pragma once

//OpenMP header
#include <omp.h>
//Standard libraries
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>
//Custom libraries
#include "Edge.hpp"

using namespace std;

// Used in both randomized_cc.cpp and deterministic_cc.cpp
vector<Edge> find_rank_and_remove_edges(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels);

// Used in randomized_cc.cpp
void map_results_back(uint32_t nNodes, const vector<Edge>& edges, const vector<uint32_t>& labels, vector<uint32_t>& map);

// Used in deterministic_cc.cpp
void find_roots(uint32_t nNodes, vector<uint32_t>& labels);