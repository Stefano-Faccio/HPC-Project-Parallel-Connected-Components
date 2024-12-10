#pragma once

#include <iostream>
#include <tuple>
#include <mpi.h>

using namespace std;

struct Edge
{
    u_int32_t from, to;

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

static_assert(sizeof(Edge) == 8, "Expecting 4B for a u_int32_t");

struct MPIEdge
{
    static MPI_Datatype edge_type;
    static bool initialized;

    static MPI_Datatype constructType()
    {
        if (!initialized)
        {
            int blocklengths[2] = {1, 1};

            // This leaks abstraction :(
            MPI_Datatype types[2] = {MPI_UINT32_T, MPI_UINT32_T};

            MPI_Aint offsets[2];

            offsets[0] = offsetof(Edge, from);
            offsets[1] = offsetof(Edge, to);

            MPI_Type_create_struct(2, blocklengths, offsets, types, &edge_type);
            MPI_Type_commit(&edge_type);

            initialized = true;
        }
        return edge_type;
    };
};