/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/BGQMultiRegistration.h
 * \brief Simple BGQ collectives over shmem/mu multi* interface
 */

#ifndef __algorithms_geometry_BGQMultiRegistration_h__
#define __algorithms_geometry_BGQMultiRegistration_h__

#include <map>
#include <vector>
#include "algorithms/interfaces/CollRegistrationInterface.h"
#include "TypeDefs.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/broadcast/MultiCastComposite.h"
#include "algorithms/protocols/allreduce/MultiCombineComposite.h"
#include "algorithms/protocols/barrier/MultiSyncComposite.h"
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"

#include "util/ccmi_debug.h" // tracing

namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;

  namespace CollRegistration
  {

    //----------------------------------------------------------------------------
    /// Declare our protocol factory templates and their metadata templates
    ///
    /// 'Pure' protocols only work on the specified (Shmem or MU) device.
    ///
    /// 'Composite' protocols combine Shmem/MU devices.
    ///
    /// 'Sub' are pure protocols that work on a subtopology of the geometry, not
    /// the whole geometry.  They may be used to create composite protocols
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    // 'Pure' Shmem allsided multisync
    //----------------------------------------------------------------------------
    void ShmemMsyncMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "ShmemMultiSync", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite<>,
    ShmemMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > ShmemMultiSyncFactory;

    //----------------------------------------------------------------------------
    // 'Pure' Shmem allsided multicombine
    //----------------------------------------------------------------------------
    void ShmemMcombMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "ShmemMultiComb", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite,
    ShmemMcombMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > ShmemMultiCombineFactory;

    //----------------------------------------------------------------------------
    // 'Pure' Shmem allsided multicast
    //----------------------------------------------------------------------------
    void ShmemMcastMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "ShmemMultiCast", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite,
    ShmemMcastMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > ShmemMultiCastFactory;


    //----------------------------------------------------------------------------
    // 'Pure' MU allsided multisync
    //----------------------------------------------------------------------------
    void MUMsyncMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "MUMultiSync", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite<>,
    MUMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUMultiSyncFactory;

    //----------------------------------------------------------------------------
    // 'Pure' MU allsided multicast
    //----------------------------------------------------------------------------
    void MUMcombMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "MUMultiComb", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite,
    MUMcombMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUMultiCombineFactory;

    //----------------------------------------------------------------------------
    // 'Pure' MU allsided multicast built on active message multicast with an
    // synchronizing multisync
    //
    // The necessary factory is defined here to implement the appropriate
    // dispatch/notifyRecv.  We simply us a map<> to associate connection id's
    // with composites.  Since it's all-sided, this should be fine.
    //
    // Connection id's are based on incrementing CommSeqConnMgr id's.
    //----------------------------------------------------------------------------
    void MUMcast2MetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "MUMultiCast_Msync", 32);
    }

    // Define our base factory
    typedef CCMI::Adaptor::CollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry>,
    MUMcast2MetaData,
    CCMI::ConnectionManager::CommSeqConnMgr > MUMultiCast2FactoryBase;

    // Extend the base factory to handle our multicast dispatch head
    class MUMultiCast2Factory : public MUMultiCast2FactoryBase
    {
      public:
        MUMultiCast2Factory (CCMI::ConnectionManager::CommSeqConnMgr *cmgr,
                             CCMI::Interfaces::NativeInterface *native,
                             pami_dispatch_multicast_fn   cb_head = NULL):
            // pass our multicast dispatch (mu2_cb_async) to the parent
            MUMultiCast2FactoryBase(cmgr, native, (pami_dispatch_multicast_fn)&mu2_cb_async)
        {
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory()\n", this));
        };
        virtual CCMI::Executor::Composite * generate(pami_geometry_t             geometry,
                                                     void                      * cmd)
        {
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory::generate()\n", this));

          // The composite will ctor the connection manager and generate a unique connection id.
          collObj *cobj = (collObj*) _alloc.allocateObject();
          TRACE_ADAPTOR((stderr, "<%p>CollectiveProtocolFactoryT::generate()\n", cobj));
          new(cobj) collObj(_native,          // Native interface
                            _cmgr,            // Connection Manager
                            geometry,         // Geometry Object
                            (pami_xfer_t*)cmd, // Parameters
                            done_fn,          // Intercept function
                            cobj,             // Intercept cookie
                            this);            // Factory

          CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry> *composite = &cobj->_obj;

          // Get the (updated and unique) connection id and store this new composite in the map[connection_id].
          BGQGeometry *bgqGeometry = (BGQGeometry *)geometry;
          unsigned comm = bgqGeometry->comm();
          cobj->_connection_id = _cmgr->getConnectionId (comm, 0, 0, 0, 0);
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory::generate() map comm %u, connection_id %u, composite %p\n", this, comm, cobj->_connection_id, composite));
          _composite_map[cobj->_connection_id] = composite;
          return composite;
        }
        static void done_fn(pami_context_t  context,
                            void           *clientdata,
                            pami_result_t   res)
        {
          collObj *cobj = (collObj *)clientdata;
          TRACE_ADAPTOR((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory::done_fn()\n", cobj));
          cobj->done_fn(context, res);
          MUMultiCast2Factory* factory = (MUMultiCast2Factory*) cobj->_factory;
          factory->_composite_map.erase(cobj->_connection_id);
          cobj->_factory->_alloc.returnObject(cobj);
        }


        ///
        /// \brief multicast dispatch - call the factory notifyRecvHead
        ///
        static void mu2_cb_async(const pami_quad_t      *msginfo,       // \param[in] msginfo    Metadata
                                 unsigned                msgcount,      // \param[in] msgcount Count of metadata
                                 unsigned                connection_id, // \param[in] connection_id  Stream ID of data
                                 size_t                  root,          // \param[in] root        Sending task
                                 size_t                  sndlen,        // \param[in] sndlen      Length of data sent
                                 void                  * clientdata,    // \param[in] clientdata  Opaque arg
                                 size_t                * rcvlen,        // \param[out] rcvlen     Length of data to receive
                                 pami_pipeworkqueue_t ** rcvpwq,        // \param[out] rcvpwq     Where to put recv data
                                 pami_callback_t       * cb_done)       // \param[out] cb_done    Completion callback to invoke when data received
        {
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory::mu2_cb_async()\n", clientdata));
          MUMultiCast2Factory* factory = (MUMultiCast2Factory*) clientdata;
          return factory->notifyRecvHead(msginfo, msgcount, connection_id, root, sndlen, rcvlen, rcvpwq, cb_done);
        }

        ///
        /// \brief Find the composite by connection_id in the map and notifyRecv()
        ///
        void notifyRecvHead(const pami_quad_t      *msginfo,       // \param[in] msginfo    Metadata
                            unsigned                msgcount,      // \param[in] msgcount Count of metadata
                            unsigned                connection_id, // \param[in] connection_id  Stream ID of data
                            size_t                  root,          // \param[in] root        Sending task
                            size_t                  sndlen,        // \param[in] sndlen      Length of data sent
                            size_t                * rcvlen,        // \param[out] rcvlen     Length of data to receive
                            pami_pipeworkqueue_t ** rcvpwq,        // \param[out] rcvpwq     Where to put recv data
                            pami_callback_t       * cb_done)       // \param[out] cb_done    Completion callback to invoke when data received
        {
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory::notifyRecvHead() msgcount %u, connection_id %u, root %zu, sndlen %zu\n", this, msgcount, connection_id, root, sndlen));
          CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry> *composite = _composite_map[connection_id];
          PAMI_assertf(composite, "connection_id %u, root %zu\n", connection_id, root);
          *rcvlen = sndlen;  // Not passed or set by notifyRecv? Just assume everything sent will be received
          return composite->notifyRecv (root,
                                        msginfo,
                                        rcvpwq,
                                        cb_done);
        };
        std::map<unsigned, CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry> * > _composite_map;
    };

    //----------------------------------------------------------------------------
    // 'Sub' Shmem allsided multisync - works on geometry->getLocalTopology()
    // (LOCAL_TOPOLOGY_INDEX)
    //----------------------------------------------------------------------------
    void SubShmemMsyncMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "ShmemMultiSyncSubComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite<LOCAL_TOPOLOGY_INDEX>,
    SubShmemMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > SubShmemMultiSyncFactory;

    //----------------------------------------------------------------------------
    // 'Sub' MU allsided multisync - works on geometry->getLocalMasterTopology()
    // (LOCAL_MASTER_TOPOLOGY_INDEX)
    //----------------------------------------------------------------------------
    void SubMUMsyncMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "MUMultiSyncSubComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite<LOCAL_MASTER_TOPOLOGY_INDEX>,
    SubMUMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > SubMUMultiSyncFactory;

    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided multisync
    //----------------------------------------------------------------------------
    void Msync2MetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "MultiSyncComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSync2Composite,
    Msync2MetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MultiSync2Factory;



    //----------------------------------------------------------------------------
    /// \brief The BGQ Multi* registration class for Shmem and MU.
    //----------------------------------------------------------------------------
    template <class T_Geometry, class T_ShmemNativeInterface, class T_MUNativeInterface>
    class BGQMultiRegistration :
        public CollRegistration<PAMI::CollRegistration::BGQMultiRegistration<T_Geometry, T_ShmemNativeInterface, T_MUNativeInterface>, T_Geometry>
    {
      public:
        inline BGQMultiRegistration(T_ShmemNativeInterface *shmem_ni,
                                    T_MUNativeInterface    *mu_ni,
                                    pami_client_t           client,
                                    pami_context_t          context,
                                    size_t                  context_id,
                                    size_t                  client_id):
            CollRegistration<PAMI::CollRegistration::BGQMultiRegistration<T_Geometry, T_ShmemNativeInterface, T_MUNativeInterface>, T_Geometry> (),
            _client(client),
            _context(context),
            _context_id(context_id),
            _sconnmgr(65535),
            _csconnmgr(),
            _shmem_barrier_composite(NULL),
            _sub_shmem_barrier_composite(NULL),
            _mu_barrier_composite(NULL),
            _sub_mu_barrier_composite(NULL),
            _msync_composite(NULL),
            _shmem_ni(shmem_ni),
            _shmem_msync_factory(&_sconnmgr, _shmem_ni),
            _sub_shmem_msync_factory(&_sconnmgr, _shmem_ni),
            _shmem_mcast_factory(&_sconnmgr, _shmem_ni),
            _shmem_mcomb_factory(&_sconnmgr, _shmem_ni),
            _mu_ni(mu_ni),
            _mu_msync_factory(&_sconnmgr, _mu_ni),
            _sub_mu_msync_factory(&_sconnmgr, _mu_ni),
            _mu_mcast2_factory(NULL),
            _mu_mcomb_factory(&_sconnmgr, _mu_ni),
            _msync_composite_factory(&_sconnmgr, NULL)
        {
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration()\n", this));
          DO_DEBUG((templateName<T_Geometry>()));
          DO_DEBUG((templateName<T_ShmemNativeInterface>()));
          DO_DEBUG((templateName<T_MUNativeInterface>()));

          //set the mapid functions
          if (__global.useshmem())// && (__global.topology_local.size() > 1))
            {
              _shmem_msync_factory.setMapIdToGeometry(mapidtogeometry);
              _sub_shmem_msync_factory.setMapIdToGeometry(mapidtogeometry);
            }

          if (__global.useMU())
            {
              _mu_msync_factory.setMapIdToGeometry(mapidtogeometry);
              _sub_mu_msync_factory.setMapIdToGeometry(mapidtogeometry);
              _msync_composite_factory.setMapIdToGeometry(mapidtogeometry);

              // Can't be ctor'd unless the NI was created
              _mu_mcast2_factory = new (_mu_mcast2_factory_storage) MUMultiCast2Factory(&_csconnmgr, _mu_ni);
            }

        }

        inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
        {
          /// \todo These are really 'must query' protocols and should not be added to the regular protocol list
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() phase %d, context_id %zu, geometry %p, msync %p, mcast %p, mcomb %p\n", this, phase, context_id, geometry, &_shmem_msync_factory, &_shmem_mcast_factory, &_shmem_mcomb_factory));
          pami_xfer_t xfer = {0};
          PAMI::Topology * topology = (PAMI::Topology*) geometry->getTopology(0);
          PAMI::Topology * local_sub_topology = (PAMI::Topology*) geometry->getLocalTopology();
          PAMI::Topology * master_sub_topology = (PAMI::Topology*) geometry->getLocalMasterTopology();
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() topology: size() %zu, isLocal() %u/%zu, isGlobal #u/%zu\n", this, topology->size(),  topology->isLocalToMe(), local_sub_topology->size(), master_sub_topology->size()));//,  topology->isGlobal()));

          DO_DEBUG(for (unsigned i = 0; i < topology->size(); ++i) fprintf(stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() topology[%u] = %u\n", this, i, topology->index2Rank(i)););

          if (phase == 0)
            {

              if ((__global.useshmem()) && (__global.topology_local.size() > 1)
                  && (__global.topology_local.size() == local_sub_topology->size()) ) /// \todo shmem doesn't seem to work on subnode topologies?
                {
                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Shmem local barrier\n", this));

                  _sub_shmem_barrier_composite = _sub_shmem_msync_factory.generate(geometry, &xfer);
                  geometry->setKey(_context_id, PAMI::Geometry::PAMI_CKEY_LOCALBARRIERCOMPOSITE,
                                   (void*)_sub_shmem_barrier_composite);


                  // If the geometry is all local nodes, we can use pure shmem composites.
                  if (topology->isLocalToMe())
                    {
                      TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Local Shmem factories\n", this));
                      _shmem_barrier_composite = _shmem_msync_factory.generate(geometry, &xfer);


                      // Add Barriers
                      geometry->addCollective(PAMI_XFER_BARRIER, &_shmem_msync_factory, _context_id);

                      // Add Broadcasts
                      geometry->addCollective(PAMI_XFER_BROADCAST, &_shmem_mcast_factory, _context_id);

                      // Add Allreduces
                      geometry->addCollective(PAMI_XFER_ALLREDUCE, &_shmem_mcomb_factory, _context_id);
                    }
                }


            }
          else if (phase == 1)
            {

              // If we have > 1 node, check MU
              if (__global.useMU())
                {
                  /// Remember, this is 'pure' MU - we won't do any shmem - so only one process per node
                  /// on the same T dimension.

                  /// A simple check (of sizes) to see if this subgeometry is all global,
                  /// then the geometry topology is usable by MU.
                  bool usePureMu = topology->size() == master_sub_topology->size() ? true : false;
                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() usePureMu = %u (size %zu/%zu)\n", this, usePureMu, topology->size(), master_sub_topology->size()));

                  void *val;
                  val = geometry->getKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID);
                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() PAMI_GKEY_MSYNC_CLASSROUTEID %p\n", this, val));

                  if (val && val != PAMI_CR_GKEY_FAIL)
                    {
                      // Only register protocols if we got a classroute
                      _sub_mu_barrier_composite = _sub_mu_msync_factory.generate(geometry, &xfer);

                      geometry->setKey(_context_id, PAMI::Geometry::PAMI_CKEY_GLOBALBARRIERCOMPOSITE,
                                       (void*)_sub_mu_barrier_composite);

                      // If we can use pure MU composites, add them
                      if (usePureMu  && !topology->isLocalToMe())
                        {
                          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register MU barrier\n", this));
                          _mu_barrier_composite = _mu_msync_factory.generate(geometry, &xfer);


                          // Add Barriers
                          geometry->addCollective(PAMI_XFER_BARRIER, &_mu_msync_factory, _context_id);
                        }
                    }

                  val = geometry->getKey(PAMI::Geometry::PAMI_GKEY_MCAST_CLASSROUTEID);
                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() PAMI_GKEY_MCAST_CLASSROUTEID %p\n", this, val));

                  if (val && val != PAMI_CR_GKEY_FAIL)
                    {
                      // Only register protocols if we got a classroute

                      // If we can use pure MU composites, add them
                      if (usePureMu && !topology->isLocalToMe())
                        {
                          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register MU bcast\n", this));
                          // Add Broadcasts
                          geometry->addCollective(PAMI_XFER_BROADCAST,  _mu_mcast2_factory, _context_id);
                        }
                    }

                  val = geometry->getKey(PAMI::Geometry::PAMI_GKEY_MCOMB_CLASSROUTEID);
                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() PAMI_GKEY_MCOMB_CLASSROUTEID %p\n", this, val));

                  if (val && val != PAMI_CR_GKEY_FAIL)
                    {
                      // If we can use pure MU composites, add them
                      if (usePureMu && !topology->isLocalToMe())
                        {
                          // Direct MU allreduce only on one context per node (lowest T, context 0)
                          if ((__global.mapping.isLowestT()) && (_context_id == 0))
                            // Add Allreduces
                            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register MU allreduce\n", this));

                          geometry->addCollective(PAMI_XFER_ALLREDUCE, &_mu_mcomb_factory, _context_id);
                        }
                    }

                  _msync_composite = _msync_composite_factory.generate(geometry, &xfer);
                  // Add Barriers
                  geometry->addCollective(PAMI_XFER_BARRIER, &_msync_composite_factory, _context_id);

                }

            }
          else if (phase == -1)
            {

              // remove MU collectives algorithms... TBD
              geometry->rmCollective(PAMI_XFER_BROADCAST, _mu_mcast2_factory, _context_id);
              geometry->rmCollective(PAMI_XFER_ALLREDUCE, &_mu_mcomb_factory, _context_id);
              geometry->rmCollective(PAMI_XFER_BARRIER, &_mu_msync_factory, _context_id);
            }

          return PAMI_SUCCESS;
        }

        static pami_geometry_t mapidtogeometry (int comm)
        {
          pami_geometry_t g = geometry_map[comm];
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::mapidtogeometry(%d)\n", g, comm));
          return g;
        }


      public:
        pami_client_t                                   _client;
        pami_context_t                                  _context;
        size_t                                          _context_id;

        // CCMI Connection Manager Class
        CCMI::ConnectionManager::SimpleConnMgr          _sconnmgr;
        CCMI::ConnectionManager::CommSeqConnMgr         _csconnmgr;

        // Barrier Storage
        CCMI::Executor::Composite                      *_shmem_barrier_composite;
        CCMI::Executor::Composite                      *_sub_shmem_barrier_composite;
        CCMI::Executor::Composite                      *_mu_barrier_composite;
        CCMI::Executor::Composite                      *_sub_mu_barrier_composite;
        CCMI::Executor::Composite                      *_msync_composite;

        //* SHMEM interfaces:
        // Native Interface
        T_ShmemNativeInterface                         *_shmem_ni;

        // CCMI Barrier Interface
        ShmemMultiSyncFactory                           _shmem_msync_factory;
        SubShmemMultiSyncFactory                        _sub_shmem_msync_factory;

        // CCMI Broadcast Interfaces
        ShmemMultiCastFactory                           _shmem_mcast_factory;

        // CCMI Allreduce Interface
        ShmemMultiCombineFactory                        _shmem_mcomb_factory;

        //* MU interfaces
        // Native Interface
        T_MUNativeInterface                            *_mu_ni;

        // CCMI Barrier Interface
        MUMultiSyncFactory                              _mu_msync_factory;
        SubMUMultiSyncFactory                           _sub_mu_msync_factory;

        // CCMI Broadcast Interfaces
        MUMultiCast2Factory                            *_mu_mcast2_factory;
        uint8_t                                         _mu_mcast2_factory_storage[sizeof(MUMultiCast2Factory)];

        // CCMI Allreduce Interface
        MUMultiCombineFactory                           _mu_mcomb_factory;

        // CCMI Barrier Interface
        MultiSync2Factory                               _msync_composite_factory;
    };



  };
};

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
