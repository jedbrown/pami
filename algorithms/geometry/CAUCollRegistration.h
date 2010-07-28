/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/CAUCollRegistration.h
 * \brief ???
 */

#ifndef __algorithms_geometry_CAUCollRegistration_h__
#define __algorithms_geometry_CAUCollRegistration_h__

#include <map>
#include <vector>
#include "algorithms/interfaces/NativeInterface.h"
#include "algorithms/protocols/barrier/MultiSyncComposite.h"
#include "common/lapiunix/lapifunc.h"


// Collective Registration for CAU protocols for p2p
namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;

  namespace CollRegistration
  {
    namespace CAU
    {
      namespace Barrier
      {
        void MsyncMetaData(pami_metadata_t *m)
        {
          strncpy(&m->name[0], "CAU MultiSyncComposite", 32);
        }
        typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
                                                                    MsyncMetaData,
                                                                    CCMI::ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> >
        MultiSyncFactory;
      };
      template <class T_Geometry,
                class T_Local_Device,
                class T_Global_Device,
                class T_LocalNI_AM,
                class T_GlobalNI_AM,
                class T_LocalModel,
                class T_CSMemoryManager>
      class CAURegistration :
        public CollRegistration < PAMI::CollRegistration::CAU::CAURegistration < T_Geometry,
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
        }Factories;

        typedef struct GeometryInfo
        {
          // Currently, the _niPtr array is in front
          // These have to go at the front of the struct
          // because the protocols use the key.
          CCMI::Interfaces::NativeInterface *_niPtr[2];
          Barrier::MultiSyncFactory         *_barrier;
          T_LocalModel                      *_local_model;
          T_LocalNI_AM                      *_ni;
        }GeometryInfo;



      public:
        inline CAURegistration(pami_client_t       client,
                               pami_context_t      context,
                               size_t              context_id,
                               size_t              client_id,
                               T_Local_Device     &ldev,
                               T_Global_Device    &gdev,
                               Mapping            &mapping,
                               lapi_handle_t       lapi_handle):
          CollRegistration < PAMI::CollRegistration::CAU::CAURegistration < T_Geometry,
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
          _global_task(mapping.task()),
          _global_size(mapping.size()),
          _lapi_handle(lapi_handle),
          _reduce_val((-1)&(~0x1)),
          _local_devs(ldev),
          _global_dev(gdev),
          _global_ni(_global_dev,
                     client,
                     context,
                     context_id,
                     client_id,
                     _global_task,
                     _global_size)
          {
            // To initialize shared memory, we need to provide the task offset into the
            // local nodes, and the total number of nodes we have locally
            size_t                         peer;
            size_t                         numpeers;
            mapping.task2peer(_global_task, peer);
            mapping.nodePeers(numpeers);
            _csmm.init(peer,numpeers);
          }


        inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
          {
            PAMI_assert(context_id == 0);

            // Get the topology for the local nodes
            // and the topology for the "distributed masters" for the global communication
            PAMI::Topology *local_topo        = (PAMI::Topology *) (geometry->getLocalTopology());
            PAMI::Topology *local_master_topo = (PAMI::Topology *) (geometry->getLocalMasterTopology());
            PAMI_assert(local_topo->size() != 0);
            PAMI_assert(local_master_topo->size() != 0);

            // Allocate the local models
            T_LocalModel                     *local_model  = (T_LocalModel*)_model_allocator.allocateObject();
            new(local_model)T_LocalModel(&_local_devs, geometry->comm(), local_topo, &_csmm);

            // Allocate the local native interface
            T_LocalNI_AM                     *ni           = (T_LocalNI_AM*)_ni_allocator.allocateObject();
            new(ni)T_LocalNI_AM(*local_model,
                                _client,
                                _client_id,
                                _context,
                                _context_id,
                                local_topo->rank2Index(_global_task),
                                local_topo->size());

            // Construct the geometry info object, so we can free our allocated objects later
            GeometryInfo                     *geometryInfo = (GeometryInfo*)_geom_allocator.allocateObject();
            geometryInfo->_local_model                     = local_model;
            geometryInfo->_ni                              = ni;
            geometryInfo->_niPtr[0]                        = ni;
            geometryInfo->_niPtr[1]                        = &_global_ni;

            // Allocate the factories
            Barrier::MultiSyncFactory  *barrier_reg        = (Barrier::MultiSyncFactory*)_factory_allocator.allocateObject();
            new(barrier_reg) Barrier::MultiSyncFactory(&_sconnmgr, (CCMI::Interfaces::NativeInterface *)&geometryInfo->_niPtr[0]);
            geometryInfo->_barrier                         = barrier_reg;

            // Add the geometry info to the geometry
            geometry->setKey(PAMI::Geometry::PAMI_GKEY_GEOMETRYCSNI, ni);
            geometry->addCollective(PAMI_XFER_BARRIER,barrier_reg,context_id);

            // Todo:  free the ginfo;

            return PAMI_SUCCESS;
          }

        inline pami_result_t analyze_local_impl(size_t context_id,T_Geometry *geometry, uint64_t *out)
          {
            *out = _reduce_val;
            return PAMI_SUCCESS;
          }

        inline pami_result_t analyze_global_impl(size_t context_id,T_Geometry *geometry, uint64_t in)
          {
            int               x        = ffs(in)-1;
            uint64_t          key;
            if(x!=(int)0xFFFFFFFF)
              {
                _reduce_val         &=(~(0x1<<x));
                 key                 = x;
              }
            else
              key = -1;

            pami_task_t *rl = NULL;
            PAMI::Topology *local_master_topo = (PAMI::Topology *) (geometry->getLocalMasterTopology());
            pami_result_t   rc                = local_master_topo->rankList(&rl);
            uint            num_tasks         = local_master_topo->size();
            uint           *tasks             = (uint*)rl;
            uint            groupid           = (uint)key;
            int             myrc              = -1;
            bool            participant       = geometry->isLocalMasterParticipant();
            if(num_tasks>1 && participant)
              {
                myrc = lapi_cau_group_create(_lapi_handle,groupid,num_tasks,tasks);
                if(myrc != LAPI_SUCCESS)
                  {
                    // Don't insert any algorithms
                    return PAMI_SUCCESS;
                  }
              }
            PAMI::Device::CAUGeometryInfo *gi
              = (PAMI::Device::CAUGeometryInfo *)_cau_geom_allocator.allocateObject();

            new(gi)PAMI::Device::CAUGeometryInfo(key,
                                                 geometry->comm(),
                                                 local_master_topo);

            geometry->setKey(Geometry::PAMI_GKEY_CLASSROUTEID,gi);

            return analyze(context_id, geometry, 0);
          }

        static pami_geometry_t mapidtogeometry (int comm)
          {
            pami_geometry_t g = geometry_map[comm];
            return g;
          }
      private:
        // Client, Context, and Utility variables
        pami_client_t                                                   _client;
        pami_context_t                                                  _context;
        size_t                                                          _context_id;
        size_t                                                          _client_id;
        pami_task_t                                                     _global_task;
        size_t                                                          _global_size;
        lapi_handle_t                                                   _lapi_handle;
        uint64_t                                                        _reduce_val;

        // Connection Manager
        CCMI::ConnectionManager::SimpleConnMgr<SysDep>                  _sconnmgr;

        // Devices
        T_Local_Device                                                 &_local_devs;
        T_Global_Device                                                &_global_dev;

        // Global native interface
        T_GlobalNI_AM                                                   _global_ni;

        // Factory Allocator
        // and Local NI allocator
        PAMI::MemoryAllocator<sizeof(Factories),16>                     _factory_allocator;
        PAMI::MemoryAllocator<sizeof(T_LocalModel),16>                  _model_allocator;
        PAMI::MemoryAllocator<sizeof(T_LocalNI_AM),16>                  _ni_allocator;
        PAMI::MemoryAllocator<sizeof(GeometryInfo),16>                  _geom_allocator;
        PAMI::MemoryAllocator<sizeof(PAMI::Device::CAUGeometryInfo),16> _cau_geom_allocator;
        // Collective shared memory manager
        T_CSMemoryManager                                               _csmm;
      };
    }; // CAU
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
