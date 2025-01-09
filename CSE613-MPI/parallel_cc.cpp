#include <mpi.h>
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
#include "utils/Edge.hpp"
#include "utils/MPIEdge.hpp"
#include "utils/GraphInputIterator.hpp"
#include "utils/mpi_parallel_cc_utils.hpp"

using namespace std;

#define PRINT(message, rank) \
    if(rank == 0) { \
        cout << "Point n: " << message << endl; \
    }

#define DEBUG 1



void par_MPI_slave_deterministic_cc(int rank, int group_size, uint32_t nNodes)
{
	// Receive the number of edges
	uint32_t nEdges_local;
	MPI_Scatter(nullptr, 1, MPI_UINT32_T, &nEdges_local, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	// Allocate memory for the slice of edges
	vector<Edge> edges_slice(nEdges_local);
	// Allocate memory for the labels
	vector<uint32_t> labels(nNodes);

	// Receive the slice of edges
	MPI_Scatterv(nullptr, nullptr, nullptr, MPIEdge::edge_type, edges_slice.data(), nEdges_local, MPIEdge::edge_type, 0, MPI_COMM_WORLD);
	// Receive the labels
	MPI_Bcast(labels.data(), nNodes, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	#if DEBUG
	string str = "";
	str += "SLAVE Rank: " + to_string(1) + " Edges: ";
	// Print the received edges
	for (uint32_t i = 0; i < nEdges_local; i++)
		str += "(" + to_string(edges_slice[i].from) + "," + to_string(edges_slice[i].to) + ") ";
	str += "\n";
	cout << str;
	#endif

	// Count the number of hooks
	pair<uint32_t, uint32_t> hooks = count_hooks(edges_slice);
	uint32_t hooks_small_2_large = hooks.first;
	uint32_t hooks_large_2_small = hooks.second;

	// AllReduce the number of hooks (Use of MPI_IN_PLACE option)
	MPI_Allreduce(MPI_IN_PLACE, &hooks_small_2_large, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce(MPI_IN_PLACE, &hooks_large_2_small, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);

	// Choose the hook direction
	choose_hook_direction(edges_slice, hooks_small_2_large, hooks_large_2_small, labels);

	// Merge the labels
	if (hooks_small_2_large >= hooks_large_2_small)
		MPI_Reduce(labels.data(), nullptr, nNodes, MPI_UINT32_T, MPI_MAX, 0, MPI_COMM_WORLD);
	else
		MPI_Reduce(labels.data(), nullptr, nNodes, MPI_UINT32_T, MPI_MIN, 0, MPI_COMM_WORLD);

	// Wait the master to find the roots

	// Broadcast the labels
	MPI_Bcast(labels.data(), nNodes, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	// Mark new edges
	vector<uint32_t> marked_edges_edges = mark_new_edges(edges_slice, labels);

	// Gather the marked edges
	MPI_Gatherv(marked_edges_edges.data(), nEdges_local, MPI_UINT32_T, nullptr, nullptr, nullptr, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	// Free the memory of the marked edges
	vector<uint32_t>().swap(marked_edges_edges);

	// Wait the master to compute the prefix sum


}

vector<uint32_t>& par_MPI_master_deterministic_cc(int rank, int group_size, uint32_t nNodes, uint32_t nEdges, const vector<Edge>& edges, vector<uint32_t>& labels, int* iteration) 
{
	// Increment the iteration
	(*iteration)++;
		
	PRINT("Iteration " << *iteration << " Number of edges: " << edges.size(), rank);

	// Calculate the number of edges to send to each processor 
	vector<int> edges_per_proc = calculate_edges_per_processor(rank, group_size, edges);
	// Calculate the displacements for the scatterv function
	vector<int> displacements = calculate_displacements(rank, group_size, edges_per_proc);

	// Send and receive the edges number
	uint32_t nEdges_local;
	MPI_Scatter(edges_per_proc.data(), 1, MPI_UINT32_T, &nEdges_local, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);
	
	// Base case
	if(nEdges_local == 0 || nNodes == 0) 
		return labels;

	// Allocate memory for slice of edges
	vector<Edge> edges_slice(nEdges_local);

	// Send a slice of the edges to each process
	MPI_Scatterv(edges.data(), edges_per_proc.data(), displacements.data(), MPIEdge::edge_type, edges_slice.data(), nEdges_local, MPIEdge::edge_type, 0, MPI_COMM_WORLD);
	// Broadcast the labels
	MPI_Bcast(labels.data(), nNodes, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	#if DEBUG
	string str = "";
	str += "MASTER Rank: " + to_string(rank) + " Edges: ";
	// Print the received edges
	for (uint32_t i = 0; i < nEdges_local; i++)
		str += "(" + to_string(edges_slice[i].from) + "," + to_string(edges_slice[i].to) + ") ";
	str += "\n";
	cout << str;
	#endif

	// Count the number of hooks
	pair<uint32_t, uint32_t> hooks = count_hooks(edges_slice);
	uint32_t hooks_small_2_large = hooks.first;
	uint32_t hooks_large_2_small = hooks.second;

	// AllReduce the number of hooks (Use of MPI_IN_PLACE option)
	MPI_Allreduce(MPI_IN_PLACE, &hooks_small_2_large, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce(MPI_IN_PLACE, &hooks_large_2_small, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);

	// Choose the hook direction
	choose_hook_direction(edges_slice, hooks_small_2_large, hooks_large_2_small, labels);

	// Merge the labels
	if (hooks_small_2_large >= hooks_large_2_small)
		MPI_Reduce(MPI_IN_PLACE, labels.data(), nNodes, MPI_UINT32_T, MPI_MAX, 0, MPI_COMM_WORLD);
	else
		MPI_Reduce(MPI_IN_PLACE, labels.data(), nNodes, MPI_UINT32_T, MPI_MIN, 0, MPI_COMM_WORLD);

	// Find the roots for every node
	find_roots(nNodes, labels);

	// Broadcast the labels
	MPI_Bcast(labels.data(), nNodes, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	// Mark new edges
	vector<uint32_t> marked_edges_local = mark_new_edges(edges_slice, labels);

	// Allocate memory for the marked edges
	vector<uint32_t> marked_edges(nEdges);

	// Gather the marked edges
	MPI_Gatherv(marked_edges_local.data(), nEdges_local, MPI_UINT32_T, marked_edges.data(), edges_per_proc.data(), displacements.data(), MPI_UINT32_T, 0, MPI_COMM_WORLD);

	// Compute the prefix sum of the marked edges
	vector<uint32_t> prefix_sum = compute_prefix_sum(marked_edges);

	// Free the memory of the marked edges local
	vector<uint32_t>().swap(marked_edges_local);
	// Free the memory of the marked edges
	vector<uint32_t>().swap(marked_edges);
	// Allocate memory for the next edges
	vector<Edge> nextEdges(prefix_sum.back());

	


	return labels;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "Usage: connectivity INPUT_FILE" << endl;
		return 1;
	}

	// Initialize MPI
	MPI_Init(&argc, &(argv));
	
	// Get the rank and size in the original communicator: rank is the process ID, size is the number of processes
	int32_t group_size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//Initialize the MPIEdge type
	MPIEdge::constructType();

	// Variables to store the number of nodes and edges
	uint32_t nNodes, real_edge_count;
	// Vector to store the edges
	vector<Edge> edges;
	// Vector to store the labels
	vector<uint32_t> labels;
	// Iteration counter
	int iteration;

	//---------------------- Read the graph and initialize data ----------------------
	if(rank == 0) {	
		// Read the number of vertices and edges from the input file
		GraphInputIterator input(argv[1]);

		real_edge_count = 0;
		for (auto edge : input)
		{
			// Check that the edge is valid i.e. the nodes are in the graph
			assert(edge.from < input.vertexCount());
			assert(edge.to < input.vertexCount());
			// Check that the edge is not a self loop
			if (edge.to != edge.from)
			{
				edges.push_back(edge);
				real_edge_count++;
			}
		}

		// Save the number of nodes
		nNodes = input.vertexCount();

		//Check if self loops were removed
		if(real_edge_count != input.edgeCount())
			cout << "Warning: " << input.edgeCount() - real_edge_count << " self loops were removed" << endl;

		// Initialize the labels
		labels.resize(nNodes);
		for(uint32_t i = 0; i < input.vertexCount(); i++) {
			labels[i] = i;
		}

		// Initialize the iteration counter
		iteration = 0;

		// Start the timer
		double start_time = MPI_Wtime();

		//---------------------- Broadcast the number of nodes ----------------------
		MPI_Bcast(&nNodes, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);		

		//Compute the connected components
		vector<uint32_t> map = par_MPI_master_deterministic_cc(rank, group_size, nNodes, real_edge_count, edges, labels, &iteration);

		//---------------------- End the timer and print the results ----------------------
		double end_time = MPI_Wtime();
		double elapsed_time = end_time - start_time;

		//Count the number of connected components
		uint32_t number_of_cc = unordered_set<uint32_t>(map.begin(), map.end()).size();

		cout << fixed;
		cout << "File Name: " << argv[1] << endl;
		cout << "Group Size: " << group_size << endl;
		cout << "Number of vertices: " << nNodes << endl;
		cout << "Number of edges: " << real_edge_count << endl;
		cout << "Number of connected components: " << number_of_cc << endl;
		cout << "Elapsed time: " << elapsed_time << " seconds" << endl;

		#if false
		//Print the labels at the end
		cout << "Labels at end: ";
		for (uint32_t i = 0; i < input.vertexCount(); i++) {
			cout  << map[i] << " ";
		}
		cout << endl;
		#endif
	}
	else {
		// Receive the number of nodes
		MPI_Bcast(&nNodes, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);

		//Compute the connected components
		par_MPI_slave_deterministic_cc(rank, group_size, nNodes);
	}

	//Wait for all processes to finish
	MPI_Barrier(MPI_COMM_WORLD);

	// Close MPI
	MPI_Finalize();
}
