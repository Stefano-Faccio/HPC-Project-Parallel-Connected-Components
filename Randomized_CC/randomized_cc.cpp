//OpenMP header
#include <omp.h>
//Standard libraries
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <random>
//Custom libraries
#include "../PPoPP_2018/utils/Edge.hpp"
#include "../PPoPP_2018/utils/GraphInputIterator.hpp"

using namespace std;

// Mersenne Twister 19937 generator
unsigned seed = 17;
mt19937 random_engine(seed);

vector<uint32_t> par_randomized_cc(uint32_t n, vector<Edge>& edges, vector<uint32_t>& labels) {
	if(edges.size() == 0 || n == 0) 
		return labels;
	
	vector<bool> coin_toss(n);
	for (uint32_t i = 0; i < n; i++) {
		coin_toss[i] = random_engine() % 2;
	}

	return labels;
}

int main(int argc, char* argv[]) {

    if (argc != 2 ) {
		cout << "Usage: connectivity INPUT_FILE" << endl;
		return 1;
	}

	//Open the file and read the number of vertices and edges
	GraphInputIterator input(argv[1]);
	cout << "Vertex count: " << input.vertexCount() << " Edge count: " << input.edgeCount() << endl;

	vector<Edge> edges;
	edges.reserve(input.edgeCount());
	uint32_t real_edge_count = 0;
	for (auto edge : input) {
		//Check that the edge is valid i.e. the nodes are in the graph
		assert(edge.from < input.vertexCount());
		assert(edge.to < input.vertexCount());
		//Check that the edge is not a self loop
		if (edge.to != edge.from) {
			edges.push_back(edge);
			real_edge_count++;
		}
	}
	assert(real_edge_count == input.edgeCount() && edges.size() == input.edgeCount());

	vector<uint32_t> labels(input.vertexCount());
	for(uint32_t i = 0; i < input.vertexCount(); i++) {
		labels[i] = i;
	}

	vector<uint32_t> cc = par_randomized_cc(input.vertexCount(), edges, labels);

    return 0;
}