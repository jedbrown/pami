/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file adaptor/generic/mapping_impl.h
 * \brief ???
 */

#include "mapping_impl.h"

CCMI::CollectiveMapping::CollectiveMapping (void *pers)
{
  _personality = pers;

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  _rank = rank;
  _size = size;
}
