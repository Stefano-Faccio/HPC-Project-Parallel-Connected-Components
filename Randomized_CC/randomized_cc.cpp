//OpenMP header
#include <omp.h>
//Standard libraries
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <random>
#include <chrono>
#include <cassert>
#include <algorithm>
#include <unordered_set>
//Custom libraries
#include "../PPoPP_2018/utils/Edge.hpp"
#include "../PPoPP_2018/utils/GraphInputIterator.hpp"

using namespace std;

//#define DEBUG

// Random number generator
static uint32_t seed;
#pragma omp threadprivate(seed)

void coin_toss_and_child_hook(uint32_t nNodes, vector<Edge>& edges, vector<uint32_t>& labels) {

	// Hook child to a parent based on the coin toss		
	vector<bool> coin_toss(nNodes);

	#pragma omp parallel shared(nNodes, edges, labels, coin_toss) 
	{	
		// Generate random coin tosses
		#pragma omp for
		for (uint32_t i = 0; i < nNodes; i++) {
			coin_toss[i] = rand_r(&seed) % 2;
		}

		// Hook child to a parent based on the coin toss
		//for(auto it = edges.begin(); it < edges.end(); it++)
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			// Tail is True and Head is False
			if(coin_toss[edges[i].from] && !coin_toss[edges[i].to])
			{
				// Note: race condition here
				#pragma omp critical (hook_child)
				labels[edges[i].from] = labels[edges[i].to];
			}
			else if(!coin_toss[edges[i].from] && coin_toss[edges[i].to])
			{
				#pragma omp critical (hook_child)
				labels[edges[i].to] = labels[edges[i].from];
			}
		}
	}
}

vector<Edge> find_rank_and_remove_edges(uint32_t nNodes, vector<Edge>& edges, vector<uint32_t>& labels)
{
	vector<uint32_t> s(edges.size(), 0), S(edges.size());
	// Vector to store the next edges for the recursive call
	vector<Edge> nextEdges;

	#pragma omp parallel shared(nNodes, edges, labels, s, S, nextEdges) 
	{
		// Prepare to remove edges inside the same group
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			if(labels[edges[i].from] != labels[edges[i].to])
				s[i] = 1;
			else
				s[i] = 0;
		}

		// Prefix sum TODO: parallelize
		#pragma omp single
		{
			// Prefix sum
			S[0] = s[0];
			for(uint32_t i = 1; i < edges.size(); i++)
			{
				S[i] = s[i] + S[i - 1];
			}
		}

		#pragma omp single
		{
			// Find the rank
			nextEdges.resize(S[edges.size() - 1]);
		}

		// Copy only edges that are between different groups
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			if(labels[edges[i].from] != labels[edges[i].to])
			{
				int index = S[i] - 1;
				Edge edge = {labels[edges[i].from], labels[edges[i].to]};

				#pragma omp critical (copy_edge)
				{
					nextEdges[index] = edge;
				}
			}				
		}
	}

	return nextEdges;
}

void par_randomized_cc(uint32_t nNodes, vector<Edge>& edges, vector<uint32_t>& labels, int* iteration) {

	// Increment the iteration
	(*iteration)++;

	cout << "Iteration " << *iteration << " Number of edges: " << edges.size() << endl;

	// Base case
	if(edges.size() == 0 || nNodes == 0) 
		return;
		
	// Coin toss and child hook
	coin_toss_and_child_hook(nNodes, edges, labels);

	// Find the rank 
	vector<Edge> nextEdges = find_rank_and_remove_edges(nNodes, edges, labels);

	#ifdef DEBUG
	//Print the labels
	cout << "Labels at iteration " << *iteration << ": " << endl;
	for (uint32_t i = 0; i < nNodes; i++) {
		cout << i << " ";
	}
	cout << endl;
	for (uint32_t i = 0; i < nNodes; i++) {
		cout << labels[i] << " ";
	}
	cout << endl << endl;
	#endif

	// Recursively call the function
	par_randomized_cc(nNodes, nextEdges, labels, iteration);

	//Map results back to the original graph
	#pragma omp parallel for shared(edges, labels)
	for(uint32_t i = 0; i < edges.size(); i++)
	{
		if(edges[i].to == labels[edges[i].from])
		{
			#pragma omp critical
			{
				labels[edges[i].from] = labels[edges[i].to];
			}
		}
		else if(edges[i].from == labels[edges[i].to])
		{
			#pragma omp critical
			{
				labels[edges[i].to] = labels[edges[i].from];
			}
		}
	}

	return;
}

int main(int argc, char* argv[]) {

    if (argc != 2 ) {
		cout << "Usage: connectivity INPUT_FILE" << endl;
		return 1;
	}

	// Configure the RNG: seed is private to each thread and persistent across calls
    #pragma omp parallel
    {
        seed = 25234 + 17 * omp_get_thread_num();
    }

	//Open the file and read the number of vertices and edges
	GraphInputIterator input(argv[1]);
	cout << "Vertex count: " << input.vertexCount() << " Edge count: " << input.edgeCount() << endl;

	vector<Edge> edges;
	edges.reserve(input.edgeCount());
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
	//assert(real_edge_count == input.edgeCount());
	if(real_edge_count != input.edgeCount())
	{
		cout << "Warning: " << input.edgeCount() - real_edge_count << " self loops were removed" << endl;
	}

	vector<uint32_t> labels(input.vertexCount());
	for(uint32_t i = 0; i < input.vertexCount(); i++) {
		labels[i] = i;
	}

	// Initialize the iteration counter
	int iteration = 0;

	//Start the timer
	auto start = chrono::high_resolution_clock::now();
	par_randomized_cc(input.vertexCount(), edges, labels, &iteration);
	//Stop the timer
	auto end = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
	//Print the labels
	/*
	cout << "Labels at end: ";
	for (uint32_t i = 0; i < input.vertexCount(); i++) {
		cout << "(" << i << ") " << labels[i] << " - ";
	}
	cout << endl;
	*/
	//Count the number of connected components
	unordered_set<uint32_t> cc;
	for (uint32_t i = 0; i < input.vertexCount(); i++) {
		cc.insert(labels[i]);
	}
	cout << "Connected components: " << cc.size() << endl;

	cout << "Execution time: " << duration.count() << " ms" << endl;
	cout << "Iterations: " << iteration << endl;

    return 0;
}