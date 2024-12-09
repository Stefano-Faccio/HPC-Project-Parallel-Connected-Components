#include "UnweightedIteratedSparseSampling.hpp"
#include <algorithm>
#include <vector>
//#include "MPICollector.hpp"

using namespace std;

class UnweightedIteratedSparseSampling : public Edge {
private:

	vector<Edge> sample(u_int32_t edge_count) {
		vector<Edge> edges;
		size_t size = edges_slice_.size();

		if (edge_count == edges_slice_.size()) {
			return edges_slice_;
		} else {
			for (u_int32_t i = 0; i < edge_count; i++) {
				edges.push_back(edges_slice_.at(random_engine_() % size));
			}
		}

		return edges; // NRVO
	}

	u_int32_t UnweightedIteratedSparseSampling::countEdges() {
		u_int32_t local_edges = edges_slice_.size(), edges;
		MPI::Allreduce(&local_edges, &edges, 1, MPI_UINT32_T, MPI_SUM, communicator_);
		return edges;
	}

	vector<int> edgesToSamplePerProcessor(vector<int> edges_available_per_processor) {
		u_int32_t total_edges = (u_int32_t) accumulate(edges_available_per_processor.begin(), edges_available_per_processor.end(), 0);
		u_int32_t number_of_edges_to_sample = min(
				u_int32_t(pow((float) initial_vertex_count_, 1 + epsilon_ / 2) * (1 + delta_)),
				total_edges
		);
		u_int32_t sparsity_threshold = u_int32_t(float(3) / (delta_ * delta_) * log(group_size_ / 0.9f));
		u_int32_t remaining_edges = number_of_edges_to_sample;

		vector<int> edges_per_processor(group_size_, 0);

		// First look at processors with few edges
		for (size_t i = 0; i < group_size_; i++) {
			if (edges_available_per_processor.at(i) <= sparsity_threshold) {
				edges_per_processor.at(i) = edges_available_per_processor.at(i);
				remaining_edges -= edges_available_per_processor.at(i);
				// We don't want to consider these for equal redistribution
				number_of_edges_to_sample -= edges_available_per_processor.at(i);
			}
		}

		// Assign proportional share of edges to every processor. Depending on the balance, this is necessary to
		// counter accumulating +-1 differences. This also ensures that empty slices will not be asked to sample anything.
		for (size_t i = 0; i < group_size_; i++) {
			if (edges_per_processor.at(i) != 0) {
				continue; // Skip maxed-out processors
			}

			edges_per_processor.at(i) = min(
					int(float(number_of_edges_to_sample) * edges_available_per_processor.at(i) / total_edges),
					edges_available_per_processor.at(i)
			);
			remaining_edges -= edges_per_processor.at(i);
		}

		// We cannot give the remaining requests to just any processor, as we did previously. Distribute them among
		// the ones with the capacity
		size_t spillover = 0;
		while (remaining_edges > 0) {
			if (edges_available_per_processor.at(spillover) > edges_per_processor.at(spillover)) {
				int delta = min(edges_available_per_processor.at(spillover) - edges_per_processor.at(spillover), (int) remaining_edges);
				edges_per_processor.at(spillover) += delta;
				remaining_edges -= delta;
			}
			spillover++;
		}

		return edges_per_processor;
	}

	vector<u_int32_t> edgesAvailablePerProcessor() {
		u_int32_t available = (u_int32_t) edges_slice_.size(); // MPI displacement types are rather unfortunate
		vector<u_int32_t> edges_per_processor;

		if (master()) {
			edges_per_processor.resize(group_size_);
		}

		MPI::Gather(&available, 1, MPI_UINT32_T, edges_per_processor.data(), 1, MPI_UINT32_T, root_rank_, communicator_);

		return edges_per_processor; // NRVO
	}

public: 
	void loadSlice(GraphInputIterator & input) {
		// Exposing iterables is sort of hard, this will do for now
		// TODO potentially make GII expose an iterable interface that also takes slicing into account
		u_int32_t slice_portion = input.edgeCount() / group_size_;
		u_int32_t slice_from = slice_portion * rank_;
		// The last node takes any leftover edges
		bool last = rank_ == group_size_ - 1;
		u_int32_t slice_to = last ? input.edgeCount() : slice_portion * (rank_ + 1);

		GraphInputIterator::Iterator iterator = input.begin();
		while (!iterator.end_) {
			if (iterator.position() >= slice_from && iterator.position() < slice_to) {
				edges_slice_.push_back({ iterator->from, iterator->to });
			}
			++iterator;
		}
	}

	u_int32_t connectedComponents(vector<u_int32_t> & connected_components) {
	if (vertex_count_ != initial_vertex_count_) {
		throw logic_error("Cannot perform CC on a shrinked graph");
	}

	if (master()) {
		connected_components.resize(vertex_count_);

		for (size_t i = 0; i < vertex_count_; i++) {
			connected_components.at(i) = i;
		}
	}

	// We could use just one edge info exchange per round
	while (countEdges() > 0) {
		vector<u_int32_t> vertex_map(vertex_count_);

		if (master()) {
			initiateSampling(edgesToSamplePerProcessor(edgesAvailablePerProcessor()), vertex_map);

			for (size_t i = 0; i < connected_components.size(); i++) {
				connected_components.at(i) = vertex_map.at(connected_components.at(i));
			}
		} else {
			edgesAvailablePerProcessor();
			acceptSamplingRequest();
		}

		receiveAndApplyMapping(vertex_map);
	}

	return vertex_count_;
	}

}

