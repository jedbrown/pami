/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/BGQCCMIRegistration.h
 * \brief ???
 */

#ifndef __algorithms_geometry_BGQCCMIRegistration_h__
#define __algorithms_geometry_BGQCCMIRegistration_h__

#include <map>
#include <vector>
#include "algorithms/interfaces/CollRegistrationInterface.h"
#include "SysDep.h"
#include "TypeDefs.h"

#include "util/ccmi_debug.h"

//#include "algorithms/protocols/barrier/impl.h"
//#include "algorithms/protocols/allreduce/sync_impl.h"
//#include "algorithms/protocols/allreduce/async_impl.h"
//#include "algorithms/protocols/alltoall/impl.h"

#include "algorithms/schedule/MultinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"
#include "algorithms/protocols/broadcast/BcastMultiColorCompositeT.h"

#include "algorithms/protocols/barrier/BarrierT.h"
#include "algorithms/protocols/broadcast/AsyncBroadcastT.h"

#include "algorithms/protocols/ambcast/AMBroadcastT.h"

// CCMI Template implementations
namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {

      void binomial__barrier_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"Binomial_Barrier");
      }

      bool binomial_analyze (PAMI_GEOMETRY_CLASS *geometry)
      {
        return true;
      }


      typedef BarrierT <CCMI::Schedule::ListMultinomial,
      binomial_analyze> BinomialBarrier;

      typedef BarrierFactoryT <BinomialBarrier,
      binomial__barrier_md,
      ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > BinomialBarrierFactory;
    };//Barrier

    namespace Broadcast
    {
      void get_colors (PAMI::Topology             * t,
                       unsigned                    bytes,
                       unsigned                  * colors,
                       unsigned                  & ncolors)
      {
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        ncolors = 1;
        colors[0] = CCMI::Schedule::NO_COLOR;
      }

      void binomial_broadcast_metadata(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        strcpy(&m->name[0], "Binomial_Broadcast");
      }

      void ring_broadcast_metadata(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        strcpy(&m->name[0], "Ring_Broadcast");
      }

      typedef BcastMultiColorCompositeT < 1,
      CCMI::Schedule::ListMultinomial,
      CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS>,
      get_colors > BinomialBroadcastComposite;

      typedef CollectiveProtocolFactoryT < BinomialBroadcastComposite,
      binomial_broadcast_metadata,
      CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS> > BinomialBroadcastFactory;

      typedef BcastMultiColorCompositeT < 1,
      CCMI::Schedule::RingSchedule,
      CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS>,
      get_colors > RingBroadcastComposite;

      typedef CollectiveProtocolFactoryT < RingBroadcastComposite,
      ring_broadcast_metadata,
      CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS> > RingBroadcastFactory;

      void am_rb_broadcast_metadata(pami_metadata_t *m)
      {
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        // \todo:  fill in other metadata
        strcpy(&m->name[0], "Async_RB_Binomial_Broadcast");
      }

      void am_cs_broadcast_metadata(pami_metadata_t *m)
      {
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        // \todo:  fill in other metadata
        strcpy(&m->name[0], "Async_CS_Binomial_Broadcast");
      }

      typedef AsyncBroadcastT < CCMI::Schedule::ListMultinomial,
      CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AsyncRBBinomialBroadcastComposite;
      template<>
      void AsyncRBBinomialBroadcastComposite::create_schedule(void                        * buf,
                                                              unsigned                      size,
                                                              unsigned                      root,
                                                              Interfaces::NativeInterface * native,
                                                              PAMI_GEOMETRY_CLASS          * g)
      {
        TRACE_ADAPTOR((stderr, "<%p>AsyncRBBinomialBroadcastComposite::create_schedule()\n", this));
        new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
      }

      typedef AsyncBroadcastFactoryT < AsyncRBBinomialBroadcastComposite,
      am_rb_broadcast_metadata,
      CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AsyncRBBinomialBroadcastFactory;

      template<>
      unsigned AsyncRBBinomialBroadcastFactory::getKey(unsigned                                                root,
                                                       unsigned                                                connid,
                                                       PAMI_GEOMETRY_CLASS                                    *geometry,
                                                       ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> **connmgr)
      {
        TRACE_ADAPTOR((stderr, "<%p>AsyncRBBinomialBroadcastFactory::getKey()\n", this));
        return root;
      }

      typedef AsyncBroadcastT < CCMI::Schedule::ListMultinomial,
      CCMI::ConnectionManager::CommSeqConnMgr > AsyncCSBinomialBroadcastComposite;
      template<>
      void AsyncCSBinomialBroadcastComposite::create_schedule(void                        * buf,
                                                              unsigned                      size,
                                                              unsigned                      root,
                                                              Interfaces::NativeInterface * native,
                                                              PAMI_GEOMETRY_CLASS          * g)
      {
        TRACE_ADAPTOR((stderr, "<%p>AsyncCSBinomialBroadcastComposite::create_schedule()\n", this));
        new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
      }

      typedef AsyncBroadcastFactoryT < AsyncCSBinomialBroadcastComposite,
      am_cs_broadcast_metadata,
      CCMI::ConnectionManager::CommSeqConnMgr > AsyncCSBinomialBroadcastFactory;

      template<>
      unsigned AsyncCSBinomialBroadcastFactory::getKey(unsigned                                   root,
                                                       unsigned                                   connid,
                                                       PAMI_GEOMETRY_CLASS                      * geometry,
                                                       ConnectionManager::CommSeqConnMgr        **connmgr)
      {
        TRACE_ADAPTOR((stderr, "<%p>AsyncCSBinomialBroadcastFactory::getKey\n", this));

        if (connid != (unsigned) - 1)
        {
          *connmgr = NULL; //use this key as connection id
          return connid;
        }

        return(*connmgr)->updateConnectionId( geometry->comm() );
      }

    }//Broadcast
    namespace AMBroadcast
    {
      void am_broadcast_metadata(pami_metadata_t *m)
      {
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        // \todo:  fill in other metadata
        strcpy(&m->name[0], "Binomial_AMBroadcast");
      }

      typedef AMBroadcastT < CCMI::Schedule::ListMultinomial,
      CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AMBinomialBroadcastComposite;
      template<>
      void AMBinomialBroadcastComposite::create_schedule(void                        * buf,
                                                         unsigned                      size,
                                                         unsigned                      root,
                                                         Interfaces::NativeInterface * native,
                                                         PAMI_GEOMETRY_CLASS          * g)
      {
        TRACE_ADAPTOR((stderr, "<%p>AMBinomialBroadcastComposite::create_schedule()\n", this));
        new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
      }

      typedef AMBroadcastFactoryT < AMBinomialBroadcastComposite,
      am_broadcast_metadata,
      CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AMBinomialBroadcastFactory;

    }//AMBroadcast
  }//Adaptor
}//CCMI

