#include "utils/GraphInputIterator.hpp"
#include "utils/DisjointSets.hpp"
//#include "utils.hpp"
#include <unordered_map>
#include <random>
#include <iostream>

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
	DisjointSets<u_int32_t> disjoint_set(input.vertexCount());

	u_int32_t real_edge_count = 0;
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
	unordered_map<u_int32_t, u_int32_t> components;
	for (u_int32_t i = 0; i < input.vertexCount(); i++) {
		u_int32_t rep = disjoint_set.find(i);
		if (components.find(rep) == components.end()) {
			components.insert(make_pair(rep, 1));
		} else {
			components.at(rep)++;
		}
	}

	cout << "Connected components: " << components.size() << endl;


	/*

	unordered_map<unsigned, unsigned> components;

	for (unsigned i = 0; i < input.vertexCount(); i++) {
		unsigned rep = disjoint_set.find(i);
		if (components.find(rep) == components.end()) {
			components.insert(make_pair(rep, 1));
		} else {
			components.at(rep)++;
		}
	}

	if (argc == 3) {
		cout << components.size() << endl;
		return 0;
	}

	auto max_component = components.begin();

	for (auto component = components.begin(); component != components.end(); component++) {
		if (component->second > max_component->second) {
			max_component = component;
		}
	}


	vector<unsigned> max_component_members;
	for (unsigned i = 0; i < input.vertexCount(); i++) {
		if (disjoint_set.find(i) == max_component->first) {
			max_component_members.push_back(i);
		}
	}


	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> uni(0, max_component_members.size());

	cout << "# Graph connector tool" << endl;
	cout << input.vertexCount() << " " << real_edge_count + (input.vertexCount() - max_component->second) << endl;
	for (unsigned i = 0; i < input.vertexCount(); i++) {
		if (disjoint_set.find(i) != max_component->first) {
			// Suggest a random edges
			cout << i << " " << max_component_members.at(uni(rng)) << " 5000" << endl;
		}
	}

	input.reopen();
	for (auto edge : input) {
		if (edge.to != edge.from) {
			cout << edge.from << " " << edge.to << " " << edge.weight << endl;
		}
	}
	*/
}
