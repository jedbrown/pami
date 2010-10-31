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
#include "TypeDefs.h"
#include "algorithms/protocols/broadcast/mcast_impl.h"
#include "algorithms/protocols/barrier/msync_impl.h"
#include "algorithms/protocols/allreduce/mcomb_impl.h"

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

namespace PAMI
{
  namespace CollRegistration
  {
    template <class T_Geometry, class T_NativeInterfaceAS>
    class CCMIMultiRegistration :
    public CollRegistration<PAMI::CollRegistration::CCMIMultiRegistration<T_Geometry,T_NativeInterfaceAS>,T_Geometry>
    {
    public:
      inline CCMIMultiRegistration(T_NativeInterfaceAS                 &ni,
                                   pami_client_t                        client,
                                   pami_context_t                       context,
                                   size_t                               context_id,
                                   size_t                               client_id,
                                   std::map<unsigned, pami_geometry_t> *geometry_map):
      CollRegistration<PAMI::CollRegistration::CCMIMultiRegistration<T_Geometry, T_NativeInterfaceAS>,T_Geometry> (),
      _client(client),
      _context(context),
      _context_id(context_id),
      _geometry_map(geometry_map),
      _ni(ni),
      _sconnmgr(65535),
      _msync_reg(&_sconnmgr, &_ni),
      _mcast_reg(&_sconnmgr, &_ni),
      _mcomb_reg(&_sconnmgr, &_ni)
      {
        TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
        //set the mapid functions
        _msync_reg.setMapIdToGeometry(mapidtogeometry);
      }

      inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
      {
        TRACE_ERR((stderr, "<%p>%s context_id %zu, geometry %p, msync %p, mcast %p, mcomb %p\n", this, __PRETTY_FUNCTION__, context_id, geometry,&_msync_reg, &_mcast_reg, &_mcomb_reg));
        if (phase != 0) return PAMI_SUCCESS;
        pami_xfer_t xfer = {0};
        _barrier_composite =_msync_reg.generate(geometry, &xfer);

        geometry->setKey(context_id,
                         PAMI::Geometry::PAMI_CKEY_BARRIERCOMPOSITE1,
                         (void*)_barrier_composite);
        // Set geometry-wide, across contexts, UE barrier \todo multi-context support
        geometry->setKey(PAMI::Geometry::PAMI_GKEY_UEBARRIERCOMPOSITE1,
                         (void*)_barrier_composite);

        // Add Barriers
        geometry->addCollective(PAMI_XFER_BARRIER,&_msync_reg,_context_id);

        // Add Broadcasts
        geometry->addCollective(PAMI_XFER_BROADCAST,&_mcast_reg,_context_id);

        // Add Allreduces
        geometry->addCollective(PAMI_XFER_ALLREDUCE,&_mcomb_reg,_context_id);

        return PAMI_SUCCESS;
      }

    public:
      pami_client_t                                           _client;
      pami_context_t                                          _context;
      size_t                                                  _context_id;
      std::map<unsigned, pami_geometry_t>                    *_geometry_map;

      // Barrier Storage
      CCMI::Executor::Composite                             *_barrier_composite;

      // Native Interface
      T_NativeInterfaceAS                                    &_ni;

      // CCMI Connection Manager Class
      CCMI::ConnectionManager::SimpleConnMgr                  _sconnmgr;

      // CCMI Barrier Interface
      CCMI::Adaptor::Barrier::MultiSyncFactory               _msync_reg;

      // CCMI Broadcast Interface
      CCMI::Adaptor::Broadcast::MultiCastFactory             _mcast_reg;

      // CCMI Allreduce Interface
      CCMI::Adaptor::Allreduce::MultiCombineFactory          _mcomb_reg;
    };

    template <class T_Geometry,class T_McastFactory,class T_NativeInterfaceAS>
    class CCMIMultiCastRegistration :  public CollRegistration<PAMI::CollRegistration::CCMIMultiCastRegistration<T_Geometry, T_McastFactory, T_NativeInterfaceAS>,
                                                               T_Geometry>
    {
    public:
      inline CCMIMultiCastRegistration(T_NativeInterfaceAS &ni,
                                   pami_client_t       client,
                                   pami_context_t      context,
                                   size_t             context_id,
                                   size_t             client_id):
      CollRegistration<PAMI::CollRegistration::CCMIMultiCastRegistration<T_Geometry,T_McastFactory,T_NativeInterfaceAS>,T_Geometry> (),
      _client(client),
      _context(context),
      _context_id(context_id),
      _ni(ni),
      _sconnmgr(65535),
      _mcast_reg(&_sconnmgr, &_ni)
      {
        TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
        DO_DEBUG((templateName<T_Geometry>()));
        DO_DEBUG((templateName<T_McastFactory>()));
        DO_DEBUG((templateName<T_NativeInterfaceAS>()));
      }

      inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
      {
        TRACE_ERR((stderr, "<%p>%s context_id %zu, geometry %p, mcast %p\n", this, __PRETTY_FUNCTION__, context_id, geometry,&_mcast_reg));
  if (phase != 0) return PAMI_SUCCESS;

        // Add Broadcasts
        geometry->addCollective(PAMI_XFER_BROADCAST,&_mcast_reg,_context_id);

        return PAMI_SUCCESS;
      }

    public:
      pami_client_t                                           _client;
      pami_context_t                                          _context;
      size_t                                                  _context_id;
      // Native Interface
      T_NativeInterfaceAS                                    &_ni;

      // CCMI Connection Manager Class
      CCMI::ConnectionManager::SimpleConnMgr                  _sconnmgr;

      // CCMI Broadcast Interface
      T_McastFactory                                         _mcast_reg;
    };

  };
};
#endif
