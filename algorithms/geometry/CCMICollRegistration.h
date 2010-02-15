/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/CCMICollRegistration.h
 * \brief ???
 */

#ifndef __algorithms_geometry_CCMICollRegistration_h__
#define __algorithms_geometry_CCMICollRegistration_h__

#include <map>
#include <vector>
#include "algorithms/interfaces/CollRegistrationInterface.h"
#include "SysDep.h"
#include "TypeDefs.h"
#include "algorithms/protocols/broadcast/async_impl.h"
#include "algorithms/protocols/broadcast/multi_color_impl.h"
#include "algorithms/protocols/broadcast/async_impl.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/protocols/barrier/impl.h"
#include "algorithms/protocols/allreduce/sync_impl.h"
#include "algorithms/protocols/allreduce/async_impl.h"
#include "algorithms/protocols/alltoall/impl.h"

namespace XMI
{
  extern std::map<unsigned, xmi_geometry_t> geometry_map;

  namespace CollRegistration
  {
    template <class T_Geometry,
              class T_NativeInterface,
              class T_Device>
    class CCMIRegistration :
      public CollRegistration<XMI::CollRegistration::CCMIRegistration<T_Geometry,
                                                                      T_NativeInterface,
                                                                      T_Device>,
                              T_Geometry>
      {
      public:
      inline CCMIRegistration(xmi_client_t       client,
                              xmi_context_t      context,
                              size_t             context_id,
                              size_t             client_id,
                              T_Device          &dev):
        CollRegistration<XMI::CollRegistration::CCMIRegistration<T_Geometry,
                                                                 T_NativeInterface,
                                                                 T_Device>,
                         T_Geometry> (),
        _client(client),
        _context(context),
        _context_id(context_id),
        _dev(dev),
        _minterface(dev, client,context,context_id,client_id),
        _connmgr(65535),
        _msync_reg(&_sconnmgr, &_minterface),
	_barrier_reg(NULL,&_minterface, (xmi_dispatch_multicast_fn)CCMI::Adaptor::Barrier::BinomialBarrier::cb_head),
        _binom_broadcast_reg(&_connmgr, &_minterface),
        _ring_broadcast_reg(&_connmgr, &_minterface)
          {

          }

        inline xmi_result_t analyze_impl(size_t context_id, T_Geometry *geometry)
        {
          // Add Barriers
          geometry->addCollective(XMI_XFER_BARRIER,&_msync_reg,_context_id);
          geometry->addCollective(XMI_XFER_BARRIER,&_barrier_reg,_context_id);

          // Add Broadcasts
          geometry->addCollective(XMI_XFER_BROADCAST,&_binom_broadcast_reg,_context_id);
          geometry->addCollective(XMI_XFER_BROADCAST,&_ring_broadcast_reg,_context_id);
          return XMI_SUCCESS;
        }

      static xmi_geometry_t mapidtogeometry (int comm)
        {
          xmi_geometry_t g = geometry_map[comm];
          return g;
        }

    public:
      xmi_client_t                                           _client;
      xmi_context_t                                          _context;
      size_t                                                 _context_id;

      // Native Interface
      T_Device                                              &_dev;
      T_NativeInterface                                      _minterface;

      // CCMI Connection Manager Class
      CCMI::ConnectionManager::ColorGeometryConnMgr<SysDep>  _connmgr;
      CCMI::ConnectionManager::SimpleConnMgr<SysDep>         _sconnmgr;

      // CCMI Barrier Interface
      CCMI::Adaptor::Barrier::MultiSyncFactory               _msync_reg;
      CCMI::Adaptor::Barrier::BinomialBarrierFactory         _barrier_reg;

      // CCMI Binomial and Ring Broadcast
      CCMI::Adaptor::Broadcast::BinomialBcastFactory         _binom_broadcast_reg;
      CCMI::Adaptor::Broadcast::RingBcastFactory             _ring_broadcast_reg;
    };
  };
};
#endif
