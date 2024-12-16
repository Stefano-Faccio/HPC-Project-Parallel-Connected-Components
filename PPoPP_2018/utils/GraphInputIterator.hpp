#pragma once

//Project headers
#include "Edge.hpp"
//Standard libraries
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <cassert>
#include <vector>
#include <cstdint>


using namespace std;

class GraphInputIterator
{
private:
	ifstream file_;
	uint32_t lines_, read_;
	uint32_t vertices_;
	string name_;

	// Open the file
	void open()
	{
		read_ = 0;
		file_.open(name_, ios::in);
		file_ >> vertices_;
		file_ >> lines_;
	}

	// Read the next edge from the file
	Edge read()
	{
		uint32_t from, to;
		file_ >> from >> to;
		read_++;
		return {from, to};
	}

public:
	GraphInputIterator(string name) : name_(name)
	{
		file_.exceptions(ifstream::failbit | ifstream::badbit);
		open();
	}

	~GraphInputIterator() { file_.close(); }

	uint32_t vertexCount() { return vertices_; }
	uint32_t edgeCount() { return lines_; }

	// Reset the file stream in order to read the input again
	void reopen()
	{
		file_.close();
		open();
	}

	void loadSlice(vector<Edge> &edges_slice, int32_t rank, int32_t group_size)
	{
		uint32_t slice_portion = edgeCount() / group_size;
		uint32_t slice_from = slice_portion * rank;
		// The last node takes any leftover edges
		bool last = rank == group_size - 1;
		uint32_t slice_to = last ? edgeCount() : slice_portion * (rank + 1);

		GraphInputIterator::Iterator iterator = begin();
		while (!iterator.end_)
		{
			if (iterator.position() >= slice_from && iterator.position() < slice_to)
			{
				edges_slice.push_back(*iterator);
			}
			++iterator;
		}

		assert(edges_slice.size() == slice_to - slice_from);
	}

	// Model of http://en.cppreference.com/w/cpp/concept/InputIterator concept
	class Iterator
	{
		uint32_t position_ = 0;

	public:
		bool end_;
		GraphInputIterator &parent_;
		Edge edge_;

		Iterator(bool end, GraphInputIterator &parent, Edge edge) : end_(end), parent_(parent), edge_(edge) {}

		Edge operator*()
		{
			return edge_;
		}

		Edge *operator->()
		{
			return &edge_;
		}

		uint32_t position() const
		{
			return position_;
		}

		void operator++()
		{
			if (parent_.read_ < parent_.lines_)
				edge_ = parent_.read();
			else
				end_ = true;
			position_++;
		}

		bool operator!=(Iterator &other)
		{
			return other.end_ != end_;
		}
	};

	Iterator begin()
	{
		return Iterator(false, *this, read());
	}

	Iterator end()
	{
		return Iterator(true, *this, {0, 0});
	}
};

namespace std
{
	template <>
	struct iterator_traits<GraphInputIterator::Iterator>
	{
		typedef ptrdiff_t difference_type;			  // almost always ptrdif_t
		typedef Edge value_type;					  // almost always T
		typedef Edge &reference;					  // almost always T& or const T&
		typedef Edge *pointer;						  // almost always T* or const T*
		typedef input_iterator_tag iterator_category; // usually forward_iterator_tag or similar
	};
}
