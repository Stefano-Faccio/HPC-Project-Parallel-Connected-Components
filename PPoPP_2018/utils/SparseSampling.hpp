#pragma once

#include <mpi.h>
// Project headers
#include "Edge.hpp"
#include "MPIEdge.hpp"
#include "GraphInputIterator.hpp"
#include "DisjointSets.hpp"
// Standard libraries
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>

using namespace std;

class SparseSampling : public Edge
{
public:
	// MPI related variables
	MPI_Comm communicator_;				// MPI communicator
	MPI_Datatype mpi_edge_t_;			// MPI edge type (MPI_Datatype)
	int32_t group_size_, rank_; 		// size of the group , Rank of the process
	mt19937 random_engine_;				// Mersenne Twister 19937 generator

	// Graph related variables
	uint32_t target_size_, vertex_count_, initial_vertex_count_, initial_edge_count_;
	vector<Edge> edges_slice_; // Slice of the graph edges: block of edges distributed to each process

	// Other variables
	const float epsilon_ = 0.09f;
	const float delta_ = 0.2f;
	
	// Constructor
	SparseSampling(MPI_Comm communicator, int32_t group_size, int32_t rank, int32_t seed, int32_t target_size, uint32_t vertex_count, uint32_t edge_count) : communicator_(communicator), group_size_(group_size), rank_(rank), random_engine_(seed), target_size_(target_size), vertex_count_(vertex_count), initial_vertex_count_(vertex_count), initial_edge_count_(edge_count)
	{
		// Construct the MPI edge type
		mpi_edge_t_ = MPIEdge::constructType();
	}

	// Destructor
	~SparseSampling() {}

	bool master() const
	{
		return rank_ == 0;
	}

