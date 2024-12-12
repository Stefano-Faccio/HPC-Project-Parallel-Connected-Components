#include <mpi.h>
#include "utils/GraphInputIterator.hpp"
#include "utils/SparseSampling.hpp"
#include <iostream>
#include <vector>
#include <cstdint>

using namespace std;

// Seed for the random number generator
int32_t seed = 17;

void debugPrint(const string &message, int32_t rank)
{
	if(rank == 0)
		cout << "Ciao: " << message << " " << rank << endl;
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
	int32_t rank, group_size;
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	debugPrint("1", rank);

	// Read the number of vertices and edges from the input file
	GraphInputIterator input(argv[1]);
	debugPrint("2", rank);

	// Create a sampler object in which the graph will be loaded
	SparseSampling sampler(MPI_COMM_WORLD, (int32_t)0, group_size, seed + rank, (int32_t)1, input.vertexCount(), input.edgeCount());
	debugPrint("3", rank);
	// Load the appropriate slice of the graph in every process
	sampler.loadSlice(input);
	debugPrint("4", rank);

	// Wait for all processes to finish loading the graph
	MPI_Barrier(MPI_COMM_WORLD);
	debugPrint("5", rank);

	// Compute the connected components
	vector<uint32_t> components;
	int number_of_components = sampler.connectedComponents(components);
	debugPrint("6", rank);

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
	debugPrint("7", rank);

	// Close MPI
	MPI_Finalize();
	debugPrint("8", rank);
}
