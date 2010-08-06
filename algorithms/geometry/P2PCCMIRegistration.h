/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/P2PCCMIRegistration.h
 * \brief ???
 */

#ifndef __algorithms_geometry_P2PCCMIRegistration_h__
#define __algorithms_geometry_P2PCCMIRegistration_h__

#include "algorithms/geometry/P2PCCMIRegInfo.h"

// Collective Registration for CCMI protocols for p2p
namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;
  namespace CollRegistration
  {
    namespace P2P
    {
      template < class T_Geometry,
                 class T_Local_Device,                    // Local (shmem) device
                 class T_Global_Device,                   // Global (MU) device
                 class T_Allocator,
                 class T_ShmemEager,
                 class T_ShmemDevice,
                 class T_ShmemNI_AM,
                 class T_ShmemNI_AS,
                 class T_P2PEager,
                 class T_P2PDevice,
                 class T_P2PNI_AM,
                 class T_P2PNI_AS,
                 class T_CompositeNI_AM,
                 class T_CompositeNI_AS>
      class CCMIRegistration :
        public CollRegistration < PAMI::CollRegistration::P2P::CCMIRegistration < T_Geometry,
                                                                                  T_Local_Device,
                                                                                  T_Global_Device,
                                                                                  T_Allocator,
                                                                                  T_ShmemEager,
                                                                                  T_ShmemDevice,
                                                                                  T_ShmemNI_AM,
                                                                                  T_ShmemNI_AS,
                                                                                  T_P2PEager,
                                                                                  T_P2PDevice,
                                                                                  T_P2PNI_AM,
                                                                                  T_P2PNI_AS,
                                                                                  T_CompositeNI_AM,
                                                                                  T_CompositeNI_AS>,
                                  T_Geometry >
      {
        public:
          inline CCMIRegistration(pami_client_t       client,
                                  pami_context_t      context,
                                  size_t              context_id,
                                  size_t              client_id,
                                  T_Local_Device     &ldev,
                                  T_Global_Device    &gdev,
                                  T_Allocator        &allocator,
                                  bool                use_shmem,
                                  bool                use_p2p,
                                  size_t              global_size,
                                  size_t              local_size):
            CollRegistration < PAMI::CollRegistration::P2P::CCMIRegistration < T_Geometry,
                                                                               T_Local_Device,
                                                                               T_Global_Device,
                                                                               T_Allocator,
                                                                               T_ShmemEager,
                                                                               T_ShmemDevice,
                                                                               T_ShmemNI_AM,
                                                                               T_ShmemNI_AS,
                                                                               T_P2PEager,
                                                                               T_P2PDevice,
                                                                               T_P2PNI_AM,
                                                                               T_P2PNI_AS,
                                                                               T_CompositeNI_AM,
                                                                               T_CompositeNI_AS>,
                               T_Geometry > (),
            _client(client),
            _context(context),
            _context_id(context_id),
            _client_id(client_id),
            _reduce_val(0),
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
            _ascs_binomial_allreduce_factory(NULL),
            _ascs_binomial_reduce_factory(NULL),
            _composite_ni(NULL)
          {
            TRACE_INIT((stderr, "<%p>CCMIRegistration() use_shmem %s, use_p2p %s, local_size %zu, global_size %zu\n", this, use_shmem? "true":"false",use_p2p?"true":"false",local_size,global_size ));
            if ((use_shmem) && (local_size > 1) && (use_p2p))
              {
		TRACE_INIT((stderr, "<%p>CCMIRegistration() use composite\n",this));
		// Use composite P2P/Shmem if both enabled and > 1 process per node
                // Setup Composite P2p/Shmem factories
                TRACE_INIT((stderr, "<%p>CCMIRegistration() register composite\n", this ));
                setupFactories<T_CompositeNI_AM,
                               T_CompositeNI_AS,
                               T_ShmemEager,
                               T_ShmemDevice,
                               T_P2PEager,
                               T_P2PDevice>(_local_dev, _global_dev);
              }
            else if (use_p2p)
              {
		TRACE_INIT((stderr, "<%p>CCMIRegistration() use p2p\n",this));
                // Use P2P if requested or only one process (some simple test scenario)
                // Setup P2P factories
                TRACE_INIT((stderr, "<%p>CCMIRegistration() register MU\n", this ));
                setupFactories<T_P2PNI_AM,
                               T_P2PNI_AS,
                               T_P2PEager,
                               T_P2PDevice>(_global_dev);
              }
            else if ((use_shmem) && (local_size > 1))
              {
		TRACE_INIT((stderr, "<%p>CCMIRegistration() use shmem\n",this));
                // Use Shmem if requested and available ( > 1 process per node)
                // Setup Shmem factories
                TRACE_INIT((stderr, "<%p>CCMIRegistration() register shmem\n", this ));
                setupFactories<T_ShmemNI_AM,
                               T_ShmemNI_AS,
                               T_ShmemEager,
                               T_ShmemDevice>(_local_dev);
              }
            // Disabled MU and can't use shmem (only 1 process per node)? Then setup nothing
            TRACE_INIT((stderr, "<%p>CCMIRegistration() exit\n", this));
          }

          inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
          {
            TRACE_INIT((stderr,
                           "<%p>CCMIRegistration::analyze_impl() context_id %zu, geometry %p\n",
                           this,
                           context_id,
                           geometry));
	    if (phase != 0) return PAMI_SUCCESS;

            pami_xfer_t xfer = {0};
            if (_binomial_barrier_factory == NULL) // nothing setup?
              ; // then do nothing - no shmem on 1 process per node (and other protocol is disabled)
            else
              {
		TRACE_INIT((stderr, "<%p>CCMIRegistration::analyze() add\n",this));
		_binomial_barrier_composite = _binomial_barrier_factory->generate(geometry, &xfer);

              geometry->setKey(PAMI::Geometry::PAMI_GKEY_BARRIERCOMPOSITE1,
                               (void*)_binomial_barrier_composite);

              _csconnmgr.setSequence(geometry->comm());

              geometry->addCollective(PAMI_XFER_BARRIER,
                                      _binomial_barrier_factory,
                                      _context_id);
              geometry->addCollective(PAMI_XFER_BROADCAST,
                                      _binomial_broadcast_factory,
                                      _context_id);
              geometry->addCollective(PAMI_XFER_BROADCAST,
                                      _ring_broadcast_factory,
                                      _context_id);
              geometry->addCollective(PAMI_XFER_BROADCAST,
                                      _ascs_binomial_broadcast_factory,
                                      _context_id);
              geometry->addCollective(PAMI_XFER_BROADCAST,
                                      _asrb_binomial_broadcast_factory,
                                      _context_id);
              geometry->addCollective(PAMI_XFER_AMBROADCAST,
                                      _active_binomial_broadcast_factory,
                                      _context_id);
              geometry->addCollective(PAMI_XFER_ALLREDUCE,
                                      _binomial_allreduce_factory,
                                      _context_id);
              geometry->addCollective(PAMI_XFER_ALLREDUCE,
                                      _ascs_binomial_allreduce_factory,
                                      _context_id);
              geometry->addCollective(PAMI_XFER_REDUCE,
                                      _ascs_binomial_reduce_factory,
                                      _context_id);
              }
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
            TRACE_INIT((stderr, "<%p>CCMIRegistration::mapidtogeometry()\n", g));
            return g;
          }

        private:
          template<class T_NI, class T_Protocol, class T_Device, class T_Factory>
          void setupFactory(T_NI      *&ni,
                            T_Device   &device,
                            T_Factory *&factory)
          {
            pami_result_t       result   = PAMI_ERROR;
            result = NativeInterfaceCommon::constructNativeInterface<T_Allocator,
                                                                     T_NI,
                                                                     T_Protocol,
                                                                     T_Device>(_allocator,
                                                                               device,
                                                                               ni,
                                                                               _client,
                                                                               _context,
                                                                               _context_id,
                                                                               _client_id);
            PAMI_assert(result == PAMI_SUCCESS);
            COMPILE_TIME_ASSERT(sizeof(T_Factory) <= T_Allocator::objsize);
            factory = (T_Factory*) _allocator.allocateObject ();
          }

        template<class T_NI_ActiveMessage, class T_NI_Allsided, class T_Protocol, class T_Device>
          void setupFactories(T_Device &device)
          {
            T_NI_ActiveMessage *ni_am = NULL;
            T_NI_Allsided      *ni_as = NULL;

            // ----------------------------------------------------
            // Setup and Construct a binomial barrier factory from active message ni and p2p protocol
            setupFactory<T_NI_ActiveMessage, T_Protocol, T_Device,CCMI::Adaptor::P2PBarrier::BinomialBarrierFactory>(ni_am, device,_binomial_barrier_factory);
            new ((void*)_binomial_barrier_factory) CCMI::Adaptor::P2PBarrier::BinomialBarrierFactory(&_sconnmgr, ni_am, (pami_dispatch_multicast_fn)CCMI::Adaptor::P2PBarrier::BinomialBarrier::cb_head);
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Setup and Construct a binomial broadcast factory from allsided ni and p2p protocol
            setupFactory<T_NI_Allsided, T_Protocol, T_Device,CCMI::Adaptor::P2PBroadcast::BinomialBroadcastFactory>(ni_as, device,  _binomial_broadcast_factory);
            new ((void*)_binomial_broadcast_factory) CCMI::Adaptor::P2PBroadcast::BinomialBroadcastFactory(&_connmgr, ni_as);
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Setup and Construct a ring broadcast factory from allsided ni and p2p protocol
            setupFactory<T_NI_Allsided, T_Protocol, T_Device,CCMI::Adaptor::P2PBroadcast::RingBroadcastFactory>(ni_as, device,  _ring_broadcast_factory);
            new ((void*)_ring_broadcast_factory) CCMI::Adaptor::P2PBroadcast::RingBroadcastFactory(&_connmgr, ni_as);
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Setup and Construct an asynchronous, comm_id/seq_num binomial broadcast factory from active message ni and p2p protocol
            setupFactory<T_NI_ActiveMessage, T_Protocol, T_Device,CCMI::Adaptor::P2PBroadcast::AsyncCSBinomialBroadcastFactory>(ni_am, device,  _ascs_binomial_broadcast_factory);
            new ((void*)_ascs_binomial_broadcast_factory) CCMI::Adaptor::P2PBroadcast::AsyncCSBinomialBroadcastFactory(&_csconnmgr, ni_am);
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Setup and Construct an asynchronous, rank based binomial broadcast factory from active message ni and p2p protocol
            setupFactory<T_NI_ActiveMessage, T_Protocol, T_Device,CCMI::Adaptor::P2PBroadcast::AsyncRBBinomialBroadcastFactory>(ni_am, device,  _asrb_binomial_broadcast_factory);
            new ((void*)_asrb_binomial_broadcast_factory) CCMI::Adaptor::P2PBroadcast::AsyncRBBinomialBroadcastFactory(&_rbconnmgr, ni_am);
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Setup and Construct a rank based binomial active message broadcast factory from active message ni and p2p protocol
            setupFactory<T_NI_ActiveMessage, T_Protocol, T_Device,CCMI::Adaptor::P2PAMBroadcast::AMBinomialBroadcastFactory>(ni_am, device,  _active_binomial_broadcast_factory);
            new ((void*)_active_binomial_broadcast_factory) CCMI::Adaptor::P2PAMBroadcast::AMBinomialBroadcastFactory(&_rbconnmgr, ni_am);
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Setup and Construct a binomial allreducefactory from active message ni and p2p protocol
            setupFactory<T_NI_ActiveMessage, T_Protocol, T_Device,CCMI::Adaptor::P2PAllreduce::Binomial::Factory>(ni_am, device,  _binomial_allreduce_factory);
            new ((void*)_binomial_allreduce_factory) CCMI::Adaptor::P2PAllreduce::Binomial::Factory(&_rbconnmgr, ni_am, (pami_dispatch_multicast_fn)CCMI::Adaptor::P2PAllreduce::Binomial::Composite::cb_receiveHead);
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Setup and Construct an asynchronous, comm_id/seq_num binomial  allreduce factory from active message ni and p2p protocol
            setupFactory<T_NI_ActiveMessage, T_Protocol, T_Device,CCMI::Adaptor::P2PAllreduce::Binomial::AsyncCSBinomAllreduceFactory>(ni_am, device,_ascs_binomial_allreduce_factory);
            new ((void*)_ascs_binomial_allreduce_factory) CCMI::Adaptor::P2PAllreduce::Binomial::AsyncCSBinomAllreduceFactory(&_csconnmgr, ni_am);
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Setup and Construct an asynchronous, comm_id/seq_num binomial  allreduce factory from active message ni and p2p protocol
            setupFactory<T_NI_ActiveMessage, T_Protocol, T_Device,CCMI::Adaptor::P2PAllreduce::Binomial::AsyncCSBinomReduceFactory>(ni_am, device, _ascs_binomial_reduce_factory);
            new ((void*)_ascs_binomial_reduce_factory) CCMI::Adaptor::P2PAllreduce::Binomial::AsyncCSBinomReduceFactory(&_csconnmgr, ni_am);
            // ----------------------------------------------------

            //set the mapid functions
            _binomial_barrier_factory->setMapIdToGeometry(mapidtogeometry);
            _asrb_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
            _ascs_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
            _active_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
            _binomial_allreduce_factory->setMapIdToGeometry(mapidtogeometry);
            _ascs_binomial_allreduce_factory->setMapIdToGeometry(mapidtogeometry);
            _ascs_binomial_reduce_factory->setMapIdToGeometry(mapidtogeometry);
          }

          template<class T_NI,
                   class T_Protocol1,
                   class T_Device1,
                   class T_Protocol2,
                   class T_Device2,
                   class T_Factory>
          void setupFactory(T_NI        *&ni,
                            T_Device1    &device1,
                            T_Device2    &device2,
                            T_Factory   *&factory)
          {
            pami_result_t       result = PAMI_ERROR;
            // Construct an active message native interface
            result = NativeInterfaceCommon::constructNativeInterface
              <T_Allocator,
               T_NI,
               T_Protocol1,
               T_Device1,
               T_Protocol2,
               T_Device2>(_allocator,
                         device1,
                         device2,
                         ni,
                         _client,
                         _context,
                         _context_id,
                         _client_id);
            PAMI_assert(result == PAMI_SUCCESS);


            // Allocate/Construct the factory using the NI
            COMPILE_TIME_ASSERT(sizeof(T_Factory) <= T_Allocator::objsize);
            factory = (T_Factory*) _allocator.allocateObject ();
          }

          template<class T_NI_ActiveMessage,
                   class T_NI_Allsided,
                   class T_Protocol1,
                   class T_Device1,
                   class T_Protocol2,
                   class T_Device2>
          void setupFactories(T_Device1 &device1,
                              T_Device2 &device2)
          {

            T_NI_ActiveMessage *ni_am = NULL;
            T_NI_Allsided      *ni_as = NULL;

            // Setup Barriers
            setupFactory<T_NI_ActiveMessage,
                         T_Protocol1,
                         T_Device1,
                         T_Protocol2,
                         T_Device2,
                         CCMI::Adaptor::P2PBarrier::BinomialBarrierFactory>(ni_am,
                                                                            device1,
                                                                            device2,
                                                                            _binomial_barrier_factory);
            new ((void*)_binomial_barrier_factory)
              CCMI::Adaptor::P2PBarrier::BinomialBarrierFactory(&_sconnmgr,
                                                                ni_am,
                                                                (pami_dispatch_multicast_fn)CCMI::Adaptor::P2PBarrier::BinomialBarrier::cb_head);


            // Setup Broadcasts
#define SETUPFACTORY(NI, NI_VAR, FACT, FACT_VAR, CONNMGR) setupFactory<NI, \
                                                                       T_Protocol1, \
                                                                       T_Device1, \
                                                                       T_Protocol2, \
                                                                       T_Device2, \
                                                                       FACT >(NI_VAR, device1, device2, FACT_VAR); \
                                                          new ((void*)FACT_VAR) FACT(&CONNMGR, NI_VAR);
            SETUPFACTORY(T_NI_Allsided,
                         ni_as,
                         CCMI::Adaptor::P2PBroadcast::BinomialBroadcastFactory,
                         _binomial_broadcast_factory,
                         _connmgr);

            SETUPFACTORY(T_NI_Allsided,
                         ni_as,
                         CCMI::Adaptor::P2PBroadcast::RingBroadcastFactory,
                         _ring_broadcast_factory,
                         _connmgr);

            SETUPFACTORY(T_NI_ActiveMessage,
                         ni_am,
                         CCMI::Adaptor::P2PBroadcast::AsyncCSBinomialBroadcastFactory,
                         _ascs_binomial_broadcast_factory,
                         _csconnmgr);

            SETUPFACTORY(T_NI_ActiveMessage,
                         ni_am,
                         CCMI::Adaptor::P2PBroadcast::AsyncRBBinomialBroadcastFactory,
                         _asrb_binomial_broadcast_factory,
                         _rbconnmgr);

            SETUPFACTORY(T_NI_ActiveMessage,
                         ni_am,
                         CCMI::Adaptor::P2PAMBroadcast::AMBinomialBroadcastFactory,
                         _active_binomial_broadcast_factory,
                         _rbconnmgr);

            SETUPFACTORY(T_NI_ActiveMessage,
                         ni_am,
                         CCMI::Adaptor::P2PAllreduce::Binomial::Factory,
                         _binomial_allreduce_factory,
                         _rbconnmgr);
#undef SETUPFACTORY

            // Setup Allreduce
            setupFactory<T_NI_ActiveMessage,
                         T_Protocol1,
                         T_Device1,
                         T_Protocol2,
                         T_Device2,
                         CCMI::Adaptor::P2PAllreduce::Binomial::Factory>(ni_am,
                                                                         device1,
                                                                         device2,
                                                                         _binomial_allreduce_factory);
            new ((void*)_binomial_allreduce_factory)
              CCMI::Adaptor::P2PAllreduce::Binomial::Factory(&_rbconnmgr,
                                                             ni_am,
                                                             (pami_dispatch_multicast_fn)CCMI::Adaptor::P2PAllreduce::Binomial::Composite::cb_receiveHead);

            // Setup and Construct an asynchronous, comm_id/seq_num binomial  allreduce factory from active message ni and p2p protocol
            setupFactory<T_NI_ActiveMessage,
                         T_Protocol1,
                         T_Device1,
                         T_Protocol2,
                         T_Device2,
                         CCMI::Adaptor::P2PAllreduce::Binomial::AsyncCSBinomAllreduceFactory>(ni_am,
                                    device1,device2,_ascs_binomial_allreduce_factory);
            new ((void*)_ascs_binomial_allreduce_factory)
              CCMI::Adaptor::P2PAllreduce::Binomial::AsyncCSBinomAllreduceFactory(&_csconnmgr, ni_am);

            // Setup and Construct an asynchronous, comm_id/seq_num binomial reduce factory from active message ni and p2p protocol
            setupFactory<T_NI_ActiveMessage,
                         T_Protocol1,
                         T_Device1,
                         T_Protocol2,
                         T_Device2,
                         CCMI::Adaptor::P2PAllreduce::Binomial::AsyncCSBinomReduceFactory>(ni_am,
                                device1, device2, _ascs_binomial_reduce_factory);
            new ((void*)_ascs_binomial_reduce_factory)
              CCMI::Adaptor::P2PAllreduce::Binomial::AsyncCSBinomReduceFactory(&_csconnmgr, ni_am);

            //set the mapid functions
            _binomial_barrier_factory->setMapIdToGeometry(mapidtogeometry);
            _asrb_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
            _ascs_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
            _active_binomial_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
            _binomial_allreduce_factory->setMapIdToGeometry(mapidtogeometry);
            _ascs_binomial_allreduce_factory->setMapIdToGeometry(mapidtogeometry);
            _ascs_binomial_reduce_factory->setMapIdToGeometry(mapidtogeometry);
          }

      private:
          /// \todo use allocator instead of _storage?  Since they aren't always constructed, we waste memory now.
          pami_client_t                                                _client;
          pami_context_t                                               _context;
          size_t                                                       _context_id;
          size_t                                                       _client_id;
          uint64_t                                                     _reduce_val;

          // Protocol device(s) and allocator
          T_Local_Device                                              &_local_dev;
          T_Global_Device                                             &_global_dev;
          T_Allocator                                                 &_allocator;

          // Barrier Storage and Native Interface
          CCMI::Executor::Composite                                   *_binomial_barrier_composite;

          // CCMI Connection Manager Class
          CCMI::ConnectionManager::ColorGeometryConnMgr<SysDep>        _connmgr;
          CCMI::ConnectionManager::SimpleConnMgr<SysDep>               _sconnmgr;
          CCMI::ConnectionManager::RankBasedConnMgr<SysDep>            _rbconnmgr;
          CCMI::ConnectionManager::CommSeqConnMgr                      _csconnmgr;

          // CCMI Barrier Interface
          CCMI::Adaptor::P2PBarrier::BinomialBarrierFactory               *_binomial_barrier_factory;

          // CCMI Broadcasts
          CCMI::Adaptor::P2PBroadcast::BinomialBroadcastFactory           *_binomial_broadcast_factory;
          CCMI::Adaptor::P2PBroadcast::RingBroadcastFactory               *_ring_broadcast_factory;
          CCMI::Adaptor::P2PBroadcast::AsyncRBBinomialBroadcastFactory    *_asrb_binomial_broadcast_factory;
          CCMI::Adaptor::P2PBroadcast::AsyncCSBinomialBroadcastFactory    *_ascs_binomial_broadcast_factory;
          CCMI::Adaptor::P2PAMBroadcast::AMBinomialBroadcastFactory       *_active_binomial_broadcast_factory;

          // CCMI Binomial Allreduce
          CCMI::Adaptor::P2PAllreduce::Binomial::Factory                  *_binomial_allreduce_factory;

          // CCMI Async [All]Reduce
          CCMI::Adaptor::P2PAllreduce::Binomial::AsyncCSBinomAllreduceFactory  *_ascs_binomial_allreduce_factory;
          CCMI::Adaptor::P2PAllreduce::Binomial::AsyncCSBinomReduceFactory     *_ascs_binomial_reduce_factory;

          // New p2p Native interface members:

          //  Composite (shmem+MU) native interface over p2p eager
          PAMI::Protocol::Send::SendPWQ< Protocol::Send::Send>        *_composite_p2p_protocol;
          T_CompositeNI_AM                                              *_composite_ni;
          uint8_t                                                      _composite_ni_storage[sizeof(T_CompositeNI_AM)];
      };
    }; // P2P
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
