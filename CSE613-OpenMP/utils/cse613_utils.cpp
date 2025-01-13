#include "cse613_utils.hpp"

vector<Edge> find_rank_and_remove_edges(uint32_t nNodes, const vector<Edge>& edges, vector<uint32_t>& labels)
{
	// This vectors will be deallocated when the function ends
	vector<uint32_t> edges_mark(edges.size(), 0), prefix_sum(edges.size(), 0);
	// Vector to store the next edges for the recursive call
	vector<Edge> nextEdges;
	// Temporary variable for the prefix sum
	uint32_t prefix_sum_temp = 0;

	#pragma omp parallel shared(nNodes, edges, labels, edges_mark, prefix_sum, nextEdges) 
	{
		// Prepare to remove edges inside the same group
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;

			// If the nodes are in different groups, mark the edge
			if(labels[from] != labels[to])
				//Not a race condition because each thread writes to a different index
				edges_mark[i] = 1;
		}

		// if __GNUC__ >= 10, use the new omp scan directive
		// Otherwise, use the old way to do a prefix sum
		#if __GNUC__ >= 10
		// Prefix sum with parallel for
		#pragma omp for reduction(inscan, + : prefix_sum_temp)
        for (uint32_t i = 0; i < edges.size(); i++) {
            
            prefix_sum_temp += edges_mark[i]; 
            #pragma omp scan inclusive(prefix_sum_temp)
            prefix_sum[i] = prefix_sum_temp;
        }
		#else
		// Prefix sum sequential
		#pragma omp single
		{
			// Prefix sum
			prefix_sum[0] = edges_mark[0];
			for(uint32_t i = 1; i < edges.size(); i++)
			{
				prefix_sum[i] = edges_mark[i] + prefix_sum[i - 1];
			}
		}
		#endif

		// Allocate memory for the nextEdges vector 
		// Deallocate memory from the previous used vector
		#pragma omp single
		{
			vector<uint32_t>().swap(edges_mark); // Free edges_mark memory
			nextEdges.resize(prefix_sum[edges.size() - 1]);
		}
		
		// Copy only edges that are between different groups
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			uint32_t from = edges[i].from;
			uint32_t to = edges[i].to;

			// If the nodes are in different groups, add the edge to the nextEdges vector
			if(labels[from] != labels[to])
				//Not a race condition because:
				// if the condition is true, edges_mark[i] will be 1, so prefix_sum[i] will be different from prefix_sum[i-1]
				// Insert the edge in the correct position
				nextEdges[prefix_sum[i] - 1] = (labels[from] < labels[to] ? Edge{labels[from], labels[to]} : Edge{labels[to], labels[from]});			
		}
	}

	return nextEdges;
}

void map_results_back(uint32_t nNodes, const vector<Edge>& edges, const vector<uint32_t>& labels, vector<uint32_t>& map)
{
	#pragma omp parallel for shared(edges, labels, map)
	for(uint32_t i = 0; i < edges.size(); i++)
	{
		uint32_t from = edges[i].from;
		uint32_t to = edges[i].to;

		// The race condition is only in the labels that has been previously changed
		// So, again, atomic writes is sufficient
		
		if(to == labels[from])
		{
			#pragma omp atomic write
			map[from] = map[to];
		}
		else if(from == labels[to])
		{
			#pragma omp atomic write
			map[to] = map[from];
		}
	}

}

void find_roots(uint32_t nNodes, vector<uint32_t>& labels)
{
	// Let's break down the possibility of concurrency problems:
	// 1. If a node is a root, it will basically not be changed
	// 2. If a node is a leaf, it will be just written once and never read
	// 3. If a node is in the middle of a chain, it will be read and written at least once -> problem

	// I have done some tests with atomic read and atomic write, but it is slower 
	// than just reading and writing without atomic operations and give the same results
	// because the algorithm is sequantial consistent, so it is fine

	bool found = true;

	while(found)
	{
		found = false;

		#if false
		#pragma omp parallel for shared(nNodes, labels) reduction(||:found)
		for(uint32_t i = 0; i < nNodes; i++)
		{
			// I cannot safely read labels[labels[i]] because another thread can modify it
			// With atomic read, I am sure that the value is "pulled" from the memory
			uint32_t grandpa, new_gramdpa;
			#pragma omp atomic read
			grandpa = labels[labels[i]];
			#pragma omp atomic read
			new_gramdpa = labels[grandpa];
			
			// I cannot safely write labels[i] because another thread can read it 
			// With atomic write, I am sure that the value is "pushed" to the memory
			#pragma omp atomic write
			labels[i] = grandpa;
			// Maybe an flush is enough here instead of atomic write

			if(grandpa != new_gramdpa)
				found = true;
		}
		#else
		#pragma omp parallel for shared(nNodes, labels, found)
		for(uint32_t i = 0; i < nNodes; i++)
		{
			labels[i] = labels[labels[i]];

			if(labels[i] != labels[labels[i]])
				found = true;
		}
		#endif
	}

	return;
}