#pragma once

#include "Edge.hpp"
#include "GraphInputIterator.hpp"
#include "DisjointSets.hpp"
#include <mpi.h>
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>

using namespace std;

// Inherits from Edge
class SparseSampling : public Edge
{
private:
	MPI_Comm communicator_;
	const float epsilon_ = 0.09f;
	const float delta_ = 0.2f;
	int32_t rank_, color_, group_size_;
	vector<Edge> edges_slice_;
	MPI_Datatype mpi_edge_t_;
	uint32_t target_size_;
	uint32_t vertex_count_;
	uint32_t initial_vertex_count_, initial_edge_count_;
	mt19937 random_engine_;

	uint32_t countEdges()
	{
		uint32_t local_edges = edges_slice_.size(), edges;
		MPI_Allreduce(&local_edges, &edges, 1, MPI_UINT32_T, MPI_SUM, communicator_);
		return edges;
	}

	// A vector whose entries correspond to the number of edges to sample at that processor
	vector<int32_t> edgesToSamplePerProcessor(vector<uint32_t> edges_available_per_processor)
	{
		uint32_t total_edges = (uint32_t)accumulate(edges_available_per_processor.begin(), edges_available_per_processor.end(), 0);
		uint32_t number_of_edges_to_sample = min(uint32_t(pow((float)initial_vertex_count_, 1 + epsilon_ / 2) * (1 + delta_)), total_edges);
		uint32_t sparsity_threshold = uint32_t(float(3) / (delta_ * delta_) * log(group_size_ / 0.9f));
		uint32_t remaining_edges = number_of_edges_to_sample;

		vector<int32_t> edges_per_processor(group_size_, 0);

		// First look at processors with few edges
		for (uint32_t i = 0; i < group_size_; i++)
		{
			if (edges_available_per_processor.at(i) <= sparsity_threshold)
			{
				edges_per_processor.at(i) = edges_available_per_processor.at(i);
				remaining_edges -= edges_available_per_processor.at(i);
				// We don't want to consider these for equal redistribution
				number_of_edges_to_sample -= edges_available_per_processor.at(i);
			}
		}

		// Assign proportional share of edges to every processor. Depending on the balance, this is necessary to
		// counter accumulating +-1 differences. This also ensures that empty slices will not be asked to sample anything.
		for (uint32_t i = 0; i < group_size_; i++)
		{
			if (edges_per_processor.at(i) != 0)
			{
				continue; // Skip maxed-out processors
			}

			edges_per_processor.at(i) = min(
				(uint32_t)(double(number_of_edges_to_sample) * edges_available_per_processor.at(i) / total_edges),
				edges_available_per_processor.at(i));
			remaining_edges -= edges_per_processor.at(i);
		}

		// We cannot give the remaining requests to just any processor, as we did previously. Distribute them among
		// the ones with the capacity
		uint32_t spillover = 0;
		while (remaining_edges > 0)
		{
			if (edges_available_per_processor.at(spillover) > edges_per_processor.at(spillover))
			{
				uint32_t delta = min(edges_available_per_processor.at(spillover) - edges_per_processor.at(spillover), (uint32_t)remaining_edges);
				edges_per_processor.at(spillover) += delta;
				remaining_edges -= delta;
			}
			spillover++;
		}

		return edges_per_processor;
	}

	// Edges per rank, at root only
	vector<uint32_t> edgesAvailablePerProcessor()
	{
		uint32_t available = (uint32_t)edges_slice_.size(); // MPI displacement types are rather unfortunate
		vector<uint32_t> edges_per_processor;

		if (master())
		{
			edges_per_processor.resize(group_size_);
		}

		MPI_Gather(&available, 1, MPI_UINT32_T, edges_per_processor.data(), 1, MPI_UINT32_T, 0, communicator_);

		return edges_per_processor; // NRVO
	}

public:
	SparseSampling(MPI_Comm communicator, uint32_t color, int32_t group_size, int32_t seed, int32_t target_size, uint32_t vertex_count, uint32_t edge_count) : communicator_(communicator), color_(color), group_size_(group_size), random_engine_(seed), target_size_(target_size), vertex_count_(vertex_count), initial_vertex_count_(vertex_count), initial_edge_count_(edge_count)
	{
		MPI_Comm_rank(communicator_, &rank_);
		mpi_edge_t_ = MPIEdge::constructType();
	}

	~SparseSampling() {}

	int32_t rank() const
	{
		return rank_;
	}

	bool master() const
	{
		return rank_ == 0;
	}

	// The root will receive the labels of the connected components in the vector
	uint32_t connectedComponents(vector<uint32_t> &connected_components)
	{
		if (vertex_count_ != initial_vertex_count_)
		{
			throw logic_error("Cannot perform CC on a shrinked graph");
		}

		if (master())
		{
			connected_components.resize(vertex_count_);

			for (uint32_t i = 0; i < vertex_count_; i++)
			{
				connected_components.at(i) = i;
			}
		}

		// We could use just one edge info exchange per round
		while (countEdges() > 0)
		{
			vector<uint32_t> vertex_map(vertex_count_);

			if (master())
			{
				initiateSampling(edgesToSamplePerProcessor(edgesAvailablePerProcessor()), vertex_map);

				for (uint32_t i = 0; i < connected_components.size(); i++)
				{
					connected_components.at(i) = vertex_map.at(connected_components.at(i));
				}
			}
			else
			{
				edgesAvailablePerProcessor();
				acceptSamplingRequest();
			}

			receiveAndApplyMapping(vertex_map);
		}

		return vertex_count_;
	}

