#include "slave.hpp"

void par_MPI_slave_deterministic_cc(int rank, int group_size, uint32_t nNodes)
{
	// ---------------------- Receive the data ----------------------

	// Receive the number of total edges
	uint32_t nEdges;
	MPI_Bcast(&nEdges, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	// Receive the number of local edges
	uint32_t nEdges_local;
	MPI_Scatter(nullptr, 1, MPI_UINT32_T, &nEdges_local, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	// Allocate memory for the slice of edges
	vector<Edge> edges_slice(nEdges_local);

	// Receive the slice of edges
	MPI_Scatterv(nullptr, nullptr, nullptr, MPIEdge::edge_type, edges_slice.data(), nEdges_local, MPIEdge::edge_type, 0, MPI_COMM_WORLD);
	
	// Allocate memory for the labels
	vector<uint32_t> labels(nNodes);
	// Receive the labels
	MPI_Bcast(labels.data(), nNodes, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	#if DEBUG
	string str = "";
	str += "SLAVE Rank: " + to_string(1) + " Edges: ";
	// Print the received edges
	for (uint32_t i = 0; i < nEdges_local; i++)
		str += "(" + to_string(edges_slice[i].from) + "," + to_string(edges_slice[i].to) + ") ";
	str += "\n";
	cout << str;
	#endif

	// ---------------------- Count the number of hooks ----------------------

	// Count the number of hooks
	pair<uint32_t, uint32_t> hooks = count_hooks(edges_slice);
	uint32_t hooks_small_2_large = hooks.first;
	uint32_t hooks_large_2_small = hooks.second;

	// AllReduce the number of hooks (Use of MPI_IN_PLACE option)
	MPI_Allreduce(MPI_IN_PLACE, &hooks_small_2_large, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce(MPI_IN_PLACE, &hooks_large_2_small, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);

	// ---------------------- Choose the hook direction ----------------------

	// Choose the hook direction
	choose_hook_direction(edges_slice, hooks_small_2_large, hooks_large_2_small, labels);

	// Merge the labels
	if (hooks_small_2_large >= hooks_large_2_small)
		MPI_Reduce(labels.data(), nullptr, nNodes, MPI_UINT32_T, MPI_MAX, 0, MPI_COMM_WORLD);
	else
		MPI_Reduce(labels.data(), nullptr, nNodes, MPI_UINT32_T, MPI_MIN, 0, MPI_COMM_WORLD);

	// -------------------------------- Find the roots --------------------------------

	// Wait the master to find the roots

	// Broadcast the labels
	MPI_Bcast(labels.data(), nNodes, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	// ---------------------- Mark new edges ----------------------

	// Mark new edges
	vector<uint32_t> marked_edges_local = mark_new_edges(edges_slice, labels);

	// ---------------------- Compute the prefix sum ----------------------

	vector<uint32_t> prefix_sum = compute_prefix_sum(rank, group_size, nEdges, nEdges_local, marked_edges_local);

	// Free the memory of the marked edges
	vector<uint32_t>().swap(marked_edges_local);

	// Wait the master to compute the prefix sum

	// ---------------------- Create the next edges ----------------------

	// Compute the next edges
	vector<Edge> nextEdges = compute_next_edges(edges_slice, labels, prefix_sum);
	// Free the memory of the prefix sum
	vector<uint32_t>().swap(prefix_sum);

}