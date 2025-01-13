#include "utils/GraphInputIterator.hpp"
#include "utils/DisjointSets.hpp"
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <chrono>
#include <cstdint>

using namespace std;

int main(int argc, char* argv[])
{
	if (argc != 2 ) {
		cout << "Usage: connectivity INPUT_FILE" << endl;
		return 1;
	}

	// ----------------- Read the graph -----------------

	//Open the file and read the number of vertices and edges
	GraphInputIterator input(argv[1]);
	cout << "Vertex count: " << input.vertexCount() << " Edge count: " << input.edgeCount() << endl;

	vector<Edge> edges;
	
	// Read and save the graph
	for (auto edge : input) {
		//Check that the edge is valid i.e. the nodes are in the graph
		assert(edge.from < input.vertexCount());
		assert(edge.to < input.vertexCount());
		//Check that the edge is not a self loop
		if (edge.to != edge.from) 
			edges.push_back(edge);
	}

	// ----------------- Count the connected components -----------------

	//Start the timer
	auto start = chrono::high_resolution_clock::now();

	//Create a disjoint set with all the vertices 
	DisjointSets<uint32_t> disjoint_set(input.vertexCount());

	// Unify the vertices of each edge
	for (auto edge : edges) 
		disjoint_set.unify(edge.from, edge.to);

	//Stop the timer
	auto end = chrono::high_resolution_clock::now();
	//Calculate the duration
	auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

	unordered_map<uint32_t, uint32_t> components;
	for (uint32_t i = 0; i < input.vertexCount(); i++) {
		uint32_t rep = disjoint_set.find(i);
		if (components.find(rep) == components.end()) {
			components.insert(make_pair(rep, 1));
		} else {
			components.at(rep)++;
		}
	}

	// Print the results
	cout << fixed;
	cout << "------------------------------------------------" << endl;
	cout << "File Name: " << argv[1] << endl;
	cout << "Number of vertices: " << input.vertexCount() << endl;
	cout << "Number of edges: " << edges.size() << endl;
	cout << "Number of connected components: " << components.size() << endl;
	cout << "Elapsed time: " << duration.count() << " ms" << endl;
}
