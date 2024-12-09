#pragma once

#include "Edge.hpp"
#include "GraphInputIterator.hpp"
#include "IteratedSparseSampling.hpp"
#include <vector>

//Inherits from Edge
class UnweightedIteratedSparseSampling : public Edge {
private:
	const float epsilon_ = 0.09f;
	const float delta_ = 0.2f;

	u_int32_t countEdges();

	// A vector whose entries correspond to the number of edges to sample at that processor
	vector<u_int32_t> edgesToSamplePerProcessor(vector<u_int32_t> edges_available_per_processor);

	virtual vector<Edge> sample(u_int32_t edge_count);

	//Edges per rank, at root only
	vector<u_int32_t> edgesAvailablePerProcessor();

public:
	using IteratedSparseSampling<Edge>::IteratedSparseSampling;

	//The root will receive the labels of the connected components in the vector
	u_int32_t connectedComponents(vector<u_int32_t> & connected_components);

	void loadSlice(GraphInputIterator & input);
};


