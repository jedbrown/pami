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
#include "algorithms/geometry/Metadata.h"
#include "algorithms/protocols/barrier/BarrierT.h"
#include "algorithms/protocols/barrier/MultiSyncComposite.h"
#include "algorithms/protocols/broadcast/MultiCastComposite.h"
#include "algorithms/protocols/allreduce/MultiCombineComposite.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"
#include "common/lapiunix/lapifunc.h"

// Collective Registration for CAU protocols for p2p
namespace PAMI
{
  namespace CollRegistration
  {
    namespace CAU
    {

      inline metadata_result_t op_dt_metadata_function(struct pami_xfer_t *in)
        {
          const bool support[PAMI_DT_COUNT][PAMI_OP_COUNT] =
          {
        //  PAMI_UNDEFINED_OP, PAMI_NOOP, PAMI_MAX, PAMI_MIN, PAMI_SUM, PAMI_PROD, PAMI_LAND, PAMI_LOR, PAMI_LXOR, PAMI_BAND, PAMI_BOR, PAMI_BXOR, PAMI_MAXLOC, PAMI_MINLOC, PAMI_USERDEFINED_OP,
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_UNDEFINED_DT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_SIGNED_CHAR
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_UNSIGNED_CHAR
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_SIGNED_SHORT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_UNSIGNED_SHORT
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     true,      true,     true,      false,       false,       false},//PAMI_SIGNED_INT
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     true,      true,     true,      false,       false,       false},//PAMI_UNSIGNED_INT
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     true,      true,     true,      false,       false,       false},//PAMI_SIGNED_LONG_LONG
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     true,      true,     true,      false,       false,       false},//PAMI_UNSIGNED_LONG_LONG
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_FLOAT
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_DOUBLE
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LONG_DOUBLE
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOGICAL
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_SINGLE_COMPLEX
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_DOUBLE_COMPLEX
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_2INT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_SHORT_INT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_FLOAT_INT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_DOUBLE_INT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_2FLOAT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_2DOUBLE
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false} //PAMI_USERDEFINED_DT
          };
          metadata_result_t result = {0};
          result.check.datatype = support[in->cmd.xfer_allreduce.dt][in->cmd.xfer_allreduce.op]?0:1;
          return(result);
        }
      
    
      //  **********************************************************************
      //  Typedefs for template instantiations
      //  **********************************************************************
      namespace Barrier
      {
        void MsyncMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:MultiSyncComposite:SHMEM:CAU");
        }
        typedef CCMI::Adaptor::Barrier::BarrierFactory2DeviceMsync < CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
                                                                     MsyncMetaData,
                                                                     CCMI::ConnectionManager::SimpleConnMgr>
        MultiSyncFactory;

        void MsyncBSRMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:MultiSyncComposite:BSR:CAU");
        }
        typedef CCMI::Adaptor::Barrier::BarrierFactory2DeviceMsync < CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
                                                                     MsyncBSRMetaData,
                                                                     CCMI::ConnectionManager::SimpleConnMgr>
        MultiSyncBSRFactory;


      };

      namespace Broadcast
      {
        void McastMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:MultiCastComposite:SHMEM:CAU");
        }
        typedef CCMI::Adaptor::Broadcast::MultiCastComposite2DeviceFactoryT
        < CCMI::Adaptor::Broadcast::MultiCastComposite2Device<PAMI_GEOMETRY_CLASS>,
          McastMetaData,
          CCMI::ConnectionManager::SimpleConnMgr>
        MultiCastFactory;
      };

      namespace Allreduce
      {
        void McombineMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:MultiCombineComposite:SHMEM:CAU");
          m->check_correct.values.alldt     = 0;
          m->check_correct.values.allop     = 0;
          m->check_fn                       = CAU::op_dt_metadata_function;
          m->check_perf.values.hw_accel     = 1;
          m->range_lo_perf                  = 0;
          m->range_hi_perf                  = 64;          
        }
        typedef CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2Device<0>,
                                                                                 McombineMetaData,
                                                                                 CCMI::ConnectionManager::SimpleConnMgr>

        MultiCombineFactory;
      };
      namespace Reduce
      {
        void McombineMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:MultiCombineComposite:SHMEM:CAU");
          m->check_correct.values.alldt     = 0;
          m->check_correct.values.allop     = 0;
          m->check_fn                       = CAU::op_dt_metadata_function;
          m->check_perf.values.hw_accel     = 1;
          m->range_lo_perf                  = 0;
          m->range_hi_perf                  = 64;          
        }
        typedef CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2Device<1>,
                                                                                 McombineMetaData,
                                                                                 CCMI::ConnectionManager::SimpleConnMgr>
        MultiCombineFactory;
      };


      
      //  **********************************************************************
      //  End:  Typedefs for template instantiations
      //  **********************************************************************

      template <class T_Geometry,
                class T_Local_Device,
                class T_Local_DeviceBSR,
                class T_Global_Device,
                class T_LocalNI_AM,
                class T_GlobalNI_AM,
                class T_LocalBSRNI,
                class T_LocalModel,
                class T_CSMemoryManager>
      class CAURegistration :
        public CollRegistration < PAMI::CollRegistration::CAU::CAURegistration < T_Geometry,
                                                                                 T_Local_Device,
                                                                                 T_Local_DeviceBSR,
                                                                                 T_Global_Device,
                                                                                 T_LocalNI_AM,
                                                                                 T_GlobalNI_AM,
                                                                                 T_LocalBSRNI,
                                                                                 T_LocalModel,
                                                                                 T_CSMemoryManager>,
                                  T_Geometry >
      {

        typedef struct Factories
        {
          char barrier_blob[sizeof(Barrier::MultiSyncFactory)];
          char barrierbsr_blob[sizeof(Barrier::MultiSyncBSRFactory)];
          char broadcast_blob[sizeof(Broadcast::MultiCastFactory)];
          char allreduce_blob[sizeof(Allreduce::MultiCombineFactory)];
          char reduce_blob[sizeof(Reduce::MultiCombineFactory)];
        }Factories;

        typedef struct GeometryInfo
        {
          // Currently, the _niPtr array is in front
          // These have to go at the front of the struct
          // because the protocols use the key.
          CCMI::Interfaces::NativeInterface *_niPtr[6];
          Barrier::MultiSyncFactory         *_barrier;
          Barrier::MultiSyncBSRFactory      *_barrierbsr;
          Broadcast::MultiCastFactory       *_broadcast;
          Allreduce::MultiCombineFactory    *_allreduce;
          Reduce::MultiCombineFactory       *_reduce;

          // We will want to free these models later
          // So keep them in the geometryinfo for now
          T_LocalModel                      *_local_model;
          T_LocalNI_AM                      *_ni;
        }GeometryInfo;



      public:
        inline CAURegistration(pami_client_t                        client,
                               pami_context_t                       context,
                               size_t                               context_id,
                               size_t                               client_id,
                               T_Local_Device                      &ldev,
                               T_Local_DeviceBSR                   &ldevbsr,
                               T_Global_Device                     &gdev,
                               Mapping                             &mapping,
                               lapi_handle_t                        lapi_handle,
                               int                                 *dispatch_id,
                               std::map<unsigned, pami_geometry_t> *geometry_map,
                               Memory::MemoryManager               *mm):
          CollRegistration < PAMI::CollRegistration::CAU::CAURegistration < T_Geometry,
                                                                            T_Local_Device,
                                                                            T_Local_DeviceBSR,
                                                                            T_Global_Device,
                                                                            T_LocalNI_AM,
                                                                            T_GlobalNI_AM,
                                                                            T_LocalBSRNI,
                                                                            T_LocalModel,
                                                                            T_CSMemoryManager>,
                             T_Geometry > (),
          _client(client),
          _context(context),
          _context_id(context_id),
          _client_id(client_id),
          _dispatch_id(dispatch_id),
          _geometry_map(geometry_map),
          _global_task(mapping.task()),
          _global_size(mapping.size()),
          _lapi_handle(lapi_handle),
          _reduce_val((-1)&(~0x1)),
          _local_devs(ldev),
          _local_devs_bsr(ldevbsr),
          _global_dev(gdev),
          _g_barrier_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _l_barrierbsr_ni(_local_devs_bsr,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _g_broadcast_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _g_allreduce_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _g_reduce_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _csmm(mm)
          {
            if(getenv("MP_COLLECTIVE_GROUPS"))
              _enabled = true;
            else
              _enabled = false;
            
            if(!_enabled) return;

            if(_Lapi_env.use_mpi_shm == SHM_YES)
              _enabled = true;
            else
              _enabled = false;
            
            if(!_enabled) return;
            
            // To initialize shared memory, we need to provide the task offset into the
            // local nodes, and the total number of nodes we have locally
            size_t                         peer;
            size_t                         numpeers;
            mapping.task2peer(_global_task, peer);
            mapping.nodePeers(numpeers);
            pami_result_t rc = _csmm.init(peer,numpeers);
            PAMI_assertf(rc == PAMI_SUCCESS, "Collective shared memory allocation failed with error%d\n", rc);
          }

        inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
          {
            if(!_enabled) return PAMI_SUCCESS;
            PAMI_assert(context_id == 0);

            // Get the topology for the local nodes
            // and the topology for the "distributed masters" for the global communication
            PAMI::Topology *local_topo        = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX));
            PAMI::Topology *local_master_topo = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX));
            PAMI_assert(local_topo->size() != 0);
            PAMI_assert(local_master_topo->size() != 0);

            // Allocate the local models for shared memory
            T_LocalModel                     *local_model  = (T_LocalModel*)_model_allocator.allocateObject();
            void                             *csmm_ctrlstr = (void *) geometry->getKey(PAMI::Geometry::GKEY_GEOMETRYCSNI);
            new(local_model)T_LocalModel(&_local_devs, geometry->comm(), local_topo, &_csmm, csmm_ctrlstr);

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
            geometryInfo->_niPtr[1]                        = &_g_barrier_ni;
            geometryInfo->_niPtr[2]                        = &_l_barrierbsr_ni;
            geometryInfo->_niPtr[3]                        = &_g_broadcast_ni;
            geometryInfo->_niPtr[4]                        = &_g_allreduce_ni;
            geometryInfo->_niPtr[5]                        = &_g_reduce_ni;

            // Allocate the factories
            //  ----->  Barrier
            Barrier::MultiSyncFactory  *barrier_reg        = (Barrier::MultiSyncFactory*)_factory_allocator.allocateObject();
            new(barrier_reg) Barrier::MultiSyncFactory(&_sconnmgr,
                                                       (CCMI::Interfaces::NativeInterface *)geometryInfo->_niPtr[0],
                                                       (CCMI::Interfaces::NativeInterface *)geometryInfo->_niPtr[1]);
            geometryInfo->_barrier                         = barrier_reg;

            //  ----->  BarrierBSR
            Barrier::MultiSyncBSRFactory  *barrierbsr_reg  = (Barrier::MultiSyncBSRFactory*)_factory_allocator.allocateObject();
            new(barrierbsr_reg) Barrier::MultiSyncBSRFactory(&_sconnmgr,
                                                             (CCMI::Interfaces::NativeInterface *)geometryInfo->_niPtr[0],
                                                             (CCMI::Interfaces::NativeInterface *)geometryInfo->_niPtr[2]);
            geometryInfo->_barrierbsr                      = barrierbsr_reg;

            
            //  ----->  Broadcast
            Broadcast::MultiCastFactory  *broadcast_reg    = (Broadcast::MultiCastFactory*)_factory_allocator.allocateObject();
            new(broadcast_reg) Broadcast::MultiCastFactory(&_sconnmgr,
                                                           (CCMI::Interfaces::NativeInterface *)geometryInfo->_niPtr[0],
                                                           false,  // local protocols are not active message (2 sided), will not register
                                                           (CCMI::Interfaces::NativeInterface *)geometryInfo->_niPtr[1],
                                                           true);  // global protocols ARE active message, will register async
            geometryInfo->_broadcast                       = broadcast_reg;

            //  ----->  Allreduce
            Allreduce::MultiCombineFactory  *allreduce_reg = (Allreduce::MultiCombineFactory*)_factory_allocator.allocateObject();
            new(allreduce_reg) Allreduce::MultiCombineFactory(&_sconnmgr,
                                                              (CCMI::Interfaces::NativeInterface *)geometryInfo->_niPtr[0],
                                                              (CCMI::Interfaces::NativeInterface *)geometryInfo->_niPtr[3]);
            geometryInfo->_allreduce                       = allreduce_reg;

            //  ----->  Reduce
            Reduce::MultiCombineFactory  *reduce_reg = (Reduce::MultiCombineFactory*)_factory_allocator.allocateObject();
            new(reduce_reg) Reduce::MultiCombineFactory(&_sconnmgr,
                                                              (CCMI::Interfaces::NativeInterface *)geometryInfo->_niPtr[0],
                                                              (CCMI::Interfaces::NativeInterface *)geometryInfo->_niPtr[3]);
            geometryInfo->_reduce                          = reduce_reg;

            // Add the geometry info to the geometry
            geometry->setKey(PAMI::Geometry::GKEY_GEOMETRYCSNI, ni);
            geometry->addCollective(PAMI_XFER_BARRIER,barrier_reg,context_id);

            if(_local_devs_bsr.isInit())
              geometry->addCollective(PAMI_XFER_BARRIER,barrierbsr_reg,context_id);

            geometry->addCollective(PAMI_XFER_BROADCAST,broadcast_reg,context_id);
            geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE,allreduce_reg,context_id);
            geometry->addCollectiveCheck(PAMI_XFER_REDUCE,reduce_reg,context_id);

            // Todo:  free the ginfo;
            return PAMI_SUCCESS;
          }

        inline pami_result_t register_local_impl(size_t context_id,T_Geometry *geometry, uint64_t *out, int &n)
          {
            // This is where we define our contribution to the allreduce
            // In this case, we allreduce the mast of available class route id's, which we pack
            // into a single 64 bit integer.  Later, we may want to increase the size from one integer
            // to an array to be reduced (if more than 64 class routes are desired).
	    n = 1;
            *out = _reduce_val;
            if(!_enabled) return PAMI_SUCCESS;

            // prepare for collshmem device control structure address distribution
            _csmm.getSGCtrlStrVec(geometry, out+1);
            return PAMI_SUCCESS;
          }

        inline pami_result_t receive_global_impl(size_t context_id,T_Geometry *geometry, uint64_t *in, int n)
          {
            if(!_enabled) return PAMI_SUCCESS;

            // This is where we get our reduction result back from the geometry create operation
            // This bit should show the "highest" available mask of bits
            // This is the value we should use for our class route id
            int               x        = ffs(in[0])-1;
            uint64_t          key;
            if(x!=(int)0xFFFFFFFF)
              {
                _reduce_val         &=(~(0x1<<x));
                 key                 = x;
              }
            else
              key = -1;

            // Ask the geometry for the specific topologies we need
            // Parse the topologies to determine if we are going to call the group
            // create on the cau.
            pami_task_t *rl = NULL;
            PAMI::Topology *local_master_topo = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX));
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

            // This is where we construct the "device specific" information.
            // The definition of this class is specified in the CAU device.
            // We populate this device specific information into the geometry
            // The protocol will query for the device specific information,
            // and pass this into the M-* api during communication
            PAMI::Device::CAUGeometryInfo *gi
              = (PAMI::Device::CAUGeometryInfo *)_cau_geom_allocator.allocateObject();
            new(gi)PAMI::Device::CAUGeometryInfo(key,
                                                 geometry->comm(),
                                                 local_master_topo);
            geometry->setKey(Geometry::GKEY_MCAST_CLASSROUTEID,gi);
            geometry->setKey(Geometry::GKEY_MCOMB_CLASSROUTEID,gi);
            geometry->setKey(Geometry::GKEY_MSYNC_CLASSROUTEID,gi);

            uint master_rank   = ((PAMI::Topology *)geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX))->index2Rank(0);
            uint master_index  = local_master_topo->rank2Index(master_rank);
            void *ctrlstr      = (void *)in[master_index+1];
            if (ctrlstr == NULL)
              ctrlstr = _csmm.getWGCtrlStr();
            geometry->setKey(Geometry::GKEY_GEOMETRYCSNI,ctrlstr);

            // Complete the final analysis and population of the geometry structure
            // with the algorithm list
            return analyze(context_id, geometry, 0);
          }

      private:
        // Client, Context, and Utility variables
        pami_client_t                                                   _client;
        pami_context_t                                                  _context;
        size_t                                                          _context_id;
        size_t                                                          _client_id;
        int                                                            *_dispatch_id; 
        std::map<unsigned, pami_geometry_t>                            *_geometry_map;
        pami_task_t                                                     _global_task;
        size_t                                                          _global_size;
        lapi_handle_t                                                   _lapi_handle;
        uint64_t                                                        _reduce_val;
        bool                                                            _enabled;

        // Connection Manager
        CCMI::ConnectionManager::SimpleConnMgr                          _sconnmgr;

        // Devices
        T_Local_Device                                                 &_local_devs;
        T_Local_DeviceBSR                                              &_local_devs_bsr;
        T_Global_Device                                                &_global_dev;

        // Global native interface
        T_GlobalNI_AM                                                   _g_barrier_ni;
        T_GlobalNI_AM                                                   _g_broadcast_ni;
        T_GlobalNI_AM                                                   _g_allreduce_ni;
        T_GlobalNI_AM                                                   _g_reduce_ni;

        // BSR Device
        T_LocalBSRNI                                                    _l_barrierbsr_ni;

        // Factory Allocator
        // and Local NI allocator
        PAMI::MemoryAllocator<sizeof(Factories),16>                     _factory_allocator;
        PAMI::MemoryAllocator<sizeof(T_LocalModel),16>                  _model_allocator;
        PAMI::MemoryAllocator<sizeof(T_LocalModel),16>                  _model_allocator_bsr;
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