	void loadSlice(GraphInputIterator &input)
	{
		// Exposing iterables is sort of hard, this will do for now
		// TODO potentially make GII expose an iterable interface that also takes slicing into account
		uint32_t slice_portion = input.edgeCount() / group_size_;
		uint32_t slice_from = slice_portion * rank_;
		// The last node takes any leftover edges
		bool last = rank_ == group_size_ - 1;
		uint32_t slice_to = last ? input.edgeCount() : slice_portion * (rank_ + 1);

		GraphInputIterator::Iterator iterator = input.begin();
		while (!iterator.end_)
		{
			if (iterator.position() >= slice_from && iterator.position() < slice_to)
			{
				edges_slice_.push_back({iterator->from, iterator->to});
			}
			++iterator;
		}
	}

	vector<Edge> sample(uint32_t edge_count)
	{
		vector<Edge> edges;
		uint32_t size = edges_slice_.size();

		if (edge_count == edges_slice_.size())
		{
			return edges_slice_;
		}
		else
		{
			for (uint32_t i = 0; i < edge_count; i++)
			{
				edges.push_back(edges_slice_.at(random_engine_() % size));
			}
		}

		return edges; // NRVO
	}

	uint32_t initiateSampling(vector<int32_t> edges_per_processor, vector<uint32_t> &vertex_map)
	{
		uint32_t number_of_edges_to_sample = accumulate(edges_per_processor.begin(), edges_per_processor.end(), 0u);

		/**
		 * Scatter sampling requests
		 */
		uint32_t edges_to_sample_locally;
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
		vector<int32_t> relative_displacements(edges_per_processor);
		relative_displacements.insert(relative_displacements.begin(), 0); // Start at offset zero
		relative_displacements.pop_back();								  // Last element not needed

		vector<int32_t> displacements;
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
			0,
			communicator_);

		assert(global_samples.size() == number_of_edges_to_sample);

		/**
		 * Shuffle to ensure random order for the prefix
		 */
		shuffle(global_samples.begin(), global_samples.end(), random_engine_);

		/**
		 * Incremental prefix scan
		 */
		vertex_map.resize(vertex_count_);
		uint32_t resulting_vertex_count;
		prefixConnectedComponents(
			global_samples,
			vertex_map,
			target_size_,
			resulting_vertex_count);

		vertex_count_ = resulting_vertex_count;
		// Yay we are done
		return resulting_vertex_count;
	}

	/**
	 * @param edges array of {edge_count >= 0} edges
	 * @param [out] vertices_map preallocated map of {vertex_count >= 0} vertices. Will be filled with partitions label from [0, vertex_count)
	 * @param components_count the desired number of connected components
	 * @param [out] resulting_vertex_count how many vertices remain
	 * @param true if the described prefix exists
	 * I don't trust this code, it has just been copied over -- My past self has written it
	 */
	bool prefixConnectedComponents(const vector<Edge> &edges,
								   vector<uint32_t> &vertex_map,
								   uint32_t components_count,
								   uint32_t &resulting_vertex_count)
	{
		DisjointSets<uint32_t> dsets(vertex_map.size());

		if (components_count == 0 || vertex_map.size() == 0)
		{
			return true;
		}

		uint32_t components_active = vertex_map.size();
		bool found = false;

		uint32_t i = 0;
		for (; i < edges.size() && components_active > components_count; i++)
		{
			uint32_t v1_set = dsets.find(edges.at(i).from),
					 v2_set = dsets.find(edges.at(i).to);
			if (v1_set != v2_set)
			{
				components_active--;
				dsets.unify(v1_set, v2_set);
			}
		}

		if (components_active == components_count)
		{
			found = true;
		}

		// Also relabel the components to be in [0, new_vertex_count)!
		const uint64_t mapping_undefined = -1l;
		vector<uint64_t> component_labels(vertex_map.size(), mapping_undefined);
		uint32_t next_label = 0;
		for (uint32_t j = 0; j < vertex_map.size(); j++)
		{
			if (component_labels.at(dsets.find(j)) == mapping_undefined)
			{
				component_labels.at(dsets.find(j)) = next_label++;
			}

			vertex_map.at(j) = component_labels.at(dsets.find(j));
		}

		resulting_vertex_count = components_active;
		return found;
	}

	/**
	 * Maps edge endpoints after contraction.
	 * @param vertex_map The root must contain a valid vertex mapping to apply.
	 *                   vertex_map must be of the right size (number of vertices before applying the mapping).
	 */
	void receiveAndApplyMapping(vector<uint32_t> &vertex_map)
	{
		MPI_Bcast(vertex_map.data(), vertex_map.size(), MPI_UINT32_T, 0, communicator_);

		applyMapping(vertex_map);
		MPI_Bcast(&vertex_count_, 1, MPI_UINT32_T, 0, communicator_);
	}

	/**
	 * Match `initiateSampling` at non-root nodes
	 */
	void acceptSamplingRequest()
	{
		uint32_t edges_to_sample_locally;
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
			0,
			communicator_);
	}

	/**
	 * Apply the map to all endpoints, dropping loops
	 * @param vertex_map
	 */
	void applyMapping(const vector<uint32_t> &vertex_map)
	{
		vector<Edge> updated_edges;

		for (auto edge : edges_slice_)
		{
			edge.from = vertex_map.at(edge.from);
			edge.to = vertex_map.at(edge.to);
			if (edge.from != edge.to)
			{
				updated_edges.push_back(edge);
			}
		}

		edges_slice_.swap(updated_edges);
	}

	/**
	 * Sample `edge_count` edges locally, prop. to their weight
	 * @param edge_count
	 * @return The edge sample
	 */
	// virtual vector<Edge> sample(uint32_t edge_count) = 0;
};
