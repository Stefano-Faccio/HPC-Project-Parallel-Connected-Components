#pragma once

#include "Edge.hpp"
#include "GraphInputIterator.hpp"
#include <mpi.h>
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>

using namespace std;

// Inherits from Edge
class UnweightedIteratedSparseSampling : public Edge
{
private:
	MPI_Comm communicator_;
	const float epsilon_ = 0.09f;
	const float delta_ = 0.2f;
	int32_t rank_, color_, group_size_;
	vector<Edge> edges_slice_;
	MPI_Datatype mpi_edge_t_;
	u_int32_t target_size_;
	u_int32_t vertex_count_;
	u_int32_t initial_vertex_count_, initial_edge_count_;

	u_int32_t countEdges()
	{
		u_int32_t local_edges = edges_slice_.size(), edges;
		MPI_Allreduce(&local_edges, &edges, 1, MPI_UINT32_T, MPI_SUM, communicator_);
		return edges;
	}

	// A vector whose entries correspond to the number of edges to sample at that processor
	vector<u_int32_t> edgesToSamplePerProcessor(vector<u_int32_t> edges_available_per_processor)
	{
		u_int32_t total_edges = (u_int32_t)accumulate(edges_available_per_processor.begin(), edges_available_per_processor.end(), 0);
		u_int32_t number_of_edges_to_sample = min(u_int32_t(pow((float)initial_vertex_count_, 1 + epsilon_ / 2) * (1 + delta_)), total_edges);
		u_int32_t sparsity_threshold = u_int32_t(float(3) / (delta_ * delta_) * log(group_size_ / 0.9f));
		u_int32_t remaining_edges = number_of_edges_to_sample;

		vector<u_int32_t> edges_per_processor(group_size_, 0);

		// First look at processors with few edges
		for (u_int32_t i = 0; i < group_size_; i++)
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
		for (u_int32_t i = 0; i < group_size_; i++)
		{
			if (edges_per_processor.at(i) != 0)
			{
				continue; // Skip maxed-out processors
			}

			edges_per_processor.at(i) = min(
				(u_int32_t)(double(number_of_edges_to_sample) * edges_available_per_processor.at(i) / total_edges),
				edges_available_per_processor.at(i));
			remaining_edges -= edges_per_processor.at(i);
		}

		// We cannot give the remaining requests to just any processor, as we did previously. Distribute them among
		// the ones with the capacity
		u_int32_t spillover = 0;
		while (remaining_edges > 0)
		{
			if (edges_available_per_processor.at(spillover) > edges_per_processor.at(spillover))
			{
				u_int32_t delta = min(edges_available_per_processor.at(spillover) - edges_per_processor.at(spillover), (u_int32_t)remaining_edges);
				edges_per_processor.at(spillover) += delta;
				remaining_edges -= delta;
			}
			spillover++;
		}

		return edges_per_processor;
	}

	// Edges per rank, at root only
	vector<u_int32_t> edgesAvailablePerProcessor()
	{
		u_int32_t available = (u_int32_t)edges_slice_.size(); // MPI displacement types are rather unfortunate
		vector<u_int32_t> edges_per_processor;

		if (master())
		{
			edges_per_processor.resize(group_size_);
		}

		MPI_Gather(&available, 1, MPI_UINT32_T, edges_per_processor.data(), 1, MPI_UINT32_T, 0, communicator_);

		return edges_per_processor; // NRVO
	}

public:
	UnweightedIteratedSparseSampling(MPI_Comm communicator, u_int32_t color, int32_t group_size, int32_t target_size, u_int32_t vertex_count, u_int32_t edge_count) : communicator_(communicator), color_(color), group_size_(group_size), target_size_(target_size), vertex_count_(vertex_count), initial_vertex_count_(vertex_count), initial_edge_count_(edge_count)
	{
		MPI_Comm_rank(communicator_, &rank_);
		mpi_edge_t_ = MPIEdge::constructType();
	}

	~UnweightedIteratedSparseSampling() {}

	int32_t rank() const
	{
		return rank_;
	}

	bool master() const
	{
		return rank_ == 0;
	}

	// The root will receive the labels of the connected components in the vector
	u_int32_t connectedComponents(vector<u_int32_t> &connected_components)
	{
		if (vertex_count_ != initial_vertex_count_)
		{
			throw logic_error("Cannot perform CC on a shrinked graph");
		}

		if (master())
		{
			connected_components.resize(vertex_count_);

			for (size_t i = 0; i < vertex_count_; i++)
			{
				connected_components.at(i) = i;
			}
		}

		// We could use just one edge info exchange per round
		while (countEdges() > 0)
		{
			vector<u_int32_t> vertex_map(vertex_count_);

			if (master())
			{
				initiateSampling(edgesToSamplePerProcessor(edgesAvailablePerProcessor()), vertex_map);

				for (size_t i = 0; i < connected_components.size(); i++)
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
		u_int32_t slice_portion = input.edgeCount() / group_size_;
		u_int32_t slice_from = slice_portion * rank_;
		// The last node takes any leftover edges
		bool last = rank_ == group_size_ - 1;
		u_int32_t slice_to = last ? input.edgeCount() : slice_portion * (rank_ + 1);

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

	u_int32_t initiateSampling(vector<u_int32_t> edges_per_processor, vector<u_int32_t> &vertex_map)
	{
		u_int32_t number_of_edges_to_sample = accumulate(edges_per_processor.begin(), edges_per_processor.end(), 0u);

		/**
		 * Scatter sampling requests
		 */
		u_int32_t edges_to_sample_locally;
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
		vector<u_int32_t> relative_displacements(edges_per_processor);
		relative_displacements.insert(relative_displacements.begin(), 0); // Start at offset zero
		relative_displacements.pop_back();								  // Last element not needed

		vector<u_int32_t> displacements;
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
		// shuffle(global_samples.begin(), global_samples.end());

		/**
		 * Incremental prefix scan
		 */
		vertex_map.resize(vertex_count_);
		u_int32_t resulting_vertex_count;
		prefixConnectedComponents(
			global_samples,
			vertex_map,
			target_size_,
			resulting_vertex_count);

		vertex_count_ = resulting_vertex_count;
		// Yay we are done
		return resulting_vertex_count;
	}
};
