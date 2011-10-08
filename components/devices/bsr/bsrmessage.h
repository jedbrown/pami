/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bsr/bsrmessage.h
 * \brief ???
 */

#ifndef __components_devices_bsr_bsrmessage_h__
#define __components_devices_bsr_bsrmessage_h__

#include <pami.h>
#include "util/common.h"
#include "common/lapiunix/lapifunc.h"
#include "TypeDefs.h"
#include "components/memory/MemoryAllocator.h"
#include "components/devices/bsr/SaOnNodeSyncGroup.h"
#include <vector>

namespace PAMI
{
  namespace Device
  {
    class BSRGeometryInfo
    {
    public:
      BSRGeometryInfo(int       geometry_id,
                      Topology *topology,
                      uint64_t  shm_unique_key,
                      void     *shm_buffer,
                      size_t    shm_size,
                      unsigned  partition_id,
                      unsigned  member_id):
        _geometry_id(geometry_id),
        _topology(topology),
        _in_barrier(false),
        _shm_unique_key(shm_unique_key)
        {
        }
      SaOnNodeSyncGroup         _sync_group;
      int                       _geometry_id;
      Topology                 *_topology;
      bool                      _in_barrier;
      std::vector <void*>       _waiters_q;
      uint64_t                  _shm_unique_key;

    };
  };
};

#endif // __components_devices_bsr_bsrbasemessage_h__
