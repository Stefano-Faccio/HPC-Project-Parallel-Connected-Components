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

vector<int> calculate_displacements(int rank, int group_size, const vector<int>& edges_per_processor)
{
	vector<int> displacements(group_size, 0);
	for (int i = 1; i < group_size; i++)
		displacements[i] = displacements[i - 1] + edges_per_processor[i - 1];
	return displacements;
}

pair<vector<int>, vector<int>> calculate_edges_per_processor_and_displacements(int rank, int group_size, const vector<Edge>& edges)
{
	vector<int> edges_per_processor = calculate_edges_per_processor(rank, group_size, edges);
	vector<int> displacements = calculate_displacements(rank, group_size, edges_per_processor);
	return make_pair(edges_per_processor, displacements);
}