#pragma once

//Standard libraries
#include <iostream>
#include <cstdint>
#include <cstddef>

using namespace std;

struct Edge
{
    uint32_t from;
    uint32_t to;

    inline bool operator<(Edge const &other) const
    {
        return (from < other.from) || ((from == other.from) && (to < other.to));
    }

    inline bool normalized() const
    {
        return from <= to;
    }

    inline void normalize()
    {
        if (!normalized())
        {
            swap(from, to);
        }
    }

    inline bool operator==(Edge const &other) const
    {
        return (from == other.from) && (to == other.to);
    }

    inline bool operator!=(Edge const &other) const
    {
        return !this->operator==(other);
    }

    friend ostream &operator<<(ostream &out, Edge const &edge)
    {
        out << edge.from << " -- " << edge.to;
        return out;
    }
};

static_assert(sizeof(Edge) == 8, "Expecting 4B for a uint32_t");