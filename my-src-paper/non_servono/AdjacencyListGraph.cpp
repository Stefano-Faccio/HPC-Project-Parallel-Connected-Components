#include "AdjacencyListGraph.hpp"
#include <unordered_set>

using namespace std;

void AdjacencyListGraph::addEdge(unsigned from, unsigned to)
{
	if (&edges_ != parent_edges_) {
		edges_.assign(parent_edges_->begin(), parent_edges_->end());
		parent_edges_ = &edges_;
	}
	assert(from < vertex_count_);
	assert(to < vertex_count_);

	std::tie(from, to) = normalize(from, to);
	edges_.push_back({ from, to });
}

void AdjacencyListGraph::addEdge(Edge e)
{
	addEdge(e.from, e.to);
}

unsigned AdjacencyListGraph::maxVertexID() const
{
	unsigned max = { 0 };
	for (auto const & edge : *parent_edges_)
		max = std::max(max, std::max(edge.from, edge.to));

	return max;
}

/**
 * @return The graph with singleton vertices removed and edges renamed so that it is a connected graph on [0, vertex_count)
 */
std::unique_ptr<AdjacencyListGraph> AdjacencyListGraph::compact() const {
	std::unique_ptr<AdjacencyListGraph> result(new AdjacencyListGraph(vertex_count()));
	// Mapping old_id -> new_id
	std::vector<unsigned> mapping(maxVertexID() + 1, std::numeric_limits<unsigned>::max());

	// Construct the mapping
	unsigned next_id { 0 };
	for (auto const & edge : *parent_edges_) {
		if (mapping.at(edge.from) == std::numeric_limits<unsigned>::max())
			mapping.at(edge.from) = next_id++;
		if (mapping.at(edge.to) == std::numeric_limits<unsigned>::max())
			mapping.at(edge.to) = next_id++;
	}

	for (auto const & edge : *parent_edges_)
		result->addEdge(mapping.at(edge.from), mapping.at(edge.to));

	return result;
}

void AdjacencyListGraph::weaklyContractEdge(unsigned from, unsigned to)
{
	// std::cout << "Contracting " << from << " -- " << to << std::endl;
	// Since we are in a weak contraction phase, we need to check for the actual state of partitions
	unsigned actual_from = { disjoint_sets_.find(from) },
			actual_to = { disjoint_sets_.find(to) };

	// The edge does not exist. This is OK since this might be due to a previous contraction from the same sample
	if (actual_from == actual_to)
		return;

	disjoint_sets_.unify(actual_from, actual_to);
	assert(disjoint_sets_.find(actual_from) == disjoint_sets_.find(actual_to));

	vertex_count_--;
}

/**
 * Contraction with loop removal and edge merging
 *
 * Beware: This does not check for existence of the edge! It will happily merge any vertices you pass to it!
 */
void AdjacencyListGraph::contractEdge(unsigned from, unsigned to)
{
	weaklyContractEdge(from, to);
	//finalizeContractionPhase();
	// TODO assert canonical form
}


AdjacencyListGraph::vector<Edge> const & AdjacencyListGraph::edges() const
{
	return *parent_edges_;
}


std::ostream & operator<< (std::ostream & out, AdjacencyListGraph const & graph)
{
	for (auto & edge : *graph.parent_edges_)
		out << edge << std::endl;
	return out;
}

std::tuple<unsigned, unsigned> AdjacencyListGraph::normalize(unsigned v1, unsigned v2) const
{
	return std::tuple<unsigned, unsigned>(std::min(v1, v2), std::max(v1, v2));
}
