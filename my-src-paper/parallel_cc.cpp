#include <mpi.h>
#include <iostream>
#include <vector>
#include "utils/GraphInputIterator.hpp"
#include "utils/SparseSampling.hpp"
#include <cstdint>

using namespace std;

// Seed for the random number generator
int32_t seed = 17;

int foonkfnsd();

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
	int32_t rank, group_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);

	// Read the number of vertices and edges from the input file
	GraphInputIterator input(argv[1]);

	// Create a sampler object in which the graph will be loaded
	SparseSampling sampler(MPI_COMM_WORLD, (int32_t)0, group_size, seed + rank, (int32_t)1, input.vertexCount(), input.edgeCount());
	// Load the appropriate slice of the graph in every process
	sampler.loadSlice(input);

	// Wait for all processes to finish loading the graph
	MPI_Barrier(MPI_COMM_WORLD);

	// Compute the connected components
	vector<uint32_t> components;
	int number_of_components = sampler.connectedComponents(components);

	// Output the number of connected components if the process is the master
	if (rank == 0)
	{
		cout << fixed;
		cout << argv[1] << ","
			 << group_size << ","
			 << input.vertexCount() << ","
			 << input.edgeCount() << ","
			 << "cc" << ","
			 << number_of_components << endl;
	}

	// Close MPI
	MPI_Finalize();
}


int foonkfnsd()
{
	return 0;
}