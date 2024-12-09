#include <mpi.h>
#include <iostream>
#include <vector>
#include "utils/GraphInputIterator.hpp"
//#include "utils/UnweightedIteratedSparseSampling.hpp"

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

	/*
	UnweightedIteratedSparseSampling sampler(MPI_COMM_WORLD, 0, p, rank, 1, input.vertexCount(), input.edgeCount());
	sampler.loadSlice(input);
	UnweightedIteratedSparseSampling iteration_sampler(sampler);
	*/

	vector<unsigned> components;

	MPI_Barrier(MPI_COMM_WORLD);

	int number_of_components = 0;

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
