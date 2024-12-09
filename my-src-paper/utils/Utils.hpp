#pragma once

#include <iostream>
#include <tuple>

using namespace std;

struct Edge {
    u_int32_t from, to;

    inline bool operator<(Edge const & other) const {
        return (from < other.from) || ((from == other.from) && (to < other.to));
    }

    inline bool normalized() const {
        return from <= to;
    }

    inline void normalize() {
        if (! normalized()) {
            swap(from, to);
        }
    }

    inline bool operator==(Edge const & other) const {
        return (from == other.from) && (to == other.to);
    }

    inline bool operator!=(Edge const & other) const {
        return ! this->operator==(other);
    }

    friend ostream & operator<< (ostream & out, Edge const & edge) {
        out << edge.from << " -- "  << edge.to;
        return out;
    }
};

static_assert(sizeof(Edge) == 8, "Expecting 4B for a u_int32_t");