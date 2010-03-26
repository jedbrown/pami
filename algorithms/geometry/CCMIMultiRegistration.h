/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/CCMIMultiRegistration.h
 * \brief Simple collectives over multi* interface
 */

#ifndef __algorithms_geometry_CCMIMultiRegistration_h__
#define __algorithms_geometry_CCMIMultiRegistration_h__

#include <map>
#include <vector>
#include "algorithms/interfaces/CollRegistrationInterface.h"
#include "SysDep.h"
#include "TypeDefs.h"
#include "algorithms/protocols/broadcast/mcast_impl.h"
//#include "algorithms/protocols/broadcast/multi_color_impl.h"
//#include "algorithms/protocols/broadcast/async_impl.h"
//#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/protocols/barrier/msync_impl.h"
//#include "algorithms/protocols/allreduce/sync_impl.h"
//#include "algorithms/protocols/allreduce/async_impl.h"
//#include "algorithms/protocols/alltoall/impl.h"

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;

  namespace CollRegistration
  {
    template <class T_Geometry,
    class T_NativeInterfaceAS>
    class CCMIMultiRegistration :
    public CollRegistration<PAMI::CollRegistration::CCMIMultiRegistration<T_Geometry,
                                                                          T_NativeInterfaceAS>,
                            T_Geometry>
    {
    public:
      inline CCMIMultiRegistration(T_NativeInterfaceAS &ni,
                                   pami_client_t       client,
                                   pami_context_t      context,
                                   size_t             context_id,
                                   size_t             client_id):
      CollRegistration<PAMI::CollRegistration::CCMIMultiRegistration<T_Geometry,
      T_NativeInterfaceAS>,
      T_Geometry> (),
      _client(client),
      _context(context),
      _context_id(context_id),
      _ni(ni),
      _sconnmgr(65535),
      _msync_reg(&_sconnmgr, &_ni),
      _mcast_reg(&_sconnmgr, &_ni)
      {
        TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
        //set the mapid functions
        _msync_reg.setMapIdToGeometry(mapidtogeometry);
      }

      inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry)
      {
        TRACE_ERR((stderr, "<%p>%s context_id %zu, geometry %p\n", this, __PRETTY_FUNCTION__, context_id, geometry));
        pami_xfer_t xfer = {0};
        _barrier_composite =_msync_reg.generate(geometry, &xfer);

        geometry->setKey(PAMI::Geometry::PAMI_GKEY_BARRIERCOMPOSITE1,
                         (void*)_barrier_composite);

        // Add Barriers
        geometry->addCollective(PAMI_XFER_BARRIER,&_msync_reg,_context_id);

        // Add Broadcasts
        geometry->addCollective(PAMI_XFER_BROADCAST,&_mcast_reg,_context_id);

        return PAMI_SUCCESS;
      }

      static pami_geometry_t mapidtogeometry (int comm)
      {
        pami_geometry_t g = geometry_map[comm];
        TRACE_ERR((stderr, "<%p>%s\n", g, __PRETTY_FUNCTION__));
        return g;
      }

    public:
      pami_client_t                                           _client;
      pami_context_t                                          _context;
      size_t                                                  _context_id;

      // Barrier Storage
      CCMI::Executor::Composite                             *_barrier_composite;

      // Native Interface
      T_NativeInterfaceAS                                    &_ni;

      // CCMI Connection Manager Class
      CCMI::ConnectionManager::SimpleConnMgr<SysDep>         _sconnmgr;

      // CCMI Barrier Interface
      CCMI::Adaptor::Barrier::MultiSyncFactory               _msync_reg;

      // CCMI Broadcast Interface
      CCMI::Adaptor::Broadcast::MultiCastFactory             _mcast_reg;
    };
  };
};
#endif
