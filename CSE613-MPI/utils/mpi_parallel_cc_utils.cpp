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

pair<uint32_t, uint32_t> count_hooks(const vector<Edge>& edges)
{
	uint32_t hooks_small_2_large = 0, hooks_large_2_small = 0;
	for (uint32_t i = 0; i < edges.size(); i++)
	{
		uint32_t from = edges[i].from;
		uint32_t to = edges[i].to;

		if (from < to)
			hooks_small_2_large++;
		else if (from > to)
			hooks_large_2_small++;
		else
		{
			string str = "Rank: " + to_string(0) + " self loop found: " + to_string(from) + " " + to_string(to) + "\n";
			cerr << str;
		}			
	}

	return make_pair(hooks_small_2_large, hooks_large_2_small);
}

void choose_hook_direction(const vector<Edge>& edges, uint32_t hooks_small_2_large, uint32_t hooks_large_2_small, vector<uint32_t>& labels)
{
	//Choose hook direction to maximize #hooks
	for(uint32_t i = 0; i < edges.size(); i++)
	{
		uint32_t from = edges[i].from;
		uint32_t to = edges[i].to;

		if(from < to)
		{
			if(hooks_small_2_large >= hooks_large_2_small)
				labels[from] = to;
		}
		else
		{
			if(hooks_small_2_large < hooks_large_2_small)
				labels[from] = to;
		}
	}
}

void find_roots(uint32_t nNodes, vector<uint32_t>& labels)
{
	bool found = true;

	while(found)
	{
		found = false;

		for(uint32_t i = 0; i < nNodes; i++)
		{
			labels[i] = labels[labels[i]];

			if(labels[i] != labels[labels[i]])
				found = true;
		}
	}

	return;
}

vector<uint32_t> mark_new_edges(const vector<Edge>& edges, const vector<uint32_t>& labels) 
{
	vector<uint32_t> edges_mark(edges.size(), 0);
	
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;

			// If the nodes are in different groups, mark the edge
			if(labels[from] != labels[to])
				edges_mark[i] = 1;
		}
	return edges_mark;
}

vector<uint32_t> compute_prefix_sum(int rank, int group_size, uint32_t nEdges, uint32_t nEdges_local, const vector<uint32_t>& marked_edges)
{
	vector<uint32_t> prefix_sum(marked_edges.size());

	/*
	// Compute local partial prefix sum of the marked edges
	vector<uint32_t> partial_prefix_sum(nEdges_local);	
	partial_sum(marked_edges.begin(), marked_edges.end(), partial_prefix_sum.begin());

	// Scan the partial prefix sum (with that, every node has the local prefix sum of the marked edges)
	uint32_t local_sum = partial_prefix_sum.back();
	MPI_Scan(MPI_IN_PLACE, &local_sum, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
	vector<uint32_t> offset(group_size);
	// Spread the offset sum to all nodes
	MPI_Allgather(&local_sum, 1, MPI_UINT32_T, offset.data(), 1, MPI_UINT32_T, MPI_COMM_WORLD);
	*/



	return prefix_sum;
}

vector<Edge> compute_next_edges(const vector<Edge>& edges, const vector<uint32_t>& labels, const vector<uint32_t>& prefix_sum)
{
	// Allocate memory for the next edges
	vector<Edge> nextEdges(prefix_sum.back());

	// Compute the next edges
	for(uint32_t i = 0; i < edges.size(); i++)
	{
		uint32_t from = edges[i].from;
		uint32_t to = edges[i].to;

		// If the nodes are in different groups, add the edge
		if(labels[from] != labels[to])
			nextEdges[prefix_sum[i]] = from < to ? Edge{labels[from], labels[to]} : Edge{labels[to], labels[from]};
	}

	return nextEdges;
}