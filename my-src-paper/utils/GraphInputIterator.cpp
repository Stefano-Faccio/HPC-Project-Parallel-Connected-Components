#include "GraphInputIterator.hpp"

using namespace std;

void GraphInputIterator::open()
{
	read_ = 0;
	file_.open(name_, ios::in);
	// Skip comments
	file_.ignore(numeric_limits<streamsize>::max(), '\n');
	file_ >> vertices_;
	file_ >> lines_;
}

GraphInputIterator::Iterator GraphInputIterator::begin()
{
	return Iterator(false, *this, read());
}

GraphInputIterator::Iterator GraphInputIterator::end()
{
	return Iterator(true, *this, { 0, 0 });
}

void GraphInputIterator::reopen()
{
	file_.close();
	open();
}

Edge GraphInputIterator::read()
{
	u_int32_t from, to;
	file_ >> from >> to;
	read_++;
	return {from, to};
}
