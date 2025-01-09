#include "mpi_parallel_cc_master.hpp"

vector<uint32_t>& par_MPI_master_deterministic_cc(int rank, int group_size, uint32_t nNodes, uint32_t nEdges, const vector<Edge>& edges, vector<uint32_t>& labels, int* iteration) 
{
	// Increment the iteration
	(*iteration)++;
		
    string str = "Iteration " + to_string(*iteration) + " Number of edges: " + to_string(edges.size()) + "\n";
    cout << str;

	// Calculate the number of edges to send to each processor 
	vector<int> edges_per_proc = calculate_edges_per_processor(rank, group_size, edges);
	// Calculate the displacements for the scatterv function
	vector<int> displacements = calculate_displacements(rank, group_size, edges_per_proc);

	// Send and receive the edges number
	uint32_t nEdges_local;
	MPI_Scatter(edges_per_proc.data(), 1, MPI_UINT32_T, &nEdges_local, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);
	
	// Base case
	if(nEdges_local == 0 || nNodes == 0) 
		return labels;

	// Allocate memory for slice of edges
	vector<Edge> edges_slice(nEdges_local);

	// Send a slice of the edges to each process
	MPI_Scatterv(edges.data(), edges_per_proc.data(), displacements.data(), MPIEdge::edge_type, edges_slice.data(), nEdges_local, MPIEdge::edge_type, 0, MPI_COMM_WORLD);
	// Broadcast the labels
	MPI_Bcast(labels.data(), nNodes, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	#if DEBUG
	string str = "";
	str += "MASTER Rank: " + to_string(rank) + " Edges: ";
	// Print the received edges
	for (uint32_t i = 0; i < nEdges_local; i++)
		str += "(" + to_string(edges_slice[i].from) + "," + to_string(edges_slice[i].to) + ") ";
	str += "\n";
	cout << str;
	#endif

	// Count the number of hooks
	pair<uint32_t, uint32_t> hooks = count_hooks(edges_slice);
	uint32_t hooks_small_2_large = hooks.first;
	uint32_t hooks_large_2_small = hooks.second;

	// AllReduce the number of hooks (Use of MPI_IN_PLACE option)
	MPI_Allreduce(MPI_IN_PLACE, &hooks_small_2_large, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce(MPI_IN_PLACE, &hooks_large_2_small, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);

	// Choose the hook direction
	choose_hook_direction(edges_slice, hooks_small_2_large, hooks_large_2_small, labels);

	// Merge the labels
	if (hooks_small_2_large >= hooks_large_2_small)
		MPI_Reduce(MPI_IN_PLACE, labels.data(), nNodes, MPI_UINT32_T, MPI_MAX, 0, MPI_COMM_WORLD);
	else
		MPI_Reduce(MPI_IN_PLACE, labels.data(), nNodes, MPI_UINT32_T, MPI_MIN, 0, MPI_COMM_WORLD);

	// Find the roots for every node
	find_roots(nNodes, labels);

	// Broadcast the labels
	MPI_Bcast(labels.data(), nNodes, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	// Mark new edges
	vector<uint32_t> marked_edges_local = mark_new_edges(edges_slice, labels);

	// Allocate memory for the marked edges
	vector<uint32_t> marked_edges(nEdges);

	// Gather the marked edges
	MPI_Gatherv(marked_edges_local.data(), nEdges_local, MPI_UINT32_T, marked_edges.data(), edges_per_proc.data(), displacements.data(), MPI_UINT32_T, 0, MPI_COMM_WORLD);

	// Compute the prefix sum of the marked edges
	vector<uint32_t> prefix_sum = compute_prefix_sum(marked_edges);

	// Free the memory of the marked edges local
	vector<uint32_t>().swap(marked_edges_local);
	// Free the memory of the marked edges
	vector<uint32_t>().swap(marked_edges);
	// Allocate memory for the next edges
	vector<Edge> nextEdges(prefix_sum.back());

	


	return labels;
}