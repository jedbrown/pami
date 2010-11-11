/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/BGQ2DRegistration.h
 * \brief Collective Registration for BGQ Converged 2 Device protocols
 */

#ifndef __algorithms_geometry_BGQ2DRegistration_h__
#define __algorithms_geometry_BGQ2DRegistration_h__

#include <map>
#include <vector>
#include "algorithms/interfaces/NativeInterface.h"
#include "algorithms/protocols/barrier/MultiSyncComposite.h"
#include "algorithms/protocols/broadcast/MultiCastComposite.h"
#include "algorithms/protocols/allreduce/MultiCombineComposite.h"
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"


namespace PAMI
{
  namespace CollRegistration
  {
    namespace BGQ2D
    {
      //  **********************************************************************
      //  Typedefs for template instantiations
      //  **********************************************************************
      namespace Barrier
      {
        void MsyncMetaData(pami_metadata_t *m)
        {
          strncpy(&m->name[0], "MultiSync2DeviceConverged", 32);
        }
        typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
        MsyncMetaData,
        CCMI::ConnectionManager::SimpleConnMgr>
        MultiSyncFactory;
      };

      namespace Broadcast
      {
        void McastMetaData(pami_metadata_t *m)
        {
          strncpy(&m->name[0], "MultiCast2DeviceConverged", 32);
        }
        typedef CCMI::Adaptor::Broadcast::MultiCastComposite2DeviceFactoryT
        < CCMI::Adaptor::Broadcast::MultiCastComposite2Device<PAMI_GEOMETRY_CLASS, true>,
        McastMetaData,
        CCMI::ConnectionManager::SimpleConnMgr>
        MultiCastFactory;
      };

      namespace Allreduce
      {
        void McombineMetaData(pami_metadata_t *m)
        {
          strncpy(&m->name[0], "MultiCombine2DeviceConverged", 32);
        }
        typedef CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2Device,
        McombineMetaData,
        CCMI::ConnectionManager::SimpleConnMgr>
        MultiCombineFactory;

        //----------------------------------------------------------------------------
        // 'Composite' Shmem/MU allsided 2 device multicombine with no pipelining
        //----------------------------------------------------------------------------
        void Mcomb2DMetaDataNP(pami_metadata_t *m)
        {
          strncpy(&m->name[0], "MultiCombine2DeviceNPConverged", 32);
        }
        typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceNP,
          Mcomb2DMetaDataNP, CCMI::ConnectionManager::SimpleConnMgr > MultiCombineFactoryNP;

      };
      //  **********************************************************************
      //  End:  Typedefs for template instantiations
      //  **********************************************************************

      template <class T_Geometry,
      class T_Local_Device,
      class T_Global_Device,
      class T_LocalNI_AM,
      class T_GlobalNI_AM,
      class T_LocalModel,
      class T_CSMemoryManager>
      class BGQ2DRegistration :
      public CollRegistration < PAMI::CollRegistration::BGQ2D::BGQ2DRegistration < T_Geometry,
      T_Local_Device,
      T_Global_Device,
      T_LocalNI_AM,
      T_GlobalNI_AM,
      T_LocalModel,
      T_CSMemoryManager>,
      T_Geometry >
      {

        typedef struct Factories
        {
          char barrier_blob[sizeof(Barrier::MultiSyncFactory)];
          char broadcast_blob[sizeof(Broadcast::MultiCastFactory)];
          char allreduce_blob[sizeof(Allreduce::MultiCombineFactory)];
          char allreducenp_blob[sizeof(Allreduce::MultiCombineFactoryNP)];
        }Factories;

        typedef struct GeometryInfo
        {
          // Currently, the _niPtr array is in front
          // These have to go at the front of the struct
          // because the protocols use the key.
          CCMI::Interfaces::NativeInterface *_niPtr[4];
          Barrier::MultiSyncFactory         *_barrier;
          Broadcast::MultiCastFactory       *_broadcast;
          Allreduce::MultiCombineFactory    *_allreduce;
          Allreduce::MultiCombineFactoryNP  *_allreducenp;
          T_LocalModel                      *_local_model;
        }GeometryInfo;



      public:
        inline BGQ2DRegistration(pami_client_t                        client,
                                 pami_context_t                       context,
                                 size_t                               context_id,
                                 size_t                               client_id,
                                 T_Local_Device                      &ldev,
                                 T_Global_Device                     &gdev,
                                 Mapping                             &mapping,
                                 int                                 *dispatch_id,
                                 std::map<unsigned, pami_geometry_t> *geometry_map):
        CollRegistration < PAMI::CollRegistration::BGQ2D::BGQ2DRegistration < T_Geometry,
        T_Local_Device,
        T_Global_Device,
        T_LocalNI_AM,
        T_GlobalNI_AM,
        T_LocalModel,
        T_CSMemoryManager>,
        T_Geometry > (),
        _client(client),
        _context(context),
        _context_id(context_id),
        _client_id(client_id),
        _geometry_map(geometry_map),
        _dispatch_id(dispatch_id),
        _global_task(mapping.task()),
        _global_size(mapping.size()),
        _reduce_val((-1)&(~0x1)),
        _local_devs(ldev),
        _global_dev(gdev),
        _g_barrier_ni(_global_dev,client, context, context_id, client_id,dispatch_id),
        _g_broadcast_ni(_global_dev,client, context, context_id, client_id,dispatch_id),
        _g_allreduce_ni(_global_dev,client, context, context_id, client_id,dispatch_id),
        _g_allreducenp_ni(_global_dev,client, context, context_id, client_id,dispatch_id)
        {
          TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
          // To initialize shared memory, we need to provide the task offset into the
          // local nodes, and the total number of nodes we have locally
          size_t                         peer;
          size_t                         numpeers;
          mapping.task2peer(_global_task, peer);
          mapping.nodePeers(numpeers);
          TRACE_ERR((stderr, "<%p>BGQ2DRegistration() task %zu, peer %zu, numpeers %zu\n", this, task, peer, numpeers));
          _csmm.init(peer,numpeers);
        }

        inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
        {
          TRACE_ERR((stderr, "<%p>BGQ2DRegistration::analyze_impl() context %zu, geometry %p, phase %u\n", this, context_id, geometry, phase));

          if (phase != 1) return PAMI_SUCCESS; // only after analyze_global \todo clean this up

          // only support single context for now
          if (context_id != 0) return PAMI_SUCCESS;

          // Get the topology for the local nodes
          // and the topology for the "distributed masters" for the global communication
          PAMI::Topology *local_topo        = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX));
          PAMI::Topology *local_master_topo = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX));
          PAMI_assert(local_topo->size() != 0);
          PAMI_assert(local_master_topo->size() != 0);