	// The root will receive the labels of the connected components in the vector
	uint32_t connectedComponents(vector<uint32_t> &connected_components)
	{
		if (master())
		{
			// Reset the connected components vector to the size of the vertex count
			connected_components.resize(vertex_count_);

			// Initialize the connected components vector
			for (uint32_t i = 0; i < vertex_count_; i++)
				connected_components.at(i) = i;
		}

		// We could use just one edge info exchange per round

		// While there are edges to process in the whole graph
		while (countEdges() > 0) // Count the number of edges in the whole graph
		{
			// Initialize the vertex map with vertex_count_ elements
			vector<uint32_t> vertex_map(vertex_count_); 

			// NOTE:
			//  edgesAvailablePerProcessor() returns the number of edges available to each node: only the master node will have the vector edges_per_processor

			if (master())
			{
				initiateSampling(edgesToSamplePerProcessor(edgesAvailablePerProcessor()), vertex_map);

				for (uint32_t i = 0; i < connected_components.size(); i++)
					connected_components.at(i) = vertex_map.at(connected_components.at(i));
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

	// Load a slice of the graph edges: useful for parallel processing
	void loadSlice(GraphInputIterator &input)
	{
		input.loadSlice(edges_slice_, rank_, group_size_);
	}

	// Count the number of edges in the whole graph
	uint32_t countEdges()
	{
		uint32_t local_edges = edges_slice_.size(), edges;
		// MPI_Allreduce combines values from all processes and distributes the result back to all processes 
		MPI_Allreduce(&local_edges, &edges, 1, MPI_UINT32_T, MPI_SUM, communicator_);
		// Returns the total number of edges in the graph
		return edges;
	}

	// Return a vector of the number of edges that every processor should have after sampling
	vector<int32_t> edgesToSamplePerProcessor(vector<int32_t> edges_available_per_processor)
	{
		// Sum of the number of edges available to each processor to get the total number of edges in the graph
		uint32_t total_edges = (uint32_t)accumulate(edges_available_per_processor.begin(), edges_available_per_processor.end(), 0);

		uint32_t number_of_edges_to_sample = min(uint32_t(pow((float)initial_vertex_count_, 1 + epsilon_ / 2) * (1 + delta_)), total_edges);

		int32_t sparsity_threshold = int32_t(float(3) / (delta_ * delta_) * log(group_size_ / 0.9f));

		uint32_t remaining_edges = number_of_edges_to_sample;
		// Final vector of edges to sample per processor: output of the function
		vector<int32_t> edges_per_processor(group_size_, 0);

		// First look at processors with few edges
		for (int32_t i = 0; i < group_size_; i++)
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
		for (int32_t i = 0; i < group_size_; i++)
		{
			// Skip processors that have already been assigned their share
			if (edges_per_processor.at(i) == 0)
			{
				edges_per_processor.at(i) = min(
					(int32_t)(double(number_of_edges_to_sample) * edges_available_per_processor.at(i) / total_edges),
					edges_available_per_processor.at(i));
				remaining_edges -= edges_per_processor.at(i);
			}
		}

		// We cannot give the remaining requests to just any processor, as we did previously. Distribute them among
		// the ones with the capacity
		uint32_t spillover = 0;
		while (remaining_edges > 0)
		{
			if (edges_available_per_processor.at(spillover) > edges_per_processor.at(spillover))
			{
				int32_t delta = min(edges_available_per_processor.at(spillover) - edges_per_processor.at(spillover), (int32_t)remaining_edges);
				edges_per_processor.at(spillover) += delta;
				remaining_edges -= delta;
			}
			spillover++;
		}

		return edges_per_processor;
	}

	// Returns the number of edges available to each processor (only the master processor will have the vector edges_per_processor)
	vector<int32_t> edgesAvailablePerProcessor()
	{
		// The size of the slice of the graph that each processor has
		uint32_t available = (uint32_t)edges_slice_.size();

		// Only the master processor will have the vector edges_per_processor
		vector<int32_t> edges_per_processor;
		if (master())
			edges_per_processor.resize(group_size_);

		// MPI_Gather gets data from all processes to the root process
		MPI_Gather(&available, 1, MPI_UINT32_T, edges_per_processor.data(), 1, MPI_UINT32_T, 0, communicator_);

		return edges_per_processor; // NRVO: Named Return Value Optimization (Compiler optimization)
	}

	vector<Edge> sample(uint32_t edge_count)
	{
		vector<Edge> edges;
		uint32_t size = edges_slice_.size();

		// If the number of edges to sample is equal to the number of edges that the processor has, return the whole slice
		if (edge_count == edges_slice_.size())
		{
			return edges_slice_;
		}
		else
		{
			// Randomly sample edge_count edges from the slice
			// BUT 
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

		//Count of edges to sample for each processor
		uint32_t edges_to_sample_locally;
		//MPI_Scatter sends data from one process to all other processes in a communicator 
		MPI_Scatter(edges_per_processor.data(), 1, MPI_INT, &edges_to_sample_locally, 1, MPI_INT, 0, communicator_);

		//Take part in sampling
		vector<Edge> samples = sample(edges_to_sample_locally);

		// Gather samples
		// Allocate space
		vector<Edge> global_samples(number_of_edges_to_sample);
		// Calculate displacement vector
		vector<int32_t> relative_displacements(edges_per_processor); // Copy
		// Add a zero at the beginning and remove the last element
		relative_displacements.insert(relative_displacements.begin(), 0); // Start at offset zero
		relative_displacements.pop_back();								  // Last element not needed

		vector<int32_t> displacements;
		partial_sum(relative_displacements.begin(), relative_displacements.end(), back_inserter(displacements));
		/*
		Explanation of partial_sum:
		int val[] = {1,2,3,4,5};
		vector<int> result;
		partial_sum(val, val + 5, result);
		result now contains: 1 3 6 10 15
		*/

		// Gather the samples
		// MPI_Gatherv Gathers together values from a group of processes. The values can have different lengths
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

		//Shuffle to ensure random order for the prefix
		shuffle(global_samples.begin(), global_samples.end(), random_engine_);

		/**
		 * Incremental prefix scan
		 */
		vertex_map.resize(vertex_count_); // Resize the vertex_map to the size of the vertex count not needed but okay
		uint32_t resulting_vertex_count = 0;
		prefixConnectedComponents(global_samples, vertex_map, target_size_, resulting_vertex_count);

		vertex_count_ = resulting_vertex_count;

		return resulting_vertex_count;
	}

	/**
	 * @param edges array of {edge_count >= 0} edges
	 * @param [out] vertices_map preallocated map of {vertex_count >= 0} vertices. Will be filled with partitions label from [0, vertex_count)
	 * @param components_count the desired number of connected components
	 * @param [out] resulting_vertex_count how many vertices remain
	 */
	void prefixConnectedComponents(const vector<Edge> &edges, vector<uint32_t> &vertex_map, uint32_t components_count, uint32_t &resulting_vertex_count)
	{
		// Create a disjoint set for the vertices
		DisjointSets<uint32_t> dsets(vertex_map.size());

		if (components_count == 0 || vertex_map.size() == 0)
		{
			return;
		}

		uint32_t components_active = vertex_map.size();

		for (uint32_t i = 0; i < edges.size() && components_active > components_count; i++)
		{
			uint32_t v1_set = dsets.find(edges.at(i).from), v2_set = dsets.find(edges.at(i).to);
			if (v1_set != v2_set)
			{
				components_active--;
				dsets.unify(v1_set, v2_set);
			}
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
		return ;
	}

	/**
	 * Maps edge endpoints after contraction.
	 * @param vertex_map The root must contain a valid vertex mapping to apply.
	 *                   vertex_map must be of the right size (number of vertices before applying the mapping).
	 */
	void receiveAndApplyMapping(vector<uint32_t> &vertex_map)
	{
		// MPI_Bcast Broadcasts a message from the process with rank "root" to all other processes of the communicator 
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
};
