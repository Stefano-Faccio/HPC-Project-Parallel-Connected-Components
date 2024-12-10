#pragma once

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <mpi.h>
#include "DisjointSets.hpp"
#include "GraphInputIterator.hpp"

using namespace std;

/**
 * Implements ISS primitives
 * This class performs the logic of a node within a group that is specified by the communicator.
 */

class IteratedSparseSampling {
protected:

public:



	/**
	 * \return Is this the last node in its group?
	 */
	bool last() const {
		return rank_ == group_size_ - 1;
	}

	/**
	 * Read a ~ 1/(group size) slice of edges
	 */
	virtual void loadSlice() = 0;

	/**
	 * Send our slice to equivalent ranks in other groups
	 */
	void broadcastSlice(MPI_Comm equivalence_comm) {
		u_int32_t slice_size = edges_slice_.size();

		MPI_Bcast(
				&slice_size,
				1,
				MPI_UINT32_T,
				0,
				equivalence_comm
		);

		MPI_Bcast(
				edges_slice_.data(),
				edges_slice_.size(),
				mpi_edge_t_,
				0,
				equivalence_comm
		);
	}

	/**
	 * Receive our slice
	 */
	void receiveSlice(MPI_Comm equivalence_comm) {
		u_int32_t slice_size;

		MPI_Bcast(
				&slice_size,
				1,
				MPI_UINT32_T,
				0,
				equivalence_comm
		);

		edges_slice_.resize(slice_size);

		MPI_Bcast(
				edges_slice_.data(),
				edges_slice_.size(),
				mpi_edge_t_,
				0,
				equivalence_comm
		);
	}

	void setSlice(vector<Edge> edges) {
		edges_slice_ = edges;
	}













	u_int32_t vertexCount() const {
		return vertex_count_;
	}

	u_int32_t initialEdgeCount() const {
		return initial_edge_count_;
	}
};

