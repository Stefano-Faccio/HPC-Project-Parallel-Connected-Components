//OpenMP header
#include <omp.h>
//Standard libraries
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <cassert>
#include <algorithm>
#include <unordered_set>
//Custom libraries
#include "../PPoPP_2018/utils/Edge.hpp"
#include "../PPoPP_2018/utils/GraphInputIterator.hpp"

using namespace std;

void find_roots(uint32_t nNodes, vector<uint32_t>& labels);
vector<Edge> find_rank_and_remove_edges(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels);
vector<uint32_t>& par_deterministic_cc(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels, int* iteration);

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

	//vector<Edge> nextEdges = ;

	// Recursively call the function
	return par_deterministic_cc(nNodes, find_rank_and_remove_edges(nNodes, edges, labels), labels, iteration);
}

void find_roots(uint32_t nNodes, vector<uint32_t>& labels)
{
	// Let's break down the possibility of concurrency problems:
	// 1. If a node is a root, it will basically not be changed
	// 2. If a node is a leaf, it will be just written once and never read
	// 3. If a node is in the middle of a chain, it will be read and written at least once -> problem

	// I have done some tests with atomic read and atomic write, but it is slower 
	// than just reading and writing without atomic operations and give the same results
	// because the algorithm is sequantial consistent, so it is fine

	bool found = true;

	while(found)
	{
		found = false;

		#if false
		#pragma omp parallel for shared(nNodes, labels) reduction(||:found)
		for(uint32_t i = 0; i < nNodes; i++)
		{
			// I cannot safely read labels[labels[i]] because another thread can modify it
			// With atomic read, I am sure that the value is "pulled" from the memory
			uint32_t grandpa, new_gramdpa;
			#pragma omp atomic read
			grandpa = labels[labels[i]];
			#pragma omp atomic read
			new_gramdpa = labels[grandpa];
			
			// I cannot safely write labels[i] because another thread can read it 
			// With atomic write, I am sure that the value is "pushed" to the memory
			#pragma omp atomic write
			labels[i] = grandpa;
			// Maybe an flush is enough here instead of atomic write

			if(grandpa != new_gramdpa)
				found = true;
		}
		#else
		#pragma omp parallel for shared(nNodes, labels, found)
		for(uint32_t i = 0; i < nNodes; i++)
		{
			labels[i] = labels[labels[i]];

			if(labels[i] != labels[labels[i]])
				found = true;
		}
		#endif
	}

	return;
}

vector<Edge> find_rank_and_remove_edges(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels)
{
	// This vectors will be deallocated when the function ends
	vector<uint32_t> edges_mark(edges.size(), 0), prefix_sum(edges.size(), 0);
	// Vector to store the next edges for the recursive call
	vector<Edge> nextEdges;
	// Temporary variable for the prefix sum
	uint32_t prefix_sum_temp = 0;

	#pragma omp parallel shared(nNodes, edges, labels, edges_mark, prefix_sum, nextEdges) 
	{
		// Prepare to remove edges inside the same group
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;

			// If the nodes are in different groups, mark the edge
			if(labels[from] != labels[to])
				//Not a race condition because each thread writes to a different index
				edges_mark[i] = 1;
		}

		// if __GNUC__ >= 10, use the new omp scan directive
		// Otherwise, use the old way to do a prefix sum
		#if __GNUC__ >= 10
		// Prefix sum with parallel for
		#pragma omp for reduction(inscan, + : prefix_sum_temp)
        for (uint32_t i = 0; i < edges.size(); i++) {
            
            prefix_sum_temp += edges_mark[i]; 
            #pragma omp scan inclusive(prefix_sum_temp)
            prefix_sum[i] = prefix_sum_temp;
        }
		#else
		// Prefix sum sequential
		#pragma omp single
		{
			// Prefix sum
			prefix_sum[0] = edges_mark[0];
			for(uint32_t i = 1; i < edges.size(); i++)
			{
				prefix_sum[i] = edges_mark[i] + prefix_sum[i - 1];
			}
		}
		#endif

		// Allocate memory for the nextEdges vector 
		// Deallocate memory from the previous used vector
		#pragma omp single
		{
			vector<uint32_t>().swap(edges_mark); // Free edges_mark memory
			nextEdges.resize(prefix_sum[edges.size() - 1]);
		}
		
		// Copy only edges that are between different groups
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;

			// If the nodes are in different groups, add the edge to the nextEdges vector
			if(labels[from] != labels[to])
				//Not a race condition because:
				// if the condition is true, edges_mark[i] will be 1, so prefix_sum[i] will be different from prefix_sum[i-1]
				nextEdges[prefix_sum[i] - 1] = (labels[from] < labels[to] ? Edge{labels[from], labels[to]} : Edge{labels[to], labels[from]});			
		}
	}

	return nextEdges;
}

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

