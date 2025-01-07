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
#include "../PPoPP_2018/utils/Edge.hpp"
#include "../PPoPP_2018/utils/GraphInputIterator.hpp"

using namespace std;


#define PRINT(message, rank) \
    if(rank == 0) { \
        cout << "Point n: " << message << endl; \
    }


vector<uint32_t>& par_MPI_deterministic_cc(int rank, int group_size, uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels, int* iteration) 
{
	// Increment the iteration
	if (rank == 0)
		(*iteration)++;
	
	PRINT("Iteration " << *iteration << " Number of edges: " << edges.size(), rank);

	// Base case
	if(edges.size() == 0 || nNodes == 0) 
		return;
 
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

	// Variables to store the number of nodes and edges
	uint32_t nNodes, real_edge_count;
	// Vector to store the edges
	vector<Edge> edges;
	// Vector to store the labels
	vector<uint32_t> labels;
	// Iteration counter
	int iteration;

	//---------------------- Read the graph ----------------------
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
	}

	if( rank == 0 ) {
		// Send the number of nodes to all the processes
		for(int i = 1; i < group_size; i++) {
			MPI_Send(&nNodes, 1, MPI_UINT32_T, i, 0, MPI_COMM_WORLD);
		}
	} else {
		// Receive the number of nodes
		MPI_Recv(&nNodes, 1, MPI_UINT32_T, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	// Start the timer
	double start_time = MPI_Wtime();

	//Compute the connected components
	vector<uint32_t>& map = par_MPI_deterministic_cc(rank, group_size, nNodes, edges, labels, &iteration);

	// Output the number of connected components if the process is the master
	if (rank == 0)
	{
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

	// Close MPI
	MPI_Finalize();
}
