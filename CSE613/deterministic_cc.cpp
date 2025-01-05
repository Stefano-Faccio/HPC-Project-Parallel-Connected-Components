//OpenMP header
#include <omp.h>
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
#include "../PPoPP_2018/utils/Edge.hpp"
#include "../PPoPP_2018/utils/GraphInputIterator.hpp"
#include "utils/cse613_utils.hpp"

using namespace std;

vector<uint32_t>& par_deterministic_cc(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels, int* iteration);


int main(int argc, char* argv[]) {	

	//---------------------- Read the graph ----------------------

	if (argc != 2 ) {
		cout << "Usage: connectivity INPUT_FILE" << endl;
		return 1;
	}

	//Open the file and read the number of vertices and edges
	GraphInputIterator input(argv[1]);
	cout << "Vertex count: " << input.vertexCount() << " Edge count: " << input.edgeCount() << endl;

	vector<Edge> edges;
	//edges.reserve(input.edgeCount()); // Probably not necessary
	uint32_t real_edge_count = 0;
	for (auto edge : input) {
		//Check that the edge is valid i.e. the nodes are in the graph
		assert(edge.from < input.vertexCount());
		assert(edge.to < input.vertexCount());
		//Check that the edge is not a self loop
		if (edge.to != edge.from) {
			edges.push_back(edge);
			real_edge_count++;
		}
	}

	//Check if self loops were removed
	if(real_edge_count != input.edgeCount())
		cout << "Warning: " << input.edgeCount() - real_edge_count << " self loops were removed" << endl;


	//---------------------- Compute CC ----------------------

	// Initialize the labels
	vector<uint32_t> labels(input.vertexCount());
	for(uint32_t i = 0; i < input.vertexCount(); i++) {
		labels[i] = i;
	}

	// Initialize the iteration counter
	int iteration = 0;

	//Start the timer
	auto start = chrono::high_resolution_clock::now();

	//Compute the connected components
	vector<uint32_t>& map = par_deterministic_cc(input.vertexCount(), edges, labels, &iteration);

	//Stop the timer
	auto end = chrono::high_resolution_clock::now();
	//Calculate the duration
	auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

	#if false
	//Print the labels at the end
	cout << "Labels at end: ";
	for (uint32_t i = 0; i < input.vertexCount(); i++) {
		cout  << map[i] << " ";
	}
	cout << endl;
	#endif

	//Count the number of connected components
	uint32_t number_of_cc = unordered_set<uint32_t>(map.begin(), map.end()).size();

	// Print the results
	cout << "Connected components: " << number_of_cc << endl;
	cout << "Execution time: " << duration.count() << " ms" << endl;
	cout << "Iterations: " << iteration << endl;

    return 0;
}

vector<uint32_t>& par_deterministic_cc(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels, int* iteration) {

	// Increment the iteration
	(*iteration)++;

	cout << "Iteration " << *iteration << " Number of edges: " << edges.size() << endl;

	// Base case
	if(edges.size() == 0 || nNodes == 0) 
		return labels;

	uint32_t hooks_small_2_large = 0, hooks_large_2_small = 0;

	#pragma omp parallel shared(nNodes, edges, labels) 
	{
		//Count hooks from smaller to larger indices, and vice versa
		#pragma omp for reduction(+:hooks_small_2_large) reduction(+:hooks_large_2_small)
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;

			if(from < to)
				hooks_small_2_large += 1;
			else if(from > to)
				hooks_large_2_small += 1;
			else
				cerr << "Self loop found: " << from << " " << to << endl;
		}

		//Choose hook direction to maximize #hooks
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;

			if(from < to)
			{
				if(hooks_small_2_large >= hooks_large_2_small)
					labels[from] = to;
			}
			else
			{
				if(hooks_small_2_large < hooks_large_2_small)
					labels[from] = to;
			}
		}
	}

	// Find the roots for every node
	find_roots(nNodes, labels);

	// Compute the new set of edges
	// Recursively call the function
	return par_deterministic_cc(nNodes, find_rank_and_remove_edges(nNodes, edges, labels), labels, iteration);
}