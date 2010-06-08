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
#include "util/ccmi_debug.h"

#include "TypeDefs.h"

//#include "algorithms/protocols/alltoall/impl.h"

#include "algorithms/schedule/MultinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"
#include "algorithms/protocols/broadcast/BcastMultiColorCompositeT.h"

#include "algorithms/protocols/barrier/BarrierT.h"
#include "algorithms/protocols/broadcast/AsyncBroadcastT.h"

#include "algorithms/protocols/ambcast/AMBroadcastT.h"

#include "algorithms/protocols/allreduce/MultiColorCompositeT.h"
#include "algorithms/protocols/allreduce/ProtocolFactoryT.h"

// CCMI Template implementations for BGQ
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


      typedef CCMI::Adaptor::Barrier::BarrierT <CCMI::Schedule::ListMultinomial,
      binomial_analyze> BinomialBarrier;

      typedef CCMI::Adaptor::Barrier::BarrierFactoryT <BinomialBarrier,
      binomial__barrier_md,
      CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > BinomialBarrierFactory;
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

      typedef CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT < 1,
      CCMI::Schedule::ListMultinomial,
      CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS>,
      get_colors > BinomialBroadcastComposite;

      typedef CCMI::Adaptor::CollectiveProtocolFactoryT < BinomialBroadcastComposite,
      binomial_broadcast_metadata,
      CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS> > BinomialBroadcastFactory;

      typedef CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT < 1,
      CCMI::Schedule::RingSchedule,
      CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS>,
      get_colors > RingBroadcastComposite;

      typedef CCMI::Adaptor::CollectiveProtocolFactoryT < RingBroadcastComposite,
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

      typedef CCMI::Adaptor::Broadcast::AsyncBroadcastT < CCMI::Schedule::ListMultinomial,
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

      typedef CCMI::Adaptor::Broadcast::AsyncBroadcastFactoryT < AsyncRBBinomialBroadcastComposite,
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

      typedef CCMI::Adaptor::Broadcast::AsyncBroadcastT < CCMI::Schedule::ListMultinomial,
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

      typedef CCMI::Adaptor::Broadcast::AsyncBroadcastFactoryT < AsyncCSBinomialBroadcastComposite,
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

      typedef CCMI::Adaptor::AMBroadcast::AMBroadcastT < CCMI::Schedule::ListMultinomial,
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

      typedef CCMI::Adaptor::AMBroadcast::AMBroadcastFactoryT < AMBinomialBroadcastComposite,
      am_broadcast_metadata,
      CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AMBinomialBroadcastFactory;

    }//AMBroadcast
    namespace Allreduce
    {
      /// New Binomial algorithms
      /// class Binomial::Composite and Binomial::Factory
      ///
      /// \brief Binomial allreduce protocol
      ///
      /// Use the BinomialTreeSchedule
      ///
      namespace Binomial
      {
        void get_colors (PAMI::Topology             * t,
                         unsigned                    bytes,
                         unsigned                  * colors,
                         unsigned                  & ncolors)
        {
          ncolors = 1;
          colors[0] = CCMI::Schedule::NO_COLOR;
        }

        void binomial_allreduce_metadata(pami_metadata_t *m)
        {
          // \todo:  fill in other metadata
          strcpy(&m->name[0],"Binomial_Allreduce");
        }

        typedef CCMI::Adaptor::Allreduce::MultiColorCompositeT<1, CCMI::Executor::AllreduceBaseExec<CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> >,
        CCMI::Schedule::ListMultinomial,
        CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS>,
        get_colors> Composite;
        typedef CCMI::Adaptor::Allreduce::ProtocolFactoryT<Composite, binomial_allreduce_metadata, CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > Factory;
      };//Binomial
    };//Allreduce
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
        _connmgr(65535),
        _rbconnmgr(NULL),
        _csconnmgr(),
        _binomial_barrier_factory(NULL),
        _binomial_broadcast_factory(NULL),
        _ring_broadcast_factory(NULL),
        _asrb_binomial_broadcast_factory(NULL),
        _ascs_binomial_broadcast_factory(NULL),
        _active_binomial_broadcast_factory(NULL),
        _binomial_allreduce_factory(NULL),
        _composite_ni(NULL)
        {

          TRACE_ADAPTOR((stderr, "<%p>CCMIRegistration()\n", this ));

          // Use composite MU/Shmem if both enabled and > 1 process per node
          if ((__global.useshmem()) && (__global.topology_local.size() > 1) && (__global.useMU()))
          {
            TRACE_ADAPTOR((stderr, "<%p>CCMIRegistration() register composite\n", this ));
            // Setup Composite MU/Shmem factories
            setupFactories<CompositeNI_AM, CompositeNI_AS,ShmemEager, ShmemDevice, MUEager, MUDevice>(_local_dev,_global_dev);
          }
          // Use MU if requested or only one process (some simple test scenario)
          else if (__global.useMU() || (__global.topology_global.size() == 1))
          {
            TRACE_ADAPTOR((stderr, "<%p>CCMIRegistration() register MU\n", this ));
            // Setup MU factories
            setupFactories<MUNI_AM, MUNI_AS,MUEager,MUDevice>(_global_dev);

          }
          // Use Shmem if requested and available ( > 1 process per node)
          else if ((__global.useshmem()) && (__global.topology_local.size() > 1))
          {
            TRACE_ADAPTOR((stderr, "<%p>CCMIRegistration() register shmem\n", this ));
            // Setup Shmem factories
            setupFactories<ShmemNI_AM,ShmemNI_AS,ShmemEager,ShmemDevice>(_local_dev);
          }
          // Disabled MU and can't use shmem (only 1 process per node)? Then abort.
          else PAMI_abort();

          TRACE_ADAPTOR((stderr, "<%p>CCMIRegistration() exit\n", this));
        }

        inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry)
        {
          TRACE_ADAPTOR((stderr, "<%p>CCMIRegistration::analyze_impl() context_id %zu, geometry %p\n", this, context_id, geometry));

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

          // Add allreduce
          geometry->addCollective(PAMI_XFER_ALLREDUCE,  _binomial_allreduce_factory,        _context_id);

          return PAMI_SUCCESS;
        }

        static pami_geometry_t mapidtogeometry (int comm)
        {
          pami_geometry_t g = geometry_map[comm];
          TRACE_ADAPTOR((stderr, "<%p>CCMIRegistration::mapidtogeometry()\n", g));
          return g;
        }

      public:

        /// \todo use allocator instead of _storage?  Since they aren't always constructed, we waste memory now.

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

        // CCMI Connection Manager Class
        CCMI::ConnectionManager::ColorGeometryConnMgr<SysDep>        _connmgr;
        CCMI::ConnectionManager::SimpleConnMgr<SysDep>               _sconnmgr;
        CCMI::ConnectionManager::RankBasedConnMgr<SysDep>            _rbconnmgr;
        CCMI::ConnectionManager::CommSeqConnMgr                      _csconnmgr;

        // CCMI Barrier Interface
        CCMI::Adaptor::Barrier::BinomialBarrierFactory               *_binomial_barrier_factory;

        // CCMI Broadcasts
        CCMI::Adaptor::Broadcast::BinomialBroadcastFactory           *_binomial_broadcast_factory;
        CCMI::Adaptor::Broadcast::RingBroadcastFactory               *_ring_broadcast_factory;
        CCMI::Adaptor::Broadcast::AsyncRBBinomialBroadcastFactory    *_asrb_binomial_broadcast_factory;
        CCMI::Adaptor::Broadcast::AsyncCSBinomialBroadcastFactory    *_ascs_binomial_broadcast_factory;
        CCMI::Adaptor::AMBroadcast::AMBinomialBroadcastFactory       *_active_binomial_broadcast_factory;

        // CCMI Binomial Allreduce
        CCMI::Adaptor::Allreduce::Binomial::Factory                  *_binomial_allreduce_factory;

        // New p2p Native interface members:

        //  Composite (shmem+MU) native interface over p2p eager
        PAMI::Protocol::Send::SendPWQ< Protocol::Send::Send>        * _composite_p2p_protocol;
        CompositeNI_AM                 * _composite_ni;
        uint8_t                       _composite_ni_storage[sizeof(CompositeNI_AM)];
      private:
        template<class T_NI, class T_Protocol, class T_Device, class T_Factory> 
        void setupFactory(T_NI &ni, T_Device &device, T_Protocol &protocol, T_Factory *&factory)
        {
          pami_result_t       result = PAMI_ERROR;
          size_t              dispatch = -1;

          // Get the next dispatch id to use for this NI/protocol
          dispatch = PAMI::NativeInterfaceCommon::getNextDispatch();

          // Construct an active message native interface
          result = NativeInterfaceCommon::constructNativeInterface(_allocator, device, protocol, ni, _client, _context, _context_id, _client_id, dispatch);
          PAMI_assert(result == PAMI_SUCCESS);


          // Allocate/Construct the factory using the NI
          COMPILE_TIME_ASSERT(sizeof(T_Factory) <= T_Allocator::objsize);
          factory = (T_Factory*) _allocator.allocateObject ();
        }
        template<class T_NI_ActiveMessage, class T_NI_Allsided, class T_Protocol, class T_Device> 
        void setupFactories(T_Device &device)
        {
          T_NI_ActiveMessage *ni_am = NULL;
          T_NI_Allsided      *ni_as = NULL;
          T_Protocol         *protocol = NULL;

          // The #define FACTORY is used to shorten the code and avoid copy/paste typo's

          // ----------------------------------------------------
          // Setup and Construct a binomial barrier factory from active message ni and p2p protocol
          setupFactory(ni_am, device, protocol, _binomial_barrier_factory);
          new ((void*)_binomial_barrier_factory) CCMI::Adaptor::Barrier::BinomialBarrierFactory(&_sconnmgr, ni_am,(pami_dispatch_multicast_fn)CCMI::Adaptor::Barrier::BinomialBarrier::cb_head);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct a binomial broadcast factory from allsided ni and p2p protocol
          setupFactory(ni_as, device, protocol, _binomial_broadcast_factory);
          new ((void*)_binomial_broadcast_factory) CCMI::Adaptor::Broadcast::BinomialBroadcastFactory(&_connmgr, ni_as);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct a ring broadcast factory from allsided ni and p2p protocol
          setupFactory(ni_as, device, protocol, _ring_broadcast_factory);
          new ((void*)_ring_broadcast_factory) CCMI::Adaptor::Broadcast::RingBroadcastFactory(&_connmgr, ni_as);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct an asynchronous, comm_id/seq_num binomial broadcast factory from active message ni and p2p protocol
          setupFactory(ni_am, device, protocol, _ascs_binomial_broadcast_factory);
          new ((void*)_ascs_binomial_broadcast_factory) CCMI::Adaptor::Broadcast::AsyncCSBinomialBroadcastFactory(&_csconnmgr, ni_am);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct an asynchronous, rank based binomial broadcast factory from active message ni and p2p protocol
          setupFactory(ni_am, device, protocol, _asrb_binomial_broadcast_factory);
          new ((void*)_asrb_binomial_broadcast_factory) CCMI::Adaptor::Broadcast::AsyncRBBinomialBroadcastFactory(&_rbconnmgr, ni_am);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct a rank based binomial active message broadcast factory from active message ni and p2p protocol
          setupFactory(ni_am, device, protocol, _active_binomial_broadcast_factory);
          new ((void*)_active_binomial_broadcast_factory) CCMI::Adaptor::AMBroadcast::AMBinomialBroadcastFactory(&_rbconnmgr, ni_am);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct a binomial allreducefactory from active message ni and p2p protocol
          setupFactory(ni_am, device, protocol, _binomial_allreduce_factory);
          new ((void*)_binomial_allreduce_factory) CCMI::Adaptor::Allreduce::Binomial::Factory(&_rbconnmgr, ni_am, (pami_dispatch_multicast_fn)CCMI::Adaptor::Allreduce::Binomial::Composite::cb_receiveHead);
          // ----------------------------------------------------


          //set the mapid functions
          _binomial_barrier_factory->setMapIdToGeometry(mapidtogeometry);
          _asrb_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
          _ascs_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
          _active_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
          _binomial_allreduce_factory->setMapIdToGeometry(mapidtogeometry);
        }
        template<class T_NI, class T_Protocol1, class T_Device1, class T_Protocol2, class T_Device2, class T_Factory> 
        void setupFactory(T_NI &ni, T_Device1 &device1, T_Protocol1 &protocol1, T_Device2 &device2, T_Protocol2 &protocol2, T_Factory *&factory)
        {
          pami_result_t       result = PAMI_ERROR;
          size_t              dispatch = -1;

          // Get the next dispatch id to use for this NI/protocol
          dispatch = PAMI::NativeInterfaceCommon::getNextDispatch();

          // Construct an active message native interface
          result = NativeInterfaceCommon::constructNativeInterface(_allocator, device1, protocol1, device2, protocol2, ni, _client, _context, _context_id, _client_id, dispatch);
          PAMI_assert(result == PAMI_SUCCESS);


          // Allocate/Construct the factory using the NI
          COMPILE_TIME_ASSERT(sizeof(T_Factory) <= T_Allocator::objsize);
          factory = (T_Factory*) _allocator.allocateObject ();
        }
        template<class T_NI_ActiveMessage, class T_NI_Allsided, class T_Protocol1, class T_Device1, class T_Protocol2, class T_Device2> 
        void setupFactories(T_Device1 &device1, T_Device2 &device2)
        {
          T_NI_ActiveMessage *ni_am = NULL;
          T_NI_Allsided      *ni_as = NULL;
          T_Protocol1         *protocol1 = NULL;
          T_Protocol2         *protocol2 = NULL;

          // The #define FACTORY is used to shorten the code and avoid copy/paste typo's

          // ----------------------------------------------------
          // Setup and Construct a binomial barrier factory from active message ni and p2p protocol
          setupFactory(ni_am, device1, protocol1, device2, protocol2, _binomial_barrier_factory);
          new ((void*)_binomial_barrier_factory) CCMI::Adaptor::Barrier::BinomialBarrierFactory(&_sconnmgr, ni_am,(pami_dispatch_multicast_fn)CCMI::Adaptor::Barrier::BinomialBarrier::cb_head);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct a binomial broadcast factory from allsided ni and p2p protocol
          setupFactory(ni_as, device1, protocol1, device2, protocol2, _binomial_broadcast_factory);
          new ((void*)_binomial_broadcast_factory) CCMI::Adaptor::Broadcast::BinomialBroadcastFactory(&_connmgr, ni_as);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct a ring broadcast factory from allsided ni and p2p protocol
          setupFactory(ni_as, device1, protocol1, device2, protocol2, _ring_broadcast_factory);
          new ((void*)_ring_broadcast_factory) CCMI::Adaptor::Broadcast::RingBroadcastFactory(&_connmgr, ni_as);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct an asynchronous, comm_id/seq_num binomial broadcast factory from active message ni and p2p protocol
          setupFactory(ni_am, device1, protocol1, device2, protocol2, _ascs_binomial_broadcast_factory);
          new ((void*)_ascs_binomial_broadcast_factory) CCMI::Adaptor::Broadcast::AsyncCSBinomialBroadcastFactory(&_csconnmgr, ni_am);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct an asynchronous, rank based binomial broadcast factory from active message ni and p2p protocol
          setupFactory(ni_am, device1, protocol1, device2, protocol2, _asrb_binomial_broadcast_factory);
          new ((void*)_asrb_binomial_broadcast_factory) CCMI::Adaptor::Broadcast::AsyncRBBinomialBroadcastFactory(&_rbconnmgr, ni_am);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct a rank based binomial active message broadcast factory from active message ni and p2p protocol
          setupFactory(ni_am, device1, protocol1, device2, protocol2, _active_binomial_broadcast_factory);
          new ((void*)_active_binomial_broadcast_factory) CCMI::Adaptor::AMBroadcast::AMBinomialBroadcastFactory(&_rbconnmgr, ni_am);
          // ----------------------------------------------------

          // ----------------------------------------------------
          // Setup and Construct a binomial allreducefactory from active message ni and p2p protocol
          setupFactory(ni_am, device1, protocol1, device2, protocol2, _binomial_allreduce_factory);
          new ((void*)_binomial_allreduce_factory) CCMI::Adaptor::Allreduce::Binomial::Factory(&_rbconnmgr, ni_am, (pami_dispatch_multicast_fn)CCMI::Adaptor::Allreduce::Binomial::Composite::cb_receiveHead);
          // ----------------------------------------------------


          //set the mapid functions
          _binomial_barrier_factory->setMapIdToGeometry(mapidtogeometry);
          _asrb_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
          _ascs_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
          _active_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
          _binomial_allreduce_factory->setMapIdToGeometry(mapidtogeometry);
        }
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
