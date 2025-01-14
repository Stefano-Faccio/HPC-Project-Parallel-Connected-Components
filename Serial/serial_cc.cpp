#include "utils/GraphInputIterator.hpp"
#include "utils/DisjointSets.hpp"
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <chrono>
#include <cstdint>
#include <numeric>

using namespace std;

uint32_t calc_serial_cc(uint32_t nNodes, vector<Edge> edges);

int iterations = 100;

int main(int argc, char* argv[])
{
	if (argc < 2 ) {
		cout << "Usage: connectivity INPUT_FILE [ITERATIONS]" << endl;
		return 1;
	}

	if (argc > 2) {
		iterations = atoi(argv[2]);		
	}
	cout << "Iterations: " << iterations << endl;

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

	// ----------------- Calculate the connected components -----------------

	vector<double> times(iterations, 0);
	for(int i = 0; i < iterations; i++)
	{
		double time = (double)calc_serial_cc(input.vertexCount(), edges) / 1000.0;
		times[i] = time;
		cout << time << ", ";
	}
		

	double sum = accumulate(times.begin(), times.end(), 0.0);
	double mean_times = sum / (double)iterations;

	// Print the results
	cout << fixed;
	cout << "------------------------------------------------" << endl;
	cout << "File Name: " << argv[1] << endl;
	cout << "Number of vertices: " << input.vertexCount() << endl;
	cout << "Number of edges: " << edges.size() << endl;
	cout << "Mean time: " << mean_times << " s" << endl;
	/*
	cout << "Number of connected components: " << components.size() << endl;
	cout << "Elapsed time: " << duration_s.count() << " s" << endl;
	cout << "Elapsed time: " << duration_ms.count() << " ms" << endl;
	*/
}

uint32_t calc_serial_cc(uint32_t nNodes, vector<Edge> edges)
{
	//Start the timer
	auto start = chrono::high_resolution_clock::now();

	//Create a disjoint set with all the vertices 
	DisjointSets<uint32_t> disjoint_set(nNodes);

	// Unify the vertices of each edge
	for (auto edge : edges) 
		disjoint_set.unify(edge.from, edge.to);

	//Stop the timer
	auto end = chrono::high_resolution_clock::now();
	//Calculate the duration
	auto duration_s = chrono::duration_cast<chrono::seconds>(end - start);
	auto duration_ms = chrono::duration_cast<chrono::milliseconds>(end - start);

	unordered_map<uint32_t, uint32_t> components;
	for (uint32_t i = 0; i < nNodes; i++) {
		uint32_t rep = disjoint_set.find(i);
		if (components.find(rep) == components.end()) {
			components.insert(make_pair(rep, 1));
		} else {
			components.at(rep)++;
		}
	}

	return duration_ms.count();
}