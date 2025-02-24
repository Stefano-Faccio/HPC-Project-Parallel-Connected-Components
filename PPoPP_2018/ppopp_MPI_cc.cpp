#include <mpi.h>
#include "utils/GraphInputIterator.hpp"
#include "utils/SparseSampling.hpp"
#include <iostream>
#include <vector>
#include <cstdint>

using namespace std;

#ifdef DEBUG
#define DEBUG_PRINT(message, rank) \
    if(rank == 0) { \
        cout << "Point n: " << message << endl; \
    }
#else
#define DEBUG_PRINT(message, rank)
#endif

// Seed for the random number generator
int32_t seed = 17;

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

	// Read the number of vertices and edges from the input file
	GraphInputIterator input(argv[1]);

	// Create a sampler object in which the graph will be loaded
	SparseSampling sampler(MPI_COMM_WORLD, group_size, rank, seed + rank, (int32_t)1, input.vertexCount(), input.edgeCount());

	// Load the appropriate slice of the graph in every process
	sampler.loadSlice(input);

	// Wait for all processes to finish loading the graph
	//Blocks the caller until all processes in the communicator have called it
	MPI_Barrier(MPI_COMM_WORLD);

	// Start the timer
	double start_time = MPI_Wtime();

	// Compute the connected components
	vector<uint32_t> components;
	int number_of_components = sampler.connectedComponents(components);

	// Output the number of connected components if the process is the master
	if (rank == 0)
	{
		double end_time = MPI_Wtime();
		double elapsed_time = end_time - start_time;

		cout << fixed;
		cout << "File Name: " << argv[1] << endl;
		cout << "Group Size: " << group_size << endl;
		cout << "Number of vertices: " << input.vertexCount() << endl;
		cout << "Number of edges: " << input.edgeCount() << endl;
		cout << "Number of connected components: " << number_of_components << endl;
		cout << "Elapsed time: " << elapsed_time << " seconds" << endl;
	}

	// Close MPI
	MPI_Finalize();
}