// Collective Registration for CCMI protocols on BG/Q
namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;

  namespace CollRegistration
  {
    namespace BGQ
    {
      template < class T_Geometry,
      class T_NativeInterfaceP2pActiveMessage, // Onesided/Active message on P2P protocol
      class T_NativeInterfaceP2pAllsided,      // Allsided on P2P protocol
    class T_Local_Device,                    // Local (shmem) device
    class T_Global_Device,                   // Global (MU) device
      class T_Allocator >
      class CCMIRegistration :
      public CollRegistration < PAMI::CollRegistration::BGQ::CCMIRegistration < T_Geometry,
      T_NativeInterfaceP2pActiveMessage, 
      T_NativeInterfaceP2pAllsided,      
        T_Local_Device,
        T_Global_Device,
      T_Allocator > ,
      T_Geometry >
      {
      public:
        inline CCMIRegistration(pami_client_t       client,
                                pami_context_t      context,
                                size_t              context_id,
                                size_t              client_id,
                                T_Local_Device     &ldev,
                                T_Global_Device    &gdev,
                                T_Allocator        &allocator):
        CollRegistration < PAMI::CollRegistration::BGQ::CCMIRegistration < T_Geometry,
        T_NativeInterfaceP2pActiveMessage,
        T_NativeInterfaceP2pAllsided,
            T_Local_Device,
            T_Global_Device,
        T_Allocator > ,
        T_Geometry > (),
        _client(client),
        _context(context),
        _context_id(context_id),
        _client_id(client_id),
        _local_dev(ldev),
        _global_dev(gdev),
        _allocator(allocator),
        _binomial_barrier_composite(NULL),
        _binomial_barrier_ni ((T_NativeInterfaceP2pActiveMessage*)&_binomial_barrier_ni_storage),
        _binomial_barrier_p2p_protocol(NULL),
        _binomial_broadcast_ni ((T_NativeInterfaceP2pAllsided*)&_binomial_broadcast_ni_storage),
        _binomial_broadcast_p2p_protocol(NULL),
        _ring_broadcast_ni  ((T_NativeInterfaceP2pAllsided*)&_ring_broadcast_ni_storage),
        _ring_broadcast_p2p_protocol(NULL),
        _asrb_binomial_broadcast_ni   ((T_NativeInterfaceP2pActiveMessage*)&_asrb_binomial_broadcast_ni_storage),
        _ascs_binomial_broadcast_ni   ((T_NativeInterfaceP2pActiveMessage*)&_ascs_binomial_broadcast_ni_storage),
        _active_binomial_broadcast_ni ((T_NativeInterfaceP2pActiveMessage*)&_active_binomial_broadcast_ni_storage),
//    _binomial_allreduce_ni (gdev, client,context,context_id,client_id),
        _connmgr(65535),
        _rbconnmgr(NULL),
        _csconnmgr(),
        _binomial_barrier_factory(NULL),
        _binomial_broadcast_factory(NULL),
        _ring_broadcast_factory(NULL),
        _asrb_binomial_broadcast_factory(NULL),
        _ascs_binomial_broadcast_factory(NULL),
        _active_binomial_broadcast_factory(NULL),
//    _binomial_allreduce_reg(&_rbconnmgr, &_binomial_allreduce_ni, (pami_dispatch_multicast_fn)CCMI::Adaptor::Allreduce::Binomial::Composite::cb_receiveHead)
        _shmem_p2p_protocol(NULL),
        _shmem_ni(NULL),
        _mu_p2p_protocol(NULL),
        _mu_ni(NULL),
        _composite_ni(NULL)
        {

          TRACE_ADAPTOR((stderr, "<%p>CCMIRegistration()\n", this ));

          if (__global.useMU())
          {
            pami_result_t result = PAMI_ERROR;
            size_t dispatch = -1;

            // MU over P2P eager protocol
            _mu_ni = (MUNI*) new (_mu_ni_storage) MUNI(client, context, context_id, client_id, dispatch);

            pami_dispatch_callback_fn fn;
            fn.p2p = MUNI::dispatch_p2p;
            _mu_p2p_protocol = (MUEager*) MUEager::generate(dispatch, fn, (void*) _mu_ni, gdev, _allocator, result);
            _mu_ni->setProtocol(_mu_p2p_protocol);


            _binomial_barrier_ni = (T_NativeInterfaceP2pActiveMessage*) new (_binomial_barrier_ni_storage) T_NativeInterfaceP2pActiveMessage(client, context, context_id, client_id, dispatch);
            fn.p2p = T_NativeInterfaceP2pActiveMessage::dispatch_p2p;
            _binomial_barrier_p2p_protocol = (MUEager*) MUEager::generate(dispatch, fn, (void*) _binomial_barrier_ni, gdev, _allocator, result);
            _binomial_barrier_ni->setProtocol(_binomial_barrier_p2p_protocol);

            _binomial_barrier_factory = new (_binomial_barrier_factory_storage) CCMI::Adaptor::Barrier::BinomialBarrierFactory(&_sconnmgr, _binomial_barrier_ni,(pami_dispatch_multicast_fn)CCMI::Adaptor::Barrier::BinomialBarrier::cb_head);

            _binomial_broadcast_ni = (T_NativeInterfaceP2pAllsided*) new (_binomial_broadcast_ni_storage) T_NativeInterfaceP2pAllsided(client, context, context_id, client_id, dispatch);
            fn.p2p = T_NativeInterfaceP2pAllsided::dispatch_p2p;
            _binomial_broadcast_p2p_protocol = (MUEager*) MUEager::generate(dispatch, fn, (void*) _binomial_broadcast_ni, gdev, _allocator, result);
            _binomial_broadcast_ni->setProtocol(_binomial_broadcast_p2p_protocol);

            _binomial_broadcast_factory = new (_binomial_broadcast_factory_storage) CCMI::Adaptor::Broadcast::BinomialBroadcastFactory(&_connmgr, _binomial_broadcast_ni);

            _ring_broadcast_ni = (T_NativeInterfaceP2pAllsided*) new (_ring_broadcast_ni_storage) T_NativeInterfaceP2pAllsided(client, context, context_id, client_id, dispatch);
            fn.p2p = T_NativeInterfaceP2pAllsided::dispatch_p2p;
            _ring_broadcast_p2p_protocol = (MUEager*) MUEager::generate(dispatch, fn, (void*) _ring_broadcast_ni, gdev, _allocator, result);
            _ring_broadcast_ni->setProtocol(_ring_broadcast_p2p_protocol);

            _ring_broadcast_factory = new (_ring_broadcast_factory_storage) CCMI::Adaptor::Broadcast::RingBroadcastFactory(&_connmgr, _ring_broadcast_ni);

            _ascs_binomial_broadcast_ni = (T_NativeInterfaceP2pActiveMessage*) new (_ascs_binomial_broadcast_ni_storage) T_NativeInterfaceP2pActiveMessage(client, context, context_id, client_id, dispatch);
            fn.p2p = T_NativeInterfaceP2pActiveMessage::dispatch_p2p;
            _ascs_binomial_broadcast_p2p_protocol = (MUEager*) MUEager::generate(dispatch, fn, (void*) _ascs_binomial_broadcast_ni, gdev, _allocator, result);
            _ascs_binomial_broadcast_ni->setProtocol(_ascs_binomial_broadcast_p2p_protocol);

            _ascs_binomial_broadcast_factory = new (_ascs_binomial_broadcast_factory_storage) CCMI::Adaptor::Broadcast::AsyncCSBinomialBroadcastFactory(&_csconnmgr, _ascs_binomial_broadcast_ni);

            _asrb_binomial_broadcast_ni = (T_NativeInterfaceP2pActiveMessage*) new (_asrb_binomial_broadcast_ni_storage) T_NativeInterfaceP2pActiveMessage(client, context, context_id, client_id, dispatch);
            fn.p2p = T_NativeInterfaceP2pActiveMessage::dispatch_p2p;
            _asrb_binomial_broadcast_p2p_protocol = (MUEager*) MUEager::generate(dispatch, fn, (void*) _asrb_binomial_broadcast_ni, gdev, _allocator, result);
            _asrb_binomial_broadcast_ni->setProtocol(_asrb_binomial_broadcast_p2p_protocol);

            _asrb_binomial_broadcast_factory = new (_asrb_binomial_broadcast_factory_storage) CCMI::Adaptor::Broadcast::AsyncRBBinomialBroadcastFactory(&_rbconnmgr, _asrb_binomial_broadcast_ni);

            _active_binomial_broadcast_ni = (T_NativeInterfaceP2pActiveMessage*) new (_active_binomial_broadcast_ni_storage) T_NativeInterfaceP2pActiveMessage(client, context, context_id, client_id, dispatch);
            fn.p2p = T_NativeInterfaceP2pActiveMessage::dispatch_p2p;
            _active_binomial_broadcast_p2p_protocol = (MUEager*) MUEager::generate(dispatch, fn, (void*) _active_binomial_broadcast_ni, gdev, _allocator, result);
            _active_binomial_broadcast_ni->setProtocol(_active_binomial_broadcast_p2p_protocol);

            _active_binomial_broadcast_factory = new (_active_binomial_broadcast_factory_storage) CCMI::Adaptor::AMBroadcast::AMBinomialBroadcastFactory(&_rbconnmgr, _active_binomial_broadcast_ni);

            //set the mapid functions
            _binomial_barrier_factory->setMapIdToGeometry(mapidtogeometry);
            _asrb_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
            _ascs_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
            _active_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
          }

          if ((__global.useshmem()) && (__global.topology_local.size() > 1))
          {
            pami_result_t result = PAMI_ERROR;
            size_t dispatch = -1;

            // Shmem over P2P eager protocol
            _shmem_ni = (ShmemNI*)new (_shmem_ni_storage) ShmemNI(client, context, context_id, client_id, dispatch);

            pami_dispatch_callback_fn fn;
            fn.p2p = ShmemNI::dispatch_p2p;
            _shmem_p2p_protocol = (ShmemEager*)ShmemEager::generate(dispatch, fn, _shmem_ni, ldev, _allocator, result);
            _shmem_ni->setProtocol(_shmem_p2p_protocol);
          }

          if ((__global.useshmem()) && (__global.topology_local.size() > 1) && (__global.useMU()))
          {
            pami_result_t result = PAMI_ERROR;
            size_t dispatch = -1;
            //  Composite (shmem+MU) native interface over p2p eager
            /// \todo I think dispatch id is a problem with two protocols/one NI.  Need to fix that.
            _composite_ni = (CompositeNI*)new (_composite_ni_storage) CompositeNI(client, context, context_id, client_id, dispatch);

            _composite_p2p_protocol = (PAMI::Protocol::Send::SendPWQ< Protocol::Send::Send>*) Protocol::Send::Factory::generate (_shmem_p2p_protocol, _mu_p2p_protocol, _allocator, result);
            _composite_ni->setProtocol(_composite_p2p_protocol);
          }

//
//      _binomial_allreduce_reg.setMapIdToGeometry(mapidtogeometry);
          TRACE_ADAPTOR((stderr, "<%p>CCMIRegistration() exit\n", this));
        }

        inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry)
        {
          TRACE_ADAPTOR((stderr, "<%p>CCMIRegistration::analyze_impl() context_id %zu, geometry %p\n", this, context_id, geometry));

          if (__global.useMU())
          {
            pami_xfer_t xfer = {0};
            _binomial_barrier_composite =_binomial_barrier_factory->generate(geometry,&xfer);

            geometry->setKey(PAMI::Geometry::PAMI_GKEY_BARRIERCOMPOSITE1,
                             (void*)_binomial_barrier_composite);

            // Add Barriers
            geometry->addCollective(PAMI_XFER_BARRIER,    _binomial_barrier_factory,          _context_id);

            // Add Broadcasts
            geometry->addCollective(PAMI_XFER_BROADCAST,  _binomial_broadcast_factory,        _context_id);
            geometry->addCollective(PAMI_XFER_BROADCAST,  _ring_broadcast_factory,            _context_id);
            geometry->addCollective(PAMI_XFER_BROADCAST,  _ascs_binomial_broadcast_factory,   _context_id);
            geometry->addCollective(PAMI_XFER_BROADCAST,  _asrb_binomial_broadcast_factory,   _context_id);

            //AM Broadcast
            geometry->addCollective(PAMI_XFER_AMBROADCAST,_active_binomial_broadcast_factory, _context_id);
          }

          // Add allreduce
//      geometry->addCollective(PAMI_XFER_ALLREDUCE,&_binomial_allreduce_reg,_context_id);


          return PAMI_SUCCESS;
        }

        static pami_geometry_t mapidtogeometry (int comm)
        {
          pami_geometry_t g = geometry_map[comm];
          TRACE_ADAPTOR((stderr, "<%p>CCMIRegistration::mapidtogeometry()\n", g));
          return g;
        }

      public:
        pami_client_t                               _client;
        pami_context_t                              _context;
        size_t                                      _context_id;
        size_t                                      _client_id;

        // Protocol device(s) and allocator
        T_Local_Device                             &_local_dev;
        T_Global_Device                            &_global_dev;
        T_Allocator                                &_allocator;

        // Barrier Storage and Native Interface
        CCMI::Executor::Composite                  *_binomial_barrier_composite;

        T_NativeInterfaceP2pActiveMessage          *_binomial_barrier_ni;
        uint8_t                                     _binomial_barrier_ni_storage[sizeof(T_NativeInterfaceP2pActiveMessage)];
        MUEager                                    *_binomial_barrier_p2p_protocol;


        // Barrier Storage and Native Interface
        T_NativeInterfaceP2pAllsided               *_binomial_broadcast_ni;
        uint8_t                                     _binomial_broadcast_ni_storage[sizeof(T_NativeInterfaceP2pAllsided)];
        MUEager                                    *_binomial_broadcast_p2p_protocol;

        T_NativeInterfaceP2pAllsided               *_ring_broadcast_ni;
        uint8_t                                     _ring_broadcast_ni_storage[sizeof(T_NativeInterfaceP2pAllsided)];
        MUEager                                    *_ring_broadcast_p2p_protocol;

        T_NativeInterfaceP2pActiveMessage          *_asrb_binomial_broadcast_ni;
        uint8_t                                     _asrb_binomial_broadcast_ni_storage[sizeof(T_NativeInterfaceP2pActiveMessage)];
        MUEager                                    *_asrb_binomial_broadcast_p2p_protocol;

        T_NativeInterfaceP2pActiveMessage          *_ascs_binomial_broadcast_ni;
        uint8_t                                     _ascs_binomial_broadcast_ni_storage[sizeof(T_NativeInterfaceP2pActiveMessage)];
        MUEager                                    *_ascs_binomial_broadcast_p2p_protocol;

        T_NativeInterfaceP2pActiveMessage          *_active_binomial_broadcast_ni;
        uint8_t                                     _active_binomial_broadcast_ni_storage[sizeof(T_NativeInterfaceP2pActiveMessage)];
        MUEager                                    *_active_binomial_broadcast_p2p_protocol;

        // Allreduce Storage and Native Interface
//    T_NativeInterfaceP2pActiveMessage                                    _binomial_allreduce_ni;

        // CCMI Connection Manager Class
        CCMI::ConnectionManager::ColorGeometryConnMgr<SysDep>        _connmgr;
        CCMI::ConnectionManager::SimpleConnMgr<SysDep>               _sconnmgr;
        CCMI::ConnectionManager::RankBasedConnMgr<SysDep>            _rbconnmgr;
        CCMI::ConnectionManager::CommSeqConnMgr                      _csconnmgr;

        // CCMI Barrier Interface
        CCMI::Adaptor::Barrier::BinomialBarrierFactory              *_binomial_barrier_factory;
        uint8_t                                                      _binomial_barrier_factory_storage[sizeof(CCMI::Adaptor::Broadcast::BinomialBroadcastFactory)];

        // CCMI Binomial and Ring Broadcast
        CCMI::Adaptor::Broadcast::BinomialBroadcastFactory          *_binomial_broadcast_factory;
        uint8_t                                                      _binomial_broadcast_factory_storage[sizeof(CCMI::Adaptor::Broadcast::BinomialBroadcastFactory)];
        CCMI::Adaptor::Broadcast::RingBroadcastFactory              *_ring_broadcast_factory;
        uint8_t                                                      _ring_broadcast_factory_storage[sizeof(CCMI::Adaptor::Broadcast::RingBroadcastFactory)];
        CCMI::Adaptor::Broadcast::AsyncRBBinomialBroadcastFactory   *_asrb_binomial_broadcast_factory;
        uint8_t                                                      _asrb_binomial_broadcast_factory_storage[sizeof(CCMI::Adaptor::Broadcast::AsyncRBBinomialBroadcastFactory)];
        CCMI::Adaptor::Broadcast::AsyncCSBinomialBroadcastFactory   *_ascs_binomial_broadcast_factory;
        uint8_t                                                      _ascs_binomial_broadcast_factory_storage[sizeof(CCMI::Adaptor::Broadcast::AsyncCSBinomialBroadcastFactory)];
        CCMI::Adaptor::AMBroadcast::AMBinomialBroadcastFactory      *_active_binomial_broadcast_factory;
        uint8_t                                                      _active_binomial_broadcast_factory_storage[sizeof(CCMI::Adaptor::AMBroadcast::AMBinomialBroadcastFactory)];
//
//    CCMI::Adaptor::Allreduce::Binomial::Factory            _binomial_allreduce_reg;

        // New p2p Native interface members:
        //  Shmem (only) over p2p eager
        ShmemEager                  * _shmem_p2p_protocol;
        ShmemNI                     * _shmem_ni;
        uint8_t                       _shmem_ni_storage[sizeof(ShmemNI)];
        //  MU (only) over p2p eager
        MUEager                     * _mu_p2p_protocol;
        MUNI                        * _mu_ni;
        uint8_t                       _mu_ni_storage[sizeof(MUNI)];

        //  Composite (shmem+MU) native interface over p2p eager
        PAMI::Protocol::Send::SendPWQ< Protocol::Send::Send>        * _composite_p2p_protocol;
        CompositeNI                 * _composite_ni;
        uint8_t                       _composite_ni_storage[sizeof(CompositeNI)];
      };
    }; // BGQ
  }; // CollRegistration
}; // PAMI
#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
