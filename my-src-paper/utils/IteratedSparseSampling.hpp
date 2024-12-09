#pragma once

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <mpi.h>
#include "DisjointSets.hpp"
#include "utils.hpp"
#include "GraphInputIterator.hpp"
#include "MPIDatatype.hpp"

using namespace std;

/**
 * Implements ISS primitives
 *
 * This class performs the logic of a node within a group that is specified by the communicator.
 *
 * Edge must have publicly accessible properties
 * `uint32_t from, to`
 * and be have a corresponding `MPIDatatype<Edge>` instantiation
 */

class IteratedSparseSampling {
protected:

	const int root_rank_ = 0;

	MPI_Comm communicator_;
	int rank_, color_, group_size_;
	vector<Edge> edges_slice_;
	const float epsilon_ = 0.1f;
	int32_t seed_with_offset_;
	MPI_Datatype mpi_edge_t_;
	uint32_t target_size_;
	uint32_t vertex_count_;
	uint32_t initial_vertex_count_, initial_edge_count_;

public:

	IteratedSparseSampling(MPI_Comm communicator,
						   int color,
						   int group_size,
						   int32_t seed_with_offset,
						   uint32_t target_size,
						   uint32_t vertex_count,
						   uint32_t edge_count) :
			communicator_(communicator),
			color_(color),
			group_size_(group_size),
			seed_with_offset_(seed_with_offset),
			target_size_(target_size),
			vertex_count_(vertex_count),
			initial_vertex_count_(vertex_count),
			initial_edge_count_(edge_count)
	{
		MPI_Comm_rank(communicator_, &rank_);
		mpi_edge_t_ = MPIDatatype<Edge>::constructType();
	}

	~IteratedSparseSampling() {
	}

	int rank() const {
		return rank_;
	}

	bool master() const {
		return rank_ == root_rank_;
	}

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
		uint32_t slice_size = edges_slice_.size();

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
		uint32_t slice_size;

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

	/**
	 * Maps edge endpoints after contraction.
	 * @param vertex_map The root must contain a valid vertex mapping to apply.
	 *                   vertex_map must be of the right size (number of vertices before applying the mapping).
	 */
	void receiveAndApplyMapping(vector<uint32_t> & vertex_map) {
		MPI_Bcast(vertex_map.data(), vertex_map.size(), MPI_UINT32_T, root_rank_, communicator_);

		applyMapping(vertex_map);
		MPI_Bcast(&vertex_count_, 1, MPI_UINT32_T, root_rank_, communicator_);
	}

	/**
	 * Apply the map to all endpoints, dropping loops
	 * @param vertex_map
	 */
	void applyMapping(const vector<uint32_t> & vertex_map) {
		vector<Edge> updated_edges;

		for (auto edge : edges_slice_) {
			edge.from = vertex_map.at(edge.from);
			edge.to = vertex_map.at(edge.to);
			if (edge.from != edge.to) {
				updated_edges.push_back(edge);
			}
		}

		edges_slice_.swap(updated_edges);
	}

	/**
	 * @param edges array of {edge_count >= 0} edges
	 * @param [out] vertices_map preallocated map of {vertex_count >= 0} vertices. Will be filled with partitions label from [0, vertex_count)
	 * @param components_count the desired number of connected components
	 * @param [out] resulting_vertex_count how many vertices remain
	 * @param true if the described prefix exists
	 * I don't trust this code, it has just been copied over -- My past self has written it
	 */
	bool prefixConnectedComponents(const vector<Edge> & edges,
								   vector<uint32_t> & vertex_map,
								   uint32_t components_count,
								   uint32_t & resulting_vertex_count)
	{
		DisjointSets<uint32_t> dsets(vertex_map.size());

		if (components_count == 0 || vertex_map.size() == 0) {
			return true;
		}

		uint32_t components_active = vertex_map.size();
		bool found = false;

		size_t i = 0;
		for (; i < edges.size() && components_active > components_count; i++) {
			int v1_set = dsets.find(edges.at(i).from),
					v2_set = dsets.find(edges.at(i).to);
			if (v1_set != v2_set) {
				components_active--;
				dsets.unify(v1_set, v2_set);
			}
		}

		if (components_active == components_count) {
			found = true;
		}

		// Also relabel the components to be in [0, new_vertex_count)!
		const long mapping_undefined = -1l;
		vector<long> component_labels(vertex_map.size(), mapping_undefined);
		size_t next_label = 0;
		for (uint32_t j = 0; j < vertex_map.size(); j++) {
			if (component_labels.at(dsets.find(j)) == mapping_undefined) {
				component_labels.at(dsets.find(j)) = next_label++;
			}

			vertex_map.at(j) = component_labels.at(dsets.find(j));
		}

		resulting_vertex_count = components_active;
		return found;
	}

	/**
	 * Sample `edge_count` edges locally, prop. to their weight
	 * @param edge_count
	 * @return The edge sample
	 */
	virtual vector<Edge> sample(uint32_t edge_count) = 0;

	uint32_t initiateSampling(vector<int> edges_per_processor, vector<uint32_t> & vertex_map) {
		uint32_t number_of_edges_to_sample = accumulate(edges_per_processor.begin(), edges_per_processor.end(), 0u);

		/**
		 * Scatter sampling requests
		 */
		int edges_to_sample_locally;
		MPI_Scatter(edges_per_processor.data(), 1, MPI_INT, &edges_to_sample_locally, 1, MPI_INT, 0, communicator_);

		/**
		 * Take part in sampling
		 */
		vector<Edge> samples = sample(edges_to_sample_locally);

		/**
		 * Gather samples
		 */
		// Allocate space
		vector<Edge> global_samples(number_of_edges_to_sample);
		// Calculate displacement vector
		vector<int> relative_displacements(edges_per_processor);
		relative_displacements.insert(relative_displacements.begin(), 0); // Start at offset zero
		relative_displacements.pop_back(); // Last element not needed

		vector<int> displacements;
		partial_sum(relative_displacements.begin(), relative_displacements.end(), back_inserter(displacements));

		assert(master());
		MPI_Gatherv(
				samples.data(),
				edges_to_sample_locally,
				mpi_edge_t_,
				global_samples.data(),
				edges_per_processor.data(),
				displacements.data(),
				mpi_edge_t_,
				root_rank_,
				communicator_
		);

		assert(global_samples.size() == number_of_edges_to_sample);

		/**
		 * Shuffle to ensure random order for the prefix
		 */
		//shuffle(global_samples.begin(), global_samples.end());

		/**
		 * Incremental prefix scan
		 */
		vertex_map.resize(vertex_count_);
		uint32_t resulting_vertex_count;
		prefixConnectedComponents(
				global_samples,
				vertex_map,
				target_size_,
				resulting_vertex_count
		);

		vertex_count_ = resulting_vertex_count;
		// Yay we are done
		return resulting_vertex_count;
	}

	/**
	 * Match `initiateSampling` at non-root nodes
	 */
	void acceptSamplingRequest() {
		int edges_to_sample_locally;
		MPI_Scatter(nullptr, 1, MPI_INT, &edges_to_sample_locally, 1, MPI_INT, 0, communicator_);

		vector<Edge> samples = sample(edges_to_sample_locally);

		MPI_Gatherv(
				samples.data(),
				edges_to_sample_locally,
				mpi_edge_t_,
				nullptr,
				nullptr,
				nullptr,
				mpi_edge_t_,
				root_rank_,
				communicator_
		);
	}

	uint32_t vertexCount() const {
		return vertex_count_;
	}

	uint32_t initialEdgeCount() const {
		return initial_edge_count_;
	}
};

