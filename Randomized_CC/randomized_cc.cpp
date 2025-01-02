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
#include <atomic>
//Custom libraries
#include "../PPoPP_2018/utils/Edge.hpp"
#include "../PPoPP_2018/utils/GraphInputIterator.hpp"

using namespace std;

void coin_toss_and_child_hook(uint32_t nNodes, const vector<Edge>& edges, vector<atomic<uint32_t>>& labels);
vector<Edge> find_rank_and_remove_edges(uint32_t nNodes, const vector<Edge>& edges, vector<atomic<uint32_t>>& labels);
void map_results_back(uint32_t nNodes, const vector<Edge>& edges, vector<atomic<uint32_t>>& labels);

// Random number generator
static uint32_t seed;
#pragma omp threadprivate(seed)

void par_randomized_cc(uint32_t nNodes, const vector<Edge>& edges, vector<atomic<uint32_t>>& labels, int* iteration) {

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

	/*
	if (nextEdges.size() == edges.size())
		cout << "PROBLEM" << endl;
	{
		//Print the labels
		cout << "Labels at end: ";
		for (uint32_t i = 0; i < nNodes; i++) {
			cout << "(" << i << ") " << labels[i] << " - ";
		}
		cout << endl;
		//Print the edges
		cout << "Edges: ";
		for (uint32_t i = 0; i < edges.size(); i++) {
			cout << "(" << edges[i].from << ", " << edges[i].to << ") ";
		}
		cout << endl;
		//Print the nextEdges
		cout << "Next Edges: ";
		for (uint32_t i = 0; i < nextEdges.size(); i++) {
			cout << "(" << nextEdges[i].from << ", " << nextEdges[i].to << ") ";
		}
		cout << endl;
	}*/

	// Recursively call the function
	par_randomized_cc(nNodes, nextEdges, labels, iteration);

	// Free nextEdges memory
	vector<Edge>().swap(nextEdges); 

	//Map results back to the original graph
	map_results_back(nNodes, edges, labels);

	return;
}

// Coin toss and child hook - OK
void coin_toss_and_child_hook(uint32_t nNodes, const vector<Edge>& edges, vector<atomic<uint32_t>>& labels) 
{
	// Hook child to a parent based on the coin toss		
	vector<bool> coin_toss(nNodes);

	#pragma omp parallel shared(nNodes, edges, labels, coin_toss) 
	{	
		// Generate random coin tosses
		#pragma omp single
		for (uint32_t i = 0; i < nNodes; i++) {
			// Not a race condition because each thread writes to a different index
			coin_toss[i] = rand_r(&seed) % 2; // Tail is True and Head is False
		}

		// Hook child to a parent based on the coin toss
		#pragma omp single
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;
			// Race condition on labels[edges[i].from] and labels[edges[i].to]
			// I'm using atomic variables to avoid it
			// Other option is to use a critical section or omp atomic if possible
			if(coin_toss[from] && !coin_toss[to])
				labels[from].store(labels[to].load());
			else if(!coin_toss[from] && coin_toss[to])
				labels[to].store(labels[from]);
		}
	}
}


vector<Edge> find_rank_and_remove_edges(uint32_t nNodes, const vector<Edge>& edges, vector<atomic<uint32_t>>& labels)
{
	// This vectors will be deallocated when the function ends
	vector<uint32_t> s(edges.size(), 0), S(edges.size());
	// Vector to store the next edges for the recursive call
	vector<Edge> nextEdges;

	#pragma omp parallel shared(nNodes, edges, labels, s, S, nextEdges) 
	{
		// Prepare to remove edges inside the same group
		#pragma omp single
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;
			// If the nodes are in different groups, mark the edge
			if(labels[from] != labels[to])
				//Not a race condition because each thread writes to a different index
				s[i] = 1;
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

		// Allocate memory for the nextEdges vector 
		// Deallocate memory from the previous used vector
		#pragma omp single
		{
			vector<uint32_t>().swap(s); // Free s memory
			nextEdges.resize(S[edges.size() - 1]);
		}
		
		// Copy only edges that are between different groups
		#pragma omp single
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;

			// If the nodes are in different groups, add the edge to the nextEdges vector
			if(labels[from] != labels[to])
				//Not a race condition because:
				// if the condition is true, s[i] will be 1, so S[i] will be different from S[i-1]
				nextEdges[S[i] - 1] = {labels[from], labels[to]};				
		}
	}

	return nextEdges;
}

void map_results_back(uint32_t nNodes, const vector<Edge>& edges, vector<atomic<uint32_t>>& labels, vector<atomic<uint32_t>>& map)
{
	//#pragma omp parallel for shared(edges, labels)
	for(uint32_t i = 0; i < edges.size(); i++)
	{
		uint32_t from = edges[i].from;
		uint32_t to = edges[i].to;
		// Race condition on labels avoided by using atomic variables
		if(to == labels[from])
			map[from].store(map[to]);
	}

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

	vector<atomic<uint32_t>> labels(input.vertexCount());
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
	cout << "Labels at end: ";
	for (uint32_t i = 0; i < input.vertexCount(); i++) {
		cout  << labels[i] << " ";
	}
	cout << endl;
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