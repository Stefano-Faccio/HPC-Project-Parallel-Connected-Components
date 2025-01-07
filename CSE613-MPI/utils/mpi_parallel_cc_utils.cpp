#include "mpi_parallel_cc_utils.hpp"

vector<int> calculate_edges_per_processor(int rank, int group_size, const vector<Edge>& edges)
{
	// Calculate the number of edges to send to each processor
	vector<int> edges_per_processor(group_size, edges.size() / group_size);
	// Calculate the number of remaining edges
	uint32_t remaining_edges = edges.size() % group_size;
	// Distribute the remaining edges
	assert(remaining_edges < (uint32_t)group_size);
	for (uint32_t i = 0; i < remaining_edges; i++)
		edges_per_processor[i]++;

	return edges_per_processor;
}