          void *ctrlstr  = (void *) geometry->getKey(PAMI::Geometry::GKEY_GEOMETRYCSNI);


          // Allocate the local models
          T_LocalModel                     *local_model  = (T_LocalModel*)_model_allocator.allocateObject();
          new(local_model)T_LocalModel(&_local_devs, geometry->comm(), local_topo, &_csmm, ctrlstr);

          // Allocate the local native interface
          T_LocalNI_AM                     *ni           = (T_LocalNI_AM*)_ni_allocator.allocateObject();
          new(ni)T_LocalNI_AM(*local_model,
                              _client,
                              _client_id,
                              _context,
                              _context_id,
                              local_topo->rank2Index(_global_task),
                              local_topo->size())

          // Construct the geometry info object, so we can free our allocated objects later
          GeometryInfo                     *geometryInfo = (GeometryInfo*)_geom_allocator.allocateObject();
          geometryInfo->_local_model                     = local_model;
          geometryInfo->_niPtr[0]                        = ni;
          geometryInfo->_niPtr[1]                        = &_g_barrier_ni;
          geometryInfo->_niPtr[2]                        = ni;
          geometryInfo->_niPtr[3]                        = &_g_allreducenp_ni;
        
          // Allocate the factories
          //  ----->  Barrier
          Barrier::MultiSyncFactory  *barrier_reg        = (Barrier::MultiSyncFactory*)_factory_allocator.allocateObject();
          new(barrier_reg) Barrier::MultiSyncFactory(&_sconnmgr,
                                                     (CCMI::Interfaces::NativeInterface *)&geometryInfo->_niPtr[0]);
          geometryInfo->_barrier                         = barrier_reg;

          //  ----->  Broadcast
          Broadcast::MultiCastFactory  *broadcast_reg    = (Broadcast::MultiCastFactory*)_factory_allocator.allocateObject();
          new(broadcast_reg) Broadcast::MultiCastFactory(&_sconnmgr,
                                                         (CCMI::Interfaces::NativeInterface *)ni,
                                                         false,  // local protocols are not active message (2 sided), will not register
                                                         (CCMI::Interfaces::NativeInterface *)&_g_broadcast_ni,
                                                         true);  // global protocols ARE active message, will register async
          geometryInfo->_broadcast                       = broadcast_reg;

          //  ----->  Allreduce
          Allreduce::MultiCombineFactory  *allreduce_reg = (Allreduce::MultiCombineFactory*)_factory_allocator.allocateObject();
          new(allreduce_reg) Allreduce::MultiCombineFactory(&_sconnmgr,
                                                            (CCMI::Interfaces::NativeInterface *)ni,
                                                            (CCMI::Interfaces::NativeInterface *)&_g_allreduce_ni);
          geometryInfo->_allreduce                       = allreduce_reg;

