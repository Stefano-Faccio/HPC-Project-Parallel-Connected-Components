#include <mpi.h>
#include "utils/GraphInputIterator.hpp"
#include "utils/SparseSampling.hpp"
#include <iostream>
#include <vector>
#include <cstdint>

using namespace std;

// Seed for the random number generator
int32_t seed = 17;

int main(int argc, char *argv[])
{
	cout << "Ciao0" << endl;
	if (argc != 2)
	{
		cout << "Usage: connectivity INPUT_FILE" << endl;
		return 1;
	}
	cout << "Ciao1" << endl;
	// Initialize MPI
	MPI_Init(&argc, &(argv));
	cout << "Ciao2" << endl;
	
	// Get the rank and size in the original communicator: rank is the process ID, size is the number of processes
	int32_t rank, group_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	cout << "Ciao3" << endl;

	// Read the number of vertices and edges from the input file
	GraphInputIterator input(argv[1]);
	cout << "Ciao4" << endl;

	// Create a sampler object in which the graph will be loaded
	SparseSampling sampler(MPI_COMM_WORLD, (int32_t)0, group_size, seed + rank, (int32_t)1, input.vertexCount(), input.edgeCount());
	cout << "Ciao5" << endl;
	// Load the appropriate slice of the graph in every process
	sampler.loadSlice(input);
	cout << "Ciao6" << endl;

	// Wait for all processes to finish loading the graph
	MPI_Barrier(MPI_COMM_WORLD);
	cout << "Ciao7" << endl;

	// Compute the connected components
	vector<uint32_t> components;
	int number_of_components = sampler.connectedComponents(components);
	cout << "Ciao8" << endl;

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
	cout << "Ciao9" << endl;

	// Close MPI
	MPI_Finalize();
	cout << "Ciao10" << endl;
}
