#pragma once

#include <vector>
#include <tuple>
#include "Utils.hpp"
#include "DisjointSets.hpp"

//#include "prng_engine.hpp"
//#include "UnweightedGraph.hpp"

using namespace std;

class AdjacencyListGraph {
public:

private:
	vector<Edge> edges_;
	unsigned vertex_count_;
	DisjointSets<unsigned> disjoint_sets_;
	// This is hairy. We keep this const reference to a immutable list of edges and use it up until we do a finalizeContractionPhase(),
	// then we build our internal representation (basically CoW optimization) and switch the reference to point to it.
	vector<Edge> const * parent_edges_;
public:
	AdjacencyListGraph(unsigned vertex_count) : vertex_count_(vertex_count), disjoint_sets_(vertex_count + 1), parent_edges_(&edges_)
	{}

	AdjacencyListGraph(const AdjacencyListGraph& that) : AdjacencyListGraph(that.vertex_count_, that.edges_)
	{}

	/**
	 * Edge source has to implement:
	 * - unsigned vertexCount()
	 * - InputIterator-compatible iterator type Iterator over AdjacencyListGraph::Edge
	 * - Iterator begin()
	 * - Iterator end()
	 */
	template<class EdgeSource>
	static AdjacencyListGraph fromIterator(EdgeSource & source) {
		AdjacencyListGraph graph(source.vertexCount());
		for (auto e : source)
			graph.addEdge(e);
		return graph;
	}

	// Handy when passing edge arrays over MPI
	AdjacencyListGraph(unsigned vertex_count, const vector<Edge> & edges) : edges_(edges), vertex_count_(vertex_count), disjoint_sets_(vertex_count + 1), parent_edges_(&edges_)
	{}

	AdjacencyListGraph(unsigned vertex_count, const vector<Edge> & edges, bool _use_parent_edges) :  vertex_count_(vertex_count), disjoint_sets_(vertex_count + 1), parent_edges_(&edges)
	{}

	AdjacencyListGraph(const AdjacencyListGraph& that, bool _use_parent_edges) : AdjacencyListGraph(that.vertex_count_, that.edges_, _use_parent_edges)
	{}

	void addEdge(unsigned from, unsigned to);

	void addEdge(Edge e);

	/**
	 * return The graph with singleton vertices removed and edges renamed so that it is a connected graph on [0, vertex_count)
	 */
	std::unique_ptr<AdjacencyListGraph> compact() const;

	/**
	 * Contract the edge `from -- to`
	 * If such an edge does not exist, no action is taken
	 * Loop are not removed and parallel edges are not merged after the contraction
	 * The edge count will not be accurate while the representation is denormalized
	 */
	void weaklyContractEdge(unsigned from, unsigned to);

	/**
	 * Contraction with loop removal and edge merging
	 */
	void contractEdge(unsigned from, unsigned to);

	/**
	 * Remove loop and merge edges after a series of weaklyContractEdge() calls
	 */
	//void finalizeContractionPhase(sitmo::prng_engine * random);

	unsigned vertex_count() const
	{
		return vertex_count_;
	}

	unsigned edge_count() const
	{
		return unsigned(parent_edges_->size());
	}

	unsigned maxVertexID() const;

	vector<Edge> const & edges() const;

	friend std::ostream & operator<< (std::ostream & out, AdjacencyListGraph const & graph);

private:
	std::tuple<unsigned, unsigned> normalize(unsigned v1, unsigned v2) const;
};
