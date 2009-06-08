
#ifndef __mpi_mapping_impl_h__
#define __mpi_mapping_impl_h__

#ifndef OMPI_SKIP_MPICXX
#define OMPI_SKIP_MPICXX
#endif

#include "mpi.h"
#include "collectives/interface/Mapping.h"

inline CCMI::Mapping::Mapping (void *pers)
{
  _personality = pers;

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  _rank = rank;
  _size = size;
}

#endif
