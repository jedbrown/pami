/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
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
        _sync_group(member_id, topology->size(), job_key, shm_block, shm_block_sz),
        _geometry_id(geometry_id),
        _topology(topology),
        _in_barrier(false)
        {
        }

      bool Checkpoint()
        {
          return _sync_group.Checkpoint();
        }

      bool Restart()
        {
          return _sync_group.Restart();
        }

      bool Resume()
        {
          return _sync_group.Resume();
        }

      SaOnNodeSyncGroup         _sync_group;
      int                       _geometry_id;
      Topology                 *_topology;
      bool                      _in_barrier;
      std::vector <void*>       _waiters_q;
    };
  };
};

#endif // __components_devices_bsr_bsrbasemessage_h__
