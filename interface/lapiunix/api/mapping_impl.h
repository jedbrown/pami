
#ifndef __pgasp2p_mapping_impl_h__
#define __pgasp2p_mapping_impl_h__

#include "interface/Mapping.h"
#include "interface/lapiunix/common/include/pgasrt.h"


inline CCMI::Mapping::Mapping (void *pers)
{
  _personality = pers;

  int rank, size;
  _size = __pgasrt_tsp_numnodes();
  _rank = __pgasrt_tsp_myID();
}

#endif
