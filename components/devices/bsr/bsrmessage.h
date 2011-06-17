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
#ifndef _LAPI_LINUX
#include "components/devices/bsr/SaOnNodeSyncGroup.h"
#endif
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

        }
#ifndef _LAPI_LINUX
      SaOnNodeSyncGroup         _sync_group;
#endif
      int                       _geometry_id;
      Topology                 *_topology;
      bool                      _in_barrier;
      std::vector <void*>       _waiters_q;

    };
  };
};

#endif // __components_devices_bsr_bsrbasemessage_h__
