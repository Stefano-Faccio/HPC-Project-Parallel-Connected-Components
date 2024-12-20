#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "../PPoPP_2018/utils/Edge.hpp"
#include "../PPoPP_2018/utils/GraphInputIterator.hpp"

using namespace std;

int main(int argc, char* argv[]) {

    if (argc != 2 ) {
		cout << "Usage: connectivity INPUT_FILE" << endl;
		return 1;
	}

	//Open the file and read the number of vertices and edges
	GraphInputIterator input(argv[1]);
	cout << "Vertex count: " << input.vertexCount() << " Edge count: " << input.edgeCount() << endl;

	uint32_t real_edge_count = 0;
	for (auto edge : input) {
		//Check that the edge is valid i.e. the nodes are in the graph
		assert(edge.from < input.vertexCount());
		assert(edge.to < input.vertexCount());
		//Check that the edge is not a self loop
		if (edge.to != edge.from) {
			real_edge_count++;
		}
	}


    return 0;
}