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
  _size = __pgasrt_tsp_numnodes();
  _rank = __pgasrt_tsp_myID();
}
