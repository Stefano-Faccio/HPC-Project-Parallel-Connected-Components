#include <mpi.h>
#include <iostream>
#include <vector>
#include "utils/GraphInputIterator.hpp"
#include "utils/SparseSampling.hpp"
#include <cstdint>

using namespace std;

int main(int argc, char* argv[])
{
	if (argc != 2 ) {
		cout << "Usage: connectivity INPUT_FILE" << endl;
		return 1;
	}

	MPI_Init(&argc, &(argv));

	GraphInputIterator input(argv[1]);
	int32_t rank, p;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	
	SparseSampling sampler(MPI_COMM_WORLD, (int32_t)0, p, (int32_t)1, input.vertexCount(), input.edgeCount());
	/*
	sampler.loadSlice(input);
	SparseSampling iteration_sampler(sampler);
	*/

	MPI_Barrier(MPI_COMM_WORLD);

	vector<uint32_t> components;
	int number_of_components = 0;//iteration_sampler.connectedComponents(components);

	if (rank == 0) {
		cout << fixed;
		cout << argv[1] << ","
					<< p << ","
					<< input.vertexCount() << ","
					<< input.edgeCount() << ","
					<< "cc" << ","
					<< number_of_components << endl;
	}

	MPI_Finalize();
}
