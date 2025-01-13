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
#include "utils/Edge.hpp"
#include "utils/GraphInputIterator.hpp"
#include "utils/cse613_utils.hpp"

using namespace std;

vector<uint32_t>& par_randomized_cc(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels, int* iteration);
void coin_toss_and_child_hook(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels);
void configure_RNG();

// Random number generator
// Note: omp threadprivate does not work with mt19937, for some unknown reason
// Note2: rand_r is not an option. It is thread safe and works with omp threadprivate, but is not a good random number generator 
// and give ciclic results that break the algorithm
// Note3: I'm using a thread_local variable: the behavior is the same as threadprivate, but it is a C++11 feature. 
// However, the behavior with openmp is not defined, and it is compiler dependent. With GCC it works as expected.
// Note4: The merseene twister is a very good random number generator, but it is heavy and slow.
// Maybe we should use a faster random number generator, like xorshift
thread_local mt19937 random_genator;

int main(int argc, char* argv[]) {	

	//---------------------- Configure the RNG ----------------------

	// A merseene twister is private to each thread and persistent across calls
	// Every thread will have a different seed
	configure_RNG();

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
			//Normalize the edge so that from < to
			edge.normalize();
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
	vector<uint32_t>& map = par_randomized_cc(input.vertexCount(), edges, labels, &iteration);

	//Stop the timer
	auto end = chrono::high_resolution_clock::now();
	//Calculate the duration
	auto duration_s = chrono::duration_cast<chrono::seconds>(end - start);
	auto duration_ms = chrono::duration_cast<chrono::milliseconds>(end - start);

	//Print the labels at the end
	#if false
	cout << "Labels at end: ";
	for (uint32_t i = 0; i < input.vertexCount(); i++) {
		cout  << map[i] << " ";
	}
	cout << endl;
	#endif

	//Count the number of connected components
	uint32_t number_of_cc = unordered_set<uint32_t>(map.begin(), map.end()).size();

	// Print the results
	// Print the results
	cout << fixed;
	cout << "------------------------------------------------" << endl;
	cout << "File Name: " << argv[1] << endl;
	cout << "Group Size: " << omp_get_num_threads() << endl;
	cout << "Number of vertices: " << input.vertexCount() << endl;
	cout << "Number of edges: " << real_edge_count << endl;
	cout << "Iterations: " << iteration << endl;
	cout << "Number of connected components: " << number_of_cc << endl;
	cout << "Elapsed time: " << duration_s.count() << " s" << endl;
	cout << "Elapsed time: " << duration_ms.count() << " ms" << endl;

    return 0;
}

vector<uint32_t>& par_randomized_cc(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels, int* iteration) 
{
	// Increment the iteration
	(*iteration)++;

	cout << "Iteration " << *iteration << " Number of edges: " << edges.size() << endl;

	// Base case
	if(edges.size() == 0 || nNodes == 0) 
		return labels;
		
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

	return map;
}

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

void configure_RNG()
{
	// Prime numbers for the random number generator seed: usefull to have streems of random numbers that are not correlated
	vector<int> prime_numbers = {3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941,947,953,967,971,977,983,991,997,1009,1013,1019,1021,1031,1033,1039,1049,1051,1061,1063,1069,1087,1091,1093,1097,1103,1109,1117,1123,1129,1151,1153,1163,1171,1181,1187,1193,1201,1213,1217,1223};

    #pragma omp parallel
    {
		if((uint32_t)omp_get_thread_num() > prime_numbers.size())
		{
			cout << "Error: Not enough prime numbers for the random number generator" << endl;
			exit(1);
		}
		// Seed the random number generator for each thread
		random_genator = mt19937(prime_numbers[omp_get_thread_num()]);
    }

	// To test the random number generator
	#if false
	vector<vector<uint32_t>> random_numbers(omp_get_max_threads());
	#pragma omp parallel
	{
		// Generate random numbers
		for (uint32_t i = 0; i < 10; i++) {
			random_numbers[omp_get_thread_num()].push_back(random_genator());
		}
	}
	// Print the random numbers
	for (uint32_t i = 0; i < random_numbers.size(); i++) {
		cout << "Thread " << i << ": ";
		for (uint32_t j = 0; j < random_numbers[i].size(); j++) {
			cout << random_numbers[i][j] << " ";
		}
		cout << endl;
	}

	exit(1);
	#endif
}