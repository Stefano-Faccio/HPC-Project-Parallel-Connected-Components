#pragma once

#include <boost/pending/disjoint_sets.hpp>
#include <vector>
#include <iostream>

using namespace std;
using namespace boost;

template <class ElementT>
class DisjointSets {
	vector<unsigned> ranks; // Init to all 0
	vector<ElementT> parents; // Init to 0, 1, 2, 3, ...
	disjoint_sets<unsigned *, ElementT *> dsets;

	vector<ElementT> generateParents(size_t element_count) const
	{
		vector<ElementT> elements(element_count);
		ElementT n = {0};
		generate(elements.begin(), elements.end(), [&n] { return n++; });
		return elements; // NRVO: Named Return Value Optimization (Compiler optimization)
	}

public:
	DisjointSets(vector<ElementT> const & elements) :
			ranks(elements.size(), 0),
			parents(elements), // Every elements is its own parent initially
			dsets(&ranks.at(0), &parents.at(0))
	{ }

	DisjointSets(size_t element_count) : DisjointSets(generateParents(element_count))
	{ }

	DisjointSets(const DisjointSets& that) = delete;

	ElementT find(ElementT elem) { return dsets.find_set(elem); }

	void unify(ElementT a, ElementT b) { dsets.link(a, b); }

	void print_parents() const
	{
		for (auto parent : parents) {
			cout << parent << " ";
		}
		cout << endl;
	}

	void print_ranks() const
	{
		for (auto rank : ranks) {
			cout << rank << " ";
		}
		cout << endl;
	}
};
