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
#include "SysDep.h"
#include "TypeDefs.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/protocols/broadcast/MultiCastComposite.h"
#include "algorithms/protocols/allreduce/MultiCombineComposite.h"
#include "algorithms/protocols/barrier/MultiSyncComposite.h"
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;

  namespace CollRegistration
  {

    //----------------------------------------------------------------------------
    /// Declare our protocol factory templates and their metadata templates
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    // Shmem allsided multisync
    //----------------------------------------------------------------------------
    void ShmemMsyncMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "ShmemMultiSyncComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite,
    ShmemMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > ShmemMultiSyncFactory;

    //----------------------------------------------------------------------------
    // Shmem allsided multicombine
    //----------------------------------------------------------------------------
    void ShmemMcombMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "ShmemMultiCombComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite,
    ShmemMcombMetaData,
    CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > ShmemMultiCombineFactory;

    //----------------------------------------------------------------------------
    // Shmem allsided multicast
    //----------------------------------------------------------------------------
    void ShmemMcastMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "ShmemMultiCastComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite,
    ShmemMcastMetaData,
    CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > ShmemMultiCastFactory;

    //----------------------------------------------------------------------------
    // Shmem allsided multicast built on active message multicast with an
    // synchronizing multisync
    //----------------------------------------------------------------------------
    void ShmemMcast2MetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "ShmemMultiCast2Composite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite2,
    ShmemMcast2MetaData,
    CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > ShmemMultiCast2Factory;

    //----------------------------------------------------------------------------
    // Shmem allsided multicast built on multicombine (BOR)
    //----------------------------------------------------------------------------
    void ShmemMcast3MetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "ShmemMultiCast3Composite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite3,
    ShmemMcast3MetaData,
    CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > ShmemMultiCast3Factory;

    //----------------------------------------------------------------------------
    // MU allsided multisync
    //----------------------------------------------------------------------------
    void MUMsyncMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "MUMultiSyncComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite,
    MUMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > MUMultiSyncFactory;

    //----------------------------------------------------------------------------
    // MU allsided multicast
    //----------------------------------------------------------------------------
    void MUMcombMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "MUMultiCombComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite,
    MUMcombMetaData,
    CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > MUMultiCombineFactory;

    //----------------------------------------------------------------------------
    // MU allsided multicast
    //----------------------------------------------------------------------------
    void MUMcastMetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "MUMultiCastComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite,
    MUMcastMetaData,
    CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > MUMultiCastFactory;

    //----------------------------------------------------------------------------
    // MU allsided multicast built on active message multicast with an
    // synchronizing multisync
    //----------------------------------------------------------------------------
    void MUMcast2MetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "MUMultiCast2Composite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite2,
    MUMcast2MetaData,
    CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > MUMultiCast2Factory;

    //----------------------------------------------------------------------------
    // MU allsided multicast built on multicombine (BOR)
    //----------------------------------------------------------------------------
    void MUMcast3MetaData(pami_metadata_t *m)
    {
//      pami_ca_set(&(m->geometry), 0);
      pami_ca_set(&(m->buffer), 0);
      pami_ca_set(&(m->misc), 0);
//      pami_ca_set(&(m->op[PAMI_BAND]), PAMI_SIGNED_CHAR);
      strncpy(&m->name[0], "MUMultiCast3Composite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite3,
    MUMcast3MetaData,
    CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > MUMultiCast3Factory;

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
            _shmem_barrier_composite(NULL),
            _mu_barrier_composite(NULL),
            _shmem_ni(shmem_ni),
            _shmem_msync_factory(&_sconnmgr, _shmem_ni),
            _shmem_mcast_factory(&_sconnmgr, _shmem_ni),
            _shmem_mcast2_factory(&_sconnmgr, _shmem_ni),
            _shmem_mcast3_factory(&_sconnmgr, _shmem_ni),
            _shmem_mcomb_factory(&_sconnmgr, _shmem_ni),
            _mu_ni(mu_ni),
            _mu_msync_factory(&_sconnmgr, _mu_ni),
            _mu_mcast_factory(&_sconnmgr, _mu_ni),
            _mu_mcast2_factory(&_sconnmgr, _mu_ni),
            _mu_mcast3_factory(&_sconnmgr, _mu_ni),
            _mu_mcomb_factory(&_sconnmgr, _mu_ni)
        {
          TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
          DO_DEBUG((templateName<T_Geometry>()));
          DO_DEBUG((templateName<T_ShmemNativeInterface>()));
          DO_DEBUG((templateName<T_MUNativeInterface>()));

          //set the mapid functions
          if ((__global.useshmem()) && (__global.topology_local.size() > 1))
            _shmem_msync_factory.setMapIdToGeometry(mapidtogeometry);

          if (__global.useMU())
            _mu_msync_factory.setMapIdToGeometry(mapidtogeometry);
        }

        inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry)
        {
          TRACE_ERR((stderr, "<%p>%s context_id %zu, geometry %p, msync %p, mcast %p, mcomb %p\n", this, __PRETTY_FUNCTION__, context_id, geometry, &_shmem_msync_factory, &_shmem_mcast_factory, &_shmem_mcomb_factory));
          pami_xfer_t xfer = {0};

          if ((__global.useshmem()) && (__global.topology_local.size() > 1))
            {
              TRACE_ERR((stderr, "<%p> Shemem barrier composites\n", this));
              _shmem_barrier_composite = _shmem_msync_factory.generate(geometry, &xfer);

              geometry->setKey(PAMI::Geometry::PAMI_GKEY_BARRIERCOMPOSITE1,
                               (void*)_shmem_barrier_composite);

              geometry->setKey(PAMI::Geometry::PAMI_GKEY_LOCALBARRIECOMPOSITE,
                               (void*)_shmem_barrier_composite);
            }
          else if (__global.useMU())
            {
              TRACE_ERR((stderr, "<%p> MU barrier composites\n", this));
              _mu_barrier_composite = _mu_msync_factory.generate(geometry, &xfer);

              geometry->setKey(PAMI::Geometry::PAMI_GKEY_BARRIERCOMPOSITE1,
                               (void*)_mu_barrier_composite);

//        geometry->setKey(PAMI::Geometry::PAMI_GKEY_LOCALBARRIECOMPOSITE,
//                         (void*)_shmem_barrier_composite);
            }

          //        else PAMI_abortf("No Shmem or MU?\n");

          if ((__global.useshmem()) && (__global.topology_local.size() > 1))
            {
              TRACE_ERR((stderr, "<%p> Register Shmem factories\n", this));
              // Add Barriers
              geometry->addCollective(PAMI_XFER_BARRIER, &_shmem_msync_factory, _context_id);

              // Add Broadcasts
              geometry->addCollective(PAMI_XFER_BROADCAST, &_shmem_mcast_factory, _context_id);
              //geometry->addCollective(PAMI_XFER_BROADCAST,&_shmem_mcast2_factory,_context_id);
              geometry->addCollective(PAMI_XFER_BROADCAST, &_shmem_mcast3_factory, _context_id);

              // Add Allreduces
              geometry->addCollective(PAMI_XFER_ALLREDUCE, &_shmem_mcomb_factory, _context_id);
            }

          if (__global.useMU())
            {
              TRACE_ERR((stderr, "<%p> Register MU factories\n", this));
              // Add Barriers
              geometry->addCollective(PAMI_XFER_BARRIER, &_mu_msync_factory, _context_id);

              // Add Broadcasts
              //geometry->addCollective(PAMI_XFER_BROADCAST,&_mu_mcast_factory,_context_id);
              geometry->addCollective(PAMI_XFER_BROADCAST, &_mu_mcast3_factory, _context_id);
              geometry->addCollective(PAMI_XFER_BROADCAST, &_mu_mcast2_factory, _context_id);

              // Add Allreduces
              geometry->addCollective(PAMI_XFER_ALLREDUCE, &_mu_mcomb_factory, _context_id);
            }

          return PAMI_SUCCESS;
        }

        static pami_geometry_t mapidtogeometry (int comm)
        {
          pami_geometry_t g = geometry_map[comm];
          TRACE_ERR((stderr, "<%p>%s\n", g, __PRETTY_FUNCTION__));
          return g;
        }

      public:
        pami_client_t                                   _client;
        pami_context_t                                  _context;
        size_t                                          _context_id;

        // CCMI Connection Manager Class
        CCMI::ConnectionManager::SimpleConnMgr<SysDep>  _sconnmgr;

        // Barrier Storage
        CCMI::Executor::Composite                      *_shmem_barrier_composite;
        CCMI::Executor::Composite                      *_mu_barrier_composite;

        //* SHMEM interfaces:
        // Native Interface
        T_ShmemNativeInterface                         *_shmem_ni;

        // CCMI Barrier Interface
        ShmemMultiSyncFactory                           _shmem_msync_factory;

        // CCMI Broadcast Interfaces
        ShmemMultiCastFactory                           _shmem_mcast_factory;
        ShmemMultiCast2Factory                          _shmem_mcast2_factory;
        ShmemMultiCast3Factory                          _shmem_mcast3_factory;

        // CCMI Allreduce Interface
        ShmemMultiCombineFactory                        _shmem_mcomb_factory;

        //* MU interfaces
        // Native Interface
        T_MUNativeInterface                            *_mu_ni;

        // CCMI Barrier Interface
        MUMultiSyncFactory                              _mu_msync_factory;

        // CCMI Broadcast Interfaces
        MUMultiCastFactory                              _mu_mcast_factory;
        MUMultiCast2Factory                             _mu_mcast2_factory;
        MUMultiCast3Factory                             _mu_mcast3_factory;

        // CCMI Allreduce Interface
        MUMultiCombineFactory                           _mu_mcomb_factory;
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
