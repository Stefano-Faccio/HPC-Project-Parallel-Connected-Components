#include "MPIDatatype.hpp"

bool MPIDatatype<Edge>::initialized = false;
MPI_Datatype MPIDatatype<Edge>::edge_type;
