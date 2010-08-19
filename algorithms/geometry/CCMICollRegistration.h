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
#include "TypeDefs.h"
#include "algorithms/protocols/broadcast/async_impl.h"
#include "algorithms/protocols/broadcast/multi_color_impl.h"
#include "algorithms/protocols/broadcast/async_impl.h"
#include "algorithms/protocols/ambcast/impl.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/protocols/barrier/impl.h"
#include "algorithms/protocols/allreduce/sync_impl.h"
#include "algorithms/protocols/allreduce/async_impl.h"
#include "algorithms/protocols/alltoall/impl.h"

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;

  namespace CollRegistration
  {
    template <class T_Geometry,
              class T_NativeInterface1S,
              class T_NativeInterfaceAS,
              class T_Device>
    class CCMIRegistration :
      public CollRegistration<PAMI::CollRegistration::CCMIRegistration<T_Geometry,
                                                                      T_NativeInterface1S, // Onesided NI
                                                                      T_NativeInterfaceAS, // Allsided NI
                                                                      T_Device>,
                              T_Geometry>
      {
      public:
      inline CCMIRegistration(pami_client_t       client,
                              pami_context_t      context,
                              size_t             context_id,
                              size_t             client_id,
                              T_Device          &dev):
        CollRegistration<PAMI::CollRegistration::CCMIRegistration<T_Geometry,
                                                                 T_NativeInterface1S,
                                                                 T_NativeInterfaceAS,
                                                                 T_Device>,
                         T_Geometry> (),
        _client(client),
        _context(context),
        _context_id(context_id),
        _reduce_val(0),
        _dev(dev),
  _msync_ni           (dev, client,context,context_id,client_id),
        _barrier_ni         (dev, client,context,context_id,client_id),
        _binom_broadcast_ni (dev, client,context,context_id,client_id),
        _ring_broadcast_ni  (dev, client,context,context_id,client_id),
  _asrb_binom_bcast_ni  (dev, client,context,context_id,client_id),
  _ascs_binom_bcast_ni  (dev, client,context,context_id,client_id),
  _active_binombcast_ni  (dev, client,context,context_id,client_id),
  _binom_allreduce_ni (dev, client,context,context_id,client_id),
  _alltoall_ni (dev, client, context, context_id, client_id),
        _connmgr(65535),
  _rbconnmgr(), //Doesnt use sysdeps
  _csconnmgr(), //Doesnt use sysdeps
        _msync_reg(&_sconnmgr, &_msync_ni),
        _barrier_reg(NULL,&_barrier_ni, CCMI::Adaptor::Barrier::BinomialBarrier::cb_head),
        _binom_broadcast_reg(&_connmgr, &_binom_broadcast_ni),
  _ring_broadcast_reg(&_connmgr, &_ring_broadcast_ni),
  _asrb_binom_bcast_reg(&_rbconnmgr, &_asrb_binom_bcast_ni),
  _ascs_binom_bcast_reg(&_csconnmgr, &_ascs_binom_bcast_ni),
  _active_binombcast_reg(&_rbconnmgr, &_active_binombcast_ni),
  _alltoall_reg(&_csconnmgr, &_alltoall_ni),
  _binomial_allreduce_reg(&_rbconnmgr, &_binom_allreduce_ni, CCMI::Adaptor::Allreduce::Binomial::Composite::cb_receiveHead)
          {
            TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
      //set the mapid functions
            _barrier_reg.setMapIdToGeometry(mapidtogeometry);
      _asrb_binom_bcast_reg.setMapIdToGeometry(mapidtogeometry);
      _ascs_binom_bcast_reg.setMapIdToGeometry(mapidtogeometry);
      _active_binombcast_reg.setMapIdToGeometry(mapidtogeometry);
      _alltoall_reg.setMapIdToGeometry(mapidtogeometry);

            _binomial_allreduce_reg.setMapIdToGeometry(mapidtogeometry);
          }

        inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
        {
          TRACE_ERR((stderr, "<%p>%s context_id %zu, geometry %p\n", this, __PRETTY_FUNCTION__, context_id, geometry));
    if (phase != 0) return PAMI_SUCCESS;
          pami_xfer_t xfer = {0};
          _barrier_composite =_barrier_reg.generate(geometry,
                                                    &xfer);

          geometry->setKey(context_id,
                           PAMI::Geometry::PAMI_CKEY_BARRIERCOMPOSITE1,
                           (void*)_barrier_composite);
          // Set geometry-wide, across contexts, UE barrier \todo multi-context support
          geometry->setKey(PAMI::Geometry::PAMI_GKEY_UEBARRIERCOMPOSITE1,
                           (void*)_barrier_composite);


          // Add Barriers
          geometry->addCollective(PAMI_XFER_BARRIER,&_msync_reg,_context_id);
          geometry->addCollective(PAMI_XFER_BARRIER,&_barrier_reg,_context_id);

          // Add Broadcasts
          geometry->addCollective(PAMI_XFER_BROADCAST,&_binom_broadcast_reg, _context_id);
          geometry->addCollective(PAMI_XFER_BROADCAST,&_ring_broadcast_reg,  _context_id);
          geometry->addCollective(PAMI_XFER_BROADCAST,&_asrb_binom_bcast_reg,_context_id);
          geometry->addCollective(PAMI_XFER_BROADCAST,&_ascs_binom_bcast_reg,_context_id);

    // Add allreduce
    geometry->addCollective(PAMI_XFER_ALLREDUCE,&_binomial_allreduce_reg,_context_id);

    //AM Broadcast
    geometry->addCollective(PAMI_XFER_AMBROADCAST,&_active_binombcast_reg,_context_id);

    geometry->setUEBarrier(&_barrier_reg);

          return PAMI_SUCCESS;
        }

        inline pami_result_t analyze_local_impl(size_t context_id,T_Geometry *geometry, uint64_t *out)
          {
            *out = _reduce_val;
            return analyze(context_id, geometry, 0);
          }

        inline pami_result_t analyze_global_impl(size_t context_id,T_Geometry *geometry, uint64_t in)
          {
            return PAMI_SUCCESS;
          }

      static pami_geometry_t mapidtogeometry (int comm)
        {
          pami_geometry_t g = geometry_map[comm];
          TRACE_ERR((stderr, "<%p>%s\n", g, __PRETTY_FUNCTION__));
          return g;
        }

    public:
      pami_client_t                                          _client;
      pami_context_t                                         _context;
      size_t                                                 _context_id;
      uint64_t                                               _reduce_val;

      // Barrier Storage
      CCMI::Executor::Composite                             *_barrier_composite;

      // Native Interface
      T_Device                                              &_dev;
      T_NativeInterface1S                                    _msync_ni;
      T_NativeInterface1S                                    _barrier_ni;
      T_NativeInterfaceAS                                    _binom_broadcast_ni;
      T_NativeInterfaceAS                                    _ring_broadcast_ni;
      T_NativeInterface1S                                    _asrb_binom_bcast_ni;
      T_NativeInterface1S                                    _ascs_binom_bcast_ni;
      T_NativeInterface1S                                    _active_binombcast_ni;
      T_NativeInterface1S                                    _binom_allreduce_ni;
      T_NativeInterface1S                                    _alltoall_ni;

      // CCMI Connection Manager Class
      CCMI::ConnectionManager::ColorGeometryConnMgr          _connmgr;
      CCMI::ConnectionManager::SimpleConnMgr                 _sconnmgr;
      CCMI::ConnectionManager::RankBasedConnMgr              _rbconnmgr;
      CCMI::ConnectionManager::CommSeqConnMgr                _csconnmgr;

      // CCMI Barrier Interface
      CCMI::Adaptor::Barrier::MultiSyncFactory               _msync_reg;
      CCMI::Adaptor::Barrier::BinomialBarrierFactory         _barrier_reg;

      // CCMI Binomial and Ring Broadcast
      CCMI::Adaptor::Broadcast::BinomialBcastFactory         _binom_broadcast_reg;
      CCMI::Adaptor::Broadcast::RingBcastFactory             _ring_broadcast_reg;
      CCMI::Adaptor::Broadcast::AsyncRBBinomBcastFactory     _asrb_binom_bcast_reg;
      CCMI::Adaptor::Broadcast::AsyncCSBinomBcastFactory     _ascs_binom_bcast_reg;
      CCMI::Adaptor::AMBroadcast::AMBinomBcastFactory        _active_binombcast_reg;
      CCMI::Adaptor::Alltoall::All2AllFactory                _alltoall_reg;
      CCMI::Adaptor::Allreduce::Binomial::Factory            _binomial_allreduce_reg;

    };
  };
};
#endif
