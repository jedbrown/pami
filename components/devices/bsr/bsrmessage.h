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
      BSRGeometryInfo(int          geometry_id,
                      Topology    *topology,
                      void        *shm_block,
                      size_t       shm_block_sz,
                      unsigned int job_key,
                      unsigned int member_id):
        _geometry_id(geometry_id),
        _topology(topology),
        _in_barrier(false),
        _shm_block(shm_block),
        _shm_block_sz(shm_block_sz),
        _sync_group(member_id, topology->size(), job_key, shm_block, shm_block_sz)
        {
        }
      SaOnNodeSyncGroup         _sync_group;
      int                       _geometry_id;
      Topology                 *_topology;
      bool                      _in_barrier;
      std::vector <void*>       _waiters_q;
      void                     *_shm_block;    // shared memory block passed
                                               // from collective framework
      size_t                    _shm_block_sz; // size of the shared memory buffer
    };
  };
};

#endif // __components_devices_bsr_bsrbasemessage_h__
