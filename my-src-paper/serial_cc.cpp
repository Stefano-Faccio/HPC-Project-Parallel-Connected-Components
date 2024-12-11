#include "utils/GraphInputIterator.hpp"
#include "utils/DisjointSets.hpp"
#include <unordered_map>
#include <iostream>
#include <cstdint>

using namespace std;

int main(int argc, char* argv[])
{
	if (argc != 2 ) {
		cout << "Usage: connectivity INPUT_FILE" << endl;
		return 1;
	}

	//Open the file and read the number of vertices and edges
	GraphInputIterator input(argv[1]);
	cout << "Vertex count: " << input.vertexCount() << " Edge count: " << input.edgeCount() << endl;
	
	//Create a disjoint set with all the vertices 
	DisjointSets<uint32_t> disjoint_set(input.vertexCount());

	uint32_t real_edge_count = 0;
	for (auto edge : input) {
		//Check that the edge is valid i.e. the nodes are in the graph
		assert(edge.from < input.vertexCount());
		assert(edge.to < input.vertexCount());
		//Check that the edge is not a self loop
		if (edge.to != edge.from) {
			disjoint_set.unify(edge.from, edge.to);
			real_edge_count++;
		}
	}

	//Count the number of connected components
	unordered_map<uint32_t, uint32_t> components;
	for (uint32_t i = 0; i < input.vertexCount(); i++) {
		uint32_t rep = disjoint_set.find(i);
		if (components.find(rep) == components.end()) {
			components.insert(make_pair(rep, 1));
		} else {
			components.at(rep)++;
		}
	}

	cout << "Connected components: " << components.size() << endl;
}