          //  ----->  Allreduce
          Allreduce::MultiCombineFactoryNP  *allreducenp_reg = (Allreduce::MultiCombineFactoryNP*)_factory_allocator.allocateObject();
          new(allreducenp_reg) Allreduce::MultiCombineFactoryNP(&_sconnmgr,
                                                                (CCMI::Interfaces::NativeInterface *)&geometryInfo->_niPtr[2]);
          geometryInfo->_allreducenp                     = allreducenp_reg;

          // Add the geometry info to the geometry
          geometry->setKey(PAMI::Geometry::GKEY_GEOMETRYCSNI, ni);
          geometry->addCollective(PAMI_XFER_BARRIER,barrier_reg,context_id);
          geometry->addCollective(PAMI_XFER_BROADCAST,broadcast_reg,context_id);
          geometry->addCollective(PAMI_XFER_ALLREDUCE,allreducenp_reg,context_id);
          geometry->addCollective(PAMI_XFER_ALLREDUCE,allreduce_reg,context_id);

          // Todo:  free the ginfo;
          return PAMI_SUCCESS;
        }

        inline pami_result_t analyze_local_impl(size_t context_id,T_Geometry *geometry, uint64_t *out)
        {
          TRACE_ERR((stderr, "<%p>BGQ2DRegistration::analyze_local_impl() context %zu, geometry %p, out %p\n", this, context_id, geometry, out));
          // only support single context for now
          if (context_id != 0) return PAMI_SUCCESS;

          // This is where we define our contribution to the allreduce
           _csmm.getSGCtrlStrVec(geometry, out);
           return PAMI_SUCCESS;
        }

        inline pami_result_t analyze_global_impl(size_t context_id,T_Geometry *geometry, uint64_t *in)
        {
          TRACE_ERR((stderr, "<%p>BGQ2DRegistration::analyze_global_impl() context %zu, geometry %p, in %p\n", this, context_id, geometry, in));
          // only support single context for now
          if (context_id != 0) return PAMI_SUCCESS;

          // This is where we get our reduction result back from the geometry create operation
          PAMI::Topology *local_master_topo = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX));
          PAMI::Topology *local_topo        = (PAMI::Topology *)geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX);

          uint master_rank   = local_topo->index2Rank(0);
          uint master_index  = local_master_topo->rank2Index(master_rank);
          void *ctrlstr      = (void *)in[master_index];
          if (ctrlstr == NULL) ctrlstr = (void *)_csmm.getWGCtrlStr();

          geometry->setKey(PAMI::Geometry::GKEY_GEOMETRYCSNI, ctrlstr);

          // Complete the final analysis and population of the geometry structure
          // with the algorithm list
          return analyze(context_id, geometry, 1);
        }

        static pami_geometry_t mapidtogeometry (int comm)
        {
          pami_geometry_t g = (*_geometry_map)[comm];
          return g;
        }
      private:
        // Client, Context, and Utility variables
        pami_client_t                                                   _client;
        pami_context_t                                                  _context;
        size_t                                                          _context_id;
        size_t                                                          _client_id;
        std::map<unsigned, pami_geometry_t>                            *_geometry_map;

        // This is a pointer to the current dispatch id of the context
        // This will be decremented by the ConstructNativeInterface routines
        int                                                            *_dispatch_id;

        pami_task_t                                                     _global_task;
        size_t                                                          _global_size;
        uint64_t                                                        _reduce_val;

        // Connection Manager
        CCMI::ConnectionManager::SimpleConnMgr                          _sconnmgr;

        // Devices
        T_Local_Device                                                 &_local_devs;
        T_Global_Device                                                &_global_dev;

        // Global native interface
        T_GlobalNI_AM                                                   _g_barrier_ni;
        T_GlobalNI_AM                                                   _g_broadcast_ni;
        T_GlobalNI_AM                                                   _g_allreduce_ni;
        T_GlobalNI_AM                                                   _g_allreducenp_ni;

        // Factory Allocator
        // and Local NI allocator
        PAMI::MemoryAllocator<sizeof(Factories),16>                     _factory_allocator;
        PAMI::MemoryAllocator<sizeof(T_LocalModel),16>                  _model_allocator;
        PAMI::MemoryAllocator<sizeof(T_LocalNI_AM),16>                  _ni_allocator;
        PAMI::MemoryAllocator<sizeof(GeometryInfo),16>                  _geom_allocator;

        // Collective shared memory manager
        T_CSMemoryManager                                               _csmm;
      };
    }; // BGQ2D
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
