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


//#include "algorithms/protocols/barrier/impl.h"
//#include "algorithms/protocols/allreduce/sync_impl.h"
//#include "algorithms/protocols/allreduce/async_impl.h"
//#include "algorithms/protocols/alltoall/impl.h"

#include "algorithms/schedule/MultinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"
#include "algorithms/protocols/broadcast/BcastMultiColorCompositeT.h"

#include "algorithms/protocols/broadcast/AsyncBroadcastT.h"

#include "algorithms/protocols/ambcast/AMBroadcastT.h"

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
namespace CCMI
{
  namespace Adaptor
  {
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

      void binom_bcast_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        strcpy(&m->name[0], "CCMIBinomBcast");
      }

      void ring_bcast_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        strcpy(&m->name[0], "CCMIRingBcast");
      }

      typedef BcastMultiColorCompositeT < 1,
      CCMI::Schedule::ListMultinomial,
      CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS>,
      get_colors > BinomialBcastComposite;

      typedef CollectiveProtocolFactoryT < BinomialBcastComposite,
      binom_bcast_md,
      CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS> > BinomialBcastFactory;

      typedef BcastMultiColorCompositeT < 1,
      CCMI::Schedule::RingSchedule,
      CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS>,
      get_colors > RingBcastComposite;

      typedef CollectiveProtocolFactoryT < RingBcastComposite,
      ring_bcast_md,
      CCMI::ConnectionManager::ColorGeometryConnMgr<PAMI_SYSDEP_CLASS> > RingBcastFactory;

      void am_bcast_rb(pami_metadata_t *m)
      {
        TRACE_ERR((stderr, "%s\n", __PRETTY_FUNCTION__));
        // \todo:  fill in other metadata
        strcpy(&m->name[0], "CCMI_AMRB_BinomialBcast");
      }

      void am_bcast_cs(pami_metadata_t *m)
      {
        TRACE_ERR((stderr, "%s\n", __PRETTY_FUNCTION__));
        // \todo:  fill in other metadata
        strcpy(&m->name[0], "CCMI_AMCS_BinomialBcast");
      }

      typedef AsyncBroadcastT < CCMI::Schedule::ListMultinomial,
      CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AsyncRBBinomBcastComposite;
      template<>
      void AsyncRBBinomBcastComposite::create_schedule(void                        * buf,
                                                       unsigned                      size,
                                                       unsigned                      root,
                                                       Interfaces::NativeInterface * native,
                                                       PAMI_GEOMETRY_CLASS          * g)
      {
        TRACE_ERR((stderr, "<%p>AsyncRBBinomBcastComposite::create_schedule()\n", this));
        new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
      }

      typedef AsyncBroadcastFactoryT < AsyncRBBinomBcastComposite,
      am_bcast_rb,
      CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AsyncRBBinomBcastFactory;

      template<>
      unsigned AsyncRBBinomBcastFactory::getKey(unsigned                                                root,
                                                unsigned                                                connid,
                                                PAMI_GEOMETRY_CLASS                                    *geometry,
                                                ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> **connmgr)
      {
        TRACE_ERR((stderr, "<%p>AsyncRBBinomBcastFactory::getKey()\n", this));
        return root;
      }

      typedef AsyncBroadcastT < CCMI::Schedule::ListMultinomial,
      CCMI::ConnectionManager::CommSeqConnMgr > AsyncCSBinomBcastComposite;
      template<>
      void AsyncCSBinomBcastComposite::create_schedule(void                        * buf,
                                                       unsigned                      size,
                                                       unsigned                      root,
                                                       Interfaces::NativeInterface * native,
                                                       PAMI_GEOMETRY_CLASS          * g)
      {
        TRACE_ERR((stderr, "<%p>AsyncCSBinomBcastComposite::create_schedule()\n", this));
        new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
      }

      typedef AsyncBroadcastFactoryT < AsyncCSBinomBcastComposite,
      am_bcast_cs,
      CCMI::ConnectionManager::CommSeqConnMgr > AsyncCSBinomBcastFactory;

      template<>
      unsigned AsyncCSBinomBcastFactory::getKey(unsigned                                   root,
                                                unsigned                                   connid,
                                                PAMI_GEOMETRY_CLASS                      * geometry,
                                                ConnectionManager::CommSeqConnMgr        **connmgr)
      {
        TRACE_ERR((stderr, "<%p>AsyncCSBinomBcastFactory::getKey\n", this));

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
      void am_bcast_md(pami_metadata_t *m)
      {
        TRACE_ERR((stderr, "%s\n", __PRETTY_FUNCTION__));
        // \todo:  fill in other metadata
        strcpy(&m->name[0], "CCMIAMBinomialBcast");
      }

      typedef AMBroadcastT < CCMI::Schedule::ListMultinomial,
      CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AMBinomBcastComposite;
      template<>
      void AMBinomBcastComposite::create_schedule(void                        * buf,
                                                  unsigned                      size,
                                                  unsigned                      root,
                                                  Interfaces::NativeInterface * native,
                                                  PAMI_GEOMETRY_CLASS          * g)
      {
        TRACE_ERR((stderr, "<%p>AMBinomBcastComposite::create_schedule()\n", this));
        new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
      }

      typedef AMBroadcastFactoryT < AMBinomBcastComposite,
      am_bcast_md,
      CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AMBinomBcastFactory;

    }//AMBroadcast
  }//Adaptor
}//CCMI
namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;

  namespace CollRegistration
  {
    namespace BGQ
    {
    template < class T_Geometry,
    class T_NativeInterface1S,
    class T_NativeInterfaceAS,
    class T_Local_Device,
    class T_Global_Device,
    class T_Allocator >
    class CCMIRegistration :
        public CollRegistration < PAMI::CollRegistration::BGQ::CCMIRegistration < T_Geometry,
        T_NativeInterface1S, // Onesided NI
        T_NativeInterfaceAS, // Allsided NI
        T_Local_Device,
        T_Global_Device,
        T_Allocator > ,
        T_Geometry >
    {
      public:
        inline CCMIRegistration(pami_client_t       client,
                                pami_context_t      context,
                                size_t             context_id,
                                size_t             client_id,
                                T_Local_Device     &ldev,
                                T_Global_Device    &gdev,
                                T_Allocator        &allocator):
            CollRegistration < PAMI::CollRegistration::BGQ::CCMIRegistration < T_Geometry,
            T_NativeInterface1S,
            T_NativeInterfaceAS,
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
//    _msync_ni           (gdev, client,context,context_id,client_id),
//    _barrier_ni         (gdev, client,context,context_id,client_id),
            _binom_broadcast_ni ((T_NativeInterfaceAS*)&_binom_broadcast_ni_storage),
            _binom_broadcast_eager(NULL),
            _ring_broadcast_ni  ((T_NativeInterfaceAS*)&_binom_broadcast_ni_storage),
            _ring_broadcast_eager(NULL),
            _asrb_binom_broadcast_ni  ((T_NativeInterface1S*)&_asrb_binom_broadcast_ni_storage),
            _ascs_binom_broadcast_ni((T_NativeInterface1S*)&_ascs_binom_broadcast_ni_storage),
            _active_binom_broadcast_ni  ((T_NativeInterface1S*)&_active_binom_broadcast_ni_storage),
//    _binom_allreduce_ni (gdev, client,context,context_id,client_id),
            _connmgr(65535),
            _rbconnmgr(NULL),
            _csconnmgr(),
//    _msync_reg(&_sconnmgr, &_msync_ni),
//    _barrier_reg(NULL,&_barrier_ni, (pami_dispatch_multicast_fn)CCMI::Adaptor::Barrier::BinomialBarrier::cb_head),
            _binom_broadcast_factory(NULL),
            _ring_broadcast_factory(NULL),
            _asrb_binom_broadcast_factory(NULL),
            _ascs_binom_broadcast_factory(NULL),
            _active_binom_broadcast_factory(NULL),
//    _binomial_allreduce_reg(&_rbconnmgr, &_binom_allreduce_ni, (pami_dispatch_multicast_fn)CCMI::Adaptor::Allreduce::Binomial::Composite::cb_receiveHead)
            _shmem_eager(NULL),
            _shmem_ni(NULL),
            _mu_eager(NULL),
            _mu_ni(NULL),
            _composite_ni(NULL)
        {

          TRACE_ERR((stderr, "<%p>CCMIRegistration()\n", this ));

          if (__global.useMU())
            {
              pami_result_t result = PAMI_ERROR;
              size_t dispatch = -1;

              // MU over P2P eager protocol
              _mu_ni = (MUNI*) new (_mu_ni_storage) MUNI(client, context, context_id, client_id, dispatch);

              pami_dispatch_callback_fn fn;
              fn.p2p = MUNI::dispatch_p2p;
              _mu_eager = (MUEager*) MUEager::generate(dispatch, fn, (void*) _mu_ni, gdev, _allocator, result);
              _mu_ni->setProtocol(_mu_eager);


              _binom_broadcast_ni = (T_NativeInterfaceAS*) new (_binom_broadcast_ni_storage) T_NativeInterfaceAS(client, context, context_id, client_id, dispatch);
              fn.p2p = T_NativeInterfaceAS::dispatch_p2p;
              _binom_broadcast_eager = (MUEager*) MUEager::generate(dispatch, fn, (void*) _binom_broadcast_ni, gdev, _allocator, result);
              _binom_broadcast_ni->setProtocol(_binom_broadcast_eager);

              _binom_broadcast_factory = new (_binom_broadcast_factory_storage) CCMI::Adaptor::Broadcast::BinomialBcastFactory(&_connmgr, _binom_broadcast_ni);

              _ring_broadcast_ni = (T_NativeInterfaceAS*) new (_ring_broadcast_ni_storage) T_NativeInterfaceAS(client, context, context_id, client_id, dispatch);
              fn.p2p = T_NativeInterfaceAS::dispatch_p2p;
              _ring_broadcast_eager = (MUEager*) MUEager::generate(dispatch, fn, (void*) _ring_broadcast_ni, gdev, _allocator, result);
              _ring_broadcast_ni->setProtocol(_ring_broadcast_eager);

              _ring_broadcast_factory = new (_ring_broadcast_factory_storage) CCMI::Adaptor::Broadcast::RingBcastFactory(&_connmgr, _ring_broadcast_ni);

              _ascs_binom_broadcast_ni = (T_NativeInterface1S*) new (_ascs_binom_broadcast_ni_storage) T_NativeInterface1S(client, context, context_id, client_id, dispatch);
              fn.p2p = T_NativeInterface1S::dispatch_p2p;
              _ascs_binom_broadcast_eager = (MUEager*) MUEager::generate(dispatch, fn, (void*) _ascs_binom_broadcast_ni, gdev, _allocator, result);
              _ascs_binom_broadcast_ni->setProtocol(_ascs_binom_broadcast_eager);

              _ascs_binom_broadcast_factory = new (_ascs_binom_broadcast_factory_storage) CCMI::Adaptor::Broadcast::AsyncCSBinomBcastFactory(&_csconnmgr, _ascs_binom_broadcast_ni);

              _asrb_binom_broadcast_ni = (T_NativeInterface1S*) new (_asrb_binom_broadcast_ni_storage) T_NativeInterface1S(client, context, context_id, client_id, dispatch);
              fn.p2p = T_NativeInterface1S::dispatch_p2p;
              _asrb_binom_broadcast_eager = (MUEager*) MUEager::generate(dispatch, fn, (void*) _asrb_binom_broadcast_ni, gdev, _allocator, result);
              _asrb_binom_broadcast_ni->setProtocol(_asrb_binom_broadcast_eager);

              _asrb_binom_broadcast_factory = new (_asrb_binom_broadcast_factory_storage) CCMI::Adaptor::Broadcast::AsyncRBBinomBcastFactory(&_rbconnmgr, _asrb_binom_broadcast_ni);

              _active_binom_broadcast_ni = (T_NativeInterface1S*) new (_active_binom_broadcast_ni_storage) T_NativeInterface1S(client, context, context_id, client_id, dispatch);
              fn.p2p = T_NativeInterface1S::dispatch_p2p;
              _active_binom_broadcast_eager = (MUEager*) MUEager::generate(dispatch, fn, (void*) _active_binom_broadcast_ni, gdev, _allocator, result);
              _active_binom_broadcast_ni->setProtocol(_active_binom_broadcast_eager);

              _active_binom_broadcast_factory = new (_active_binom_broadcast_factory_storage) CCMI::Adaptor::AMBroadcast::AMBinomBcastFactory(&_rbconnmgr, _active_binom_broadcast_ni);

              _asrb_binom_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
              _ascs_binom_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
              _active_binom_broadcast_factory->setMapIdToGeometry(mapidtogeometry);
            }

          if ((__global.useshmem()) && (__global.topology_local.size() > 1))
            {
              pami_result_t result = PAMI_ERROR;
              size_t dispatch = -1;

              // Shmem over P2P eager protocol
              _shmem_ni = (ShmemNI*)new (_shmem_ni_storage) ShmemNI(client, context, context_id, client_id, dispatch);

              pami_dispatch_callback_fn fn;
              fn.p2p = ShmemNI::dispatch_p2p;
              _shmem_eager = (ShmemEager*)ShmemEager::generate(dispatch, fn, _shmem_ni, ldev, _allocator, result);
              _shmem_ni->setProtocol(_shmem_eager);
            }

          if ((__global.useshmem()) && (__global.topology_local.size() > 1) && (__global.useMU()))
            {
              pami_result_t result = PAMI_ERROR;
              size_t dispatch = -1;
              //  Composite (shmem+MU) native interface over p2p eager
              /// \todo I think dispatch id is a problem with two protocols/one NI.  Need to fix that.
              _composite_ni = (CompositeNI*)new (_composite_ni_storage) CompositeNI(client, context, context_id, client_id, dispatch);

              _composite_eager = (PAMI::Protocol::Send::SendPWQ< Protocol::Send::Send>*) Protocol::Send::Factory::generate (_shmem_eager, _mu_eager, _allocator, result);
              _composite_ni->setProtocol(_composite_eager);
            }

          //set the mapid functions
//      _barrier_reg.setMapIdToGeometry(mapidtogeometry);
//
//      _binomial_allreduce_reg.setMapIdToGeometry(mapidtogeometry);
          TRACE_ERR((stderr, "<%p>CCMIRegistration() exit\n", this));
        }

        inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry)
        {
          TRACE_ERR((stderr, "<%p>CCMIRegistration::analyze_impl() context_id %zu, geometry %p\n", this, context_id, geometry));
//      pami_xfer_t xfer = {0};
//      _barrier_composite =_barrier_reg.generate(geometry,
//                                                &xfer);

//      geometry->setKey(PAMI::Geometry::PAMI_GKEY_BARRIERCOMPOSITE1,
//                       (void*)_barrier_composite);

          // Add Barriers
//      geometry->addCollective(PAMI_XFER_BARRIER,&_msync_reg,_context_id);
//      geometry->addCollective(PAMI_XFER_BARRIER,&_barrier_reg,_context_id);

          if (__global.useMU())
          {
            // Add Broadcasts
            geometry->addCollective(PAMI_XFER_BROADCAST, _binom_broadcast_factory, _context_id);
            geometry->addCollective(PAMI_XFER_BROADCAST, _ring_broadcast_factory,  _context_id);
            geometry->addCollective(PAMI_XFER_BROADCAST, _ascs_binom_broadcast_factory, _context_id);
            geometry->addCollective(PAMI_XFER_BROADCAST, _asrb_binom_broadcast_factory, _context_id);

            //AM Broadcast
            geometry->addCollective(PAMI_XFER_AMBROADCAST,_active_binom_broadcast_factory, _context_id);
          }

          // Add allreduce
//      geometry->addCollective(PAMI_XFER_ALLREDUCE,&_binomial_allreduce_reg,_context_id);


          return PAMI_SUCCESS;
        }

        static pami_geometry_t mapidtogeometry (int comm)
        {
          pami_geometry_t g = geometry_map[comm];
          TRACE_ERR((stderr, "<%p>CCMIRegistration::mapidtogeometry()\n", g));
          return g;
        }

      public:
        pami_client_t                                          _client;
        pami_context_t                                         _context;
        size_t                                                 _context_id;
        size_t                                                 _client_id;

        // Barrier Storage
//    CCMI::Executor::Composite                             *_barrier_composite;

        // Native Interface
        T_Local_Device                                        &_local_dev;
        T_Global_Device                                       &_global_dev;
        T_Allocator                                           &_allocator;
//    T_NativeInterface1S                                    _msync_ni;
//    T_NativeInterface1S                                    _barrier_ni;
        T_NativeInterfaceAS                                   *_binom_broadcast_ni;
        uint8_t                                                _binom_broadcast_ni_storage[sizeof(T_NativeInterfaceAS)];
        MUEager                                               * _binom_broadcast_eager;
        T_NativeInterfaceAS                                   *_ring_broadcast_ni;
        uint8_t                                                _ring_broadcast_ni_storage[sizeof(T_NativeInterfaceAS)];
        MUEager                                               * _ring_broadcast_eager;
        T_NativeInterface1S                                   *_asrb_binom_broadcast_ni;
        uint8_t                                                _asrb_binom_broadcast_ni_storage[sizeof(T_NativeInterface1S)];
        MUEager                                               *_asrb_binom_broadcast_eager;
        T_NativeInterface1S                                   *_ascs_binom_broadcast_ni;
        uint8_t                                                _ascs_binom_broadcast_ni_storage[sizeof(T_NativeInterface1S)];
        MUEager                                               *_ascs_binom_broadcast_eager;
        T_NativeInterface1S                                   *_active_binom_broadcast_ni;
        uint8_t                                                _active_binom_broadcast_ni_storage[sizeof(T_NativeInterface1S)];
        MUEager                                               *_active_binom_broadcast_eager;
//    T_NativeInterface1S                                    _binom_allreduce_ni;

        // CCMI Connection Manager Class
        CCMI::ConnectionManager::ColorGeometryConnMgr<SysDep>  _connmgr;
//    CCMI::ConnectionManager::SimpleConnMgr<SysDep>         _sconnmgr;
        CCMI::ConnectionManager::RankBasedConnMgr<SysDep>      _rbconnmgr;
        CCMI::ConnectionManager::CommSeqConnMgr                _csconnmgr;

        // CCMI Barrier Interface
//    CCMI::Adaptor::Barrier::MultiSyncFactory               _msync_reg;
//    CCMI::Adaptor::Barrier::BinomialBarrierFactory         _barrier_reg;

        // CCMI Binomial and Ring Broadcast
        CCMI::Adaptor::Broadcast::BinomialBcastFactory        *_binom_broadcast_factory;
        uint8_t                                                _binom_broadcast_factory_storage[sizeof(CCMI::Adaptor::Broadcast::BinomialBcastFactory)];
        CCMI::Adaptor::Broadcast::RingBcastFactory            *_ring_broadcast_factory;
        uint8_t                                                _ring_broadcast_factory_storage[sizeof(CCMI::Adaptor::Broadcast::RingBcastFactory)];
        CCMI::Adaptor::Broadcast::AsyncRBBinomBcastFactory    *_asrb_binom_broadcast_factory;
        uint8_t                                                _asrb_binom_broadcast_factory_storage[sizeof(CCMI::Adaptor::Broadcast::AsyncRBBinomBcastFactory)];
        CCMI::Adaptor::Broadcast::AsyncCSBinomBcastFactory    *_ascs_binom_broadcast_factory;
        uint8_t                                                _ascs_binom_broadcast_factory_storage[sizeof(CCMI::Adaptor::Broadcast::AsyncCSBinomBcastFactory)];
        CCMI::Adaptor::AMBroadcast::AMBinomBcastFactory       *_active_binom_broadcast_factory;
        uint8_t                                                _active_binom_broadcast_factory_storage[sizeof(CCMI::Adaptor::AMBroadcast::AMBinomBcastFactory)];
//
//    CCMI::Adaptor::Allreduce::Binomial::Factory            _binomial_allreduce_reg;

        // New p2p Native interface members:
        //  Shmem (only) over p2p eager
        ShmemEager                  * _shmem_eager;
        ShmemNI                     * _shmem_ni;
        uint8_t                       _shmem_ni_storage[sizeof(ShmemNI)];
        //  MU (only) over p2p eager
        MUEager                     * _mu_eager;
        MUNI                        * _mu_ni;
        uint8_t                       _mu_ni_storage[sizeof(MUNI)];
        //  Composite (shmem+MU) native interface over p2p eager
        PAMI::Protocol::Send::SendPWQ< Protocol::Send::Send>        * _composite_eager;
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
