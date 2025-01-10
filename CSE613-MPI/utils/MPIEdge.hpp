#pragma once

//Project headers
#include "Edge.hpp"
//MPI header
#include <mpi.h>
//Standard libraries
#include <iostream>
#include <cstdint>
#include <cstddef>

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

    /*
    static MPI_Op MPI_FILTER_EDGE_OP;
    // Create the MPI_Op for the filter function
    MPI_Op_create(filter_edges, 1, &MPI_FILTER_EDGE_OP);

    static void filter_edges(void *invec, void *inoutvec, int *len, MPI_Datatype *datatype)
    {
        Edge *invecdble = static_cast<Edge *>(invec);
        Edge *inoutvecdble = static_cast<Edge *>(inoutvec);

        // Check if all the edges are the same
        for (int i = 0; i < *len; i++)
        {
            if (invecdble[i].from != inoutvecdble[i].from || invecdble[i].to != inoutvecdble[i].to)
            {
                inoutvecdble[i].from = -1;
                inoutvecdble[i].to = -1;
            }
        }


    }
    */

};