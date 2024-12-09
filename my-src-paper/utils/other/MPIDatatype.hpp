#pragma once

#include <mpi.h>
#include "Edge.hpp"

/**
 * Trait container to allow external implementation for any type
 */
template <typename ElementType>
struct MPIDatatype {};

template <>
struct MPIDatatype<Edge> {
	// TODO global registry -- free datatype
	static MPI_Datatype edge_type;
	static bool initialized;

	static MPI_Datatype constructType() {
		if (!initialized) {
			int blocklengths[2] = { 1, 1 };

			// This leaks abstraction :(
			MPI_Datatype types[2] = { MPI_UNSIGNED, MPI_UNSIGNED };

			MPI_Aint offsets[2];

			offsets[0] = offsetof(Edge, from);
			offsets[1] = offsetof(Edge, to);

			MPI_Type_create_struct(2, blocklengths, offsets, types, &edge_type);
			MPI_Type_commit(&edge_type);

			initialized = true;
		}
		return edge_type;
	};
};