/*

	// Coin toss and child hook
	coin_toss_and_child_hook(nNodes, edges, labels);

	// Find the rank 
	vector<Edge> nextEdges = find_rank_and_remove_edges(nNodes, edges, labels);

	if(nextEdges.size() == edges.size())
	{
		//Print edges
		cerr << "Error iteration " << *iteration << " (same edges as last iterarion): ";
		for(uint32_t i = 0; i < edges.size(); i++)
			cerr << "[" << edges[i].from << "," << edges[i].to << "] ";
		cerr << endl;
	}

	// Recursively call the function
	vector<uint32_t>& map = par_randomized_cc(nNodes, nextEdges, labels, iteration);

	// Free nextEdges memory
	vector<Edge>().swap(nextEdges); 

	//Map results back to the original graph
	map_results_back(nNodes, edges, labels, map);

void coin_toss_and_child_hook(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels) 
{
	// Hook child to a parent based on the coin toss		
	vector<bool> coin_toss(nNodes);

	#pragma omp parallel shared(nNodes, edges, labels, coin_toss) 
	{	
		// Generate random coin tosses
		#pragma omp for
		for (uint32_t i = 0; i < nNodes; i++) {
			// Not a race condition because each thread writes to a different index and has a different RNG
			coin_toss[i] = random_genator() % 2; // Tail is True and Head is False
		}

		// Hook child to a parent based on the coin toss
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;

			// Race condition ONLY labels that has coin_toss TRUE (labels that has coin_toss FALSE are read only)
			// So atomic writes is sufficient 
			if(coin_toss[from] && !coin_toss[to])
			{
				#pragma omp atomic write
				labels[from] = labels[to];
			}
			else if(!coin_toss[from] && coin_toss[to])
			{
				#pragma omp atomic write
				labels[to] = labels[from];
			}
		}
	}

	return;
}

vector<Edge> find_rank_and_remove_edges(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels)
{
	// This vectors will be deallocated when the function ends
	vector<uint32_t> edges_mark(edges.size(), 0), prefix_sum(edges.size(), 0);
	// Vector to store the next edges for the recursive call
	vector<Edge> nextEdges;
	// Temporary variable for the prefix sum
	uint32_t prefix_sum_temp = 0;

	#pragma omp parallel shared(nNodes, edges, labels, edges_mark, prefix_sum, nextEdges) 
	{
		// Prepare to remove edges inside the same group
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;

			// If the nodes are in different groups, mark the edge
			if(labels[from] != labels[to])
				//Not a race condition because each thread writes to a different index
				edges_mark[i] = 1;
		}

		// if __GNUC__ >= 10, use the new omp scan directive
		// Otherwise, use the old way to do a prefix sum
		#if __GNUC__ >= 10
		// Prefix sum with parallel for
		#pragma omp for reduction(inscan, + : prefix_sum_temp)
        for (uint32_t i = 0; i < edges.size(); i++) {
            
            prefix_sum_temp += edges_mark[i]; 
            #pragma omp scan inclusive(prefix_sum_temp)
            prefix_sum[i] = prefix_sum_temp;
        }
		#else
		// Prefix sum sequential
		#pragma omp single
		{
			// Prefix sum
			prefix_sum[0] = edges_mark[0];
			for(uint32_t i = 1; i < edges.size(); i++)
			{
				prefix_sum[i] = edges_mark[i] + prefix_sum[i - 1];
			}
		}
		#endif

		// Allocate memory for the nextEdges vector 
		// Deallocate memory from the previous used vector
		#pragma omp single
		{
			vector<uint32_t>().swap(edges_mark); // Free edges_mark memory
			nextEdges.resize(prefix_sum[edges.size() - 1]);
		}
		
		// Copy only edges that are between different groups
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;

			// If the nodes are in different groups, add the edge to the nextEdges vector
			if(labels[from] != labels[to])
				//Not a race condition because:
				// if the condition is true, edges_mark[i] will be 1, so prefix_sum[i] will be different from prefix_sum[i-1]
				nextEdges[prefix_sum[i] - 1] = (labels[from] < labels[to] ? Edge{labels[from], labels[to]} : Edge{labels[to], labels[from]});			
		}
	}

	return nextEdges;
}

void map_results_back(uint32_t nNodes, const vector<Edge>& edges, const vector<uint32_t>& labels, vector<uint32_t>& map)
{
	#pragma omp parallel for shared(edges, labels, map)
	for(uint32_t i = 0; i < edges.size(); i++)
	{
		uint32_t from = edges[i].from;
		uint32_t to = edges[i].to;

		// The race condition is only in the labels that has been previously changed
		// So, again, atomic writes is sufficient
		
		if(to == labels[from])
		{
			#pragma omp atomic write
			map[from] = map[to];
		}
		else if(from == labels[to])
		{
			#pragma omp atomic write
			map[to] = map[from];
		}
	}

}
*/