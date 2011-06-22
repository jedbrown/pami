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
                      Topology *topology):
        _geometry_id(geometry_id),
        _topology(topology),
        _in_barrier(false)
        {
            GenerateUniqueKey();
        }

      void GenerateUniqueKey() {
          srand(_geometry_id);
          _shm_unique_key = 0;
          switch (_topology->type()) {
              case PAMI_RANGE_TOPOLOGY:
                  {
                      pami_task_t rfirst;
                      pami_task_t rlast;
                      _topology->rankRange(&rfirst, &rlast);
                      for (uint64_t i = (size_t)rfirst; i <= (uint64_t)rlast; i ++) {
                          _shm_unique_key += (rand()*i);
                      }
                      break;
                  }
              default:
                  {
                      pami_task_t *rlist;
                      _topology->rankList(&rlist);
                      for (int i = 0; i < _topology->size(); i ++) {
                          _shm_unique_key += (rand()*(uint64_t)rlist[i]);
                      }
                      break;
                  }
          }
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
