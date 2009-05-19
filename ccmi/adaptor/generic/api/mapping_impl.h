
#ifndef __mpi_mapping_impl_h__
#define __mpi_mapping_impl_h__


#include "mpi.h"
#include "adaptor/Mapping.h"

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
