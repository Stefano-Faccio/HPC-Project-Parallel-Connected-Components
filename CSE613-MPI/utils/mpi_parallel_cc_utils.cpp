#include "mpi_parallel_cc_utils.hpp"

vector<int> calculate_edges_per_processor(int group_size, const vector<Edge>& edges)
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

vector<int> calculate_displacements(int group_size, const vector<int>& edges_per_processor)
{
	vector<int> displacements(group_size, 0);
	for (int i = 1; i < group_size; i++)
		displacements[i] = displacements[i - 1] + edges_per_processor[i - 1];
	return displacements;
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
			string str = "Master - self loop found: " + to_string(from) + " " + to_string(to) + "\n";
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

vector<Edge> compute_next_edges(const vector<Edge>& edges, const vector<uint32_t>& labels)
{
	vector<Edge> nextEdges;

	// Compute the next edges
	for(uint32_t i = 0; i < edges.size(); i++)
	{
		uint32_t from = edges[i].from;
		uint32_t to = edges[i].to;

		// If the nodes are in different groups, add the edge
		if(labels[from] != labels[to])
		{
			Edge edge = from < to ? Edge{labels[from], labels[to]} : Edge{labels[to], labels[from]};
			nextEdges.push_back(edge);
		}
	}

	return nextEdges;
}