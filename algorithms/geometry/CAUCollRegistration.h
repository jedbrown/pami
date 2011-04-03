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

#ifdef TRACE
#undef TRACE
#define TRACE(x)  //fprintf x
#else
#define TRACE(x)  //fprintf x
#endif



// Collective Registration for CAU protocols for p2p
namespace PAMI
{
  namespace CollRegistration
  {
    namespace CAU
    {

      static inline int find_first_bit(uint64_t in)
         {
           int index;
           if (in == 0)
             return (0);
           for (index = 1; !(in&1); index++)
             in = (uint64_t)in >> 1;
           return index;
         }

      static inline int countbits (uint64_t in)
      {
        int count;
        for (count=0; in!=0; count++)
          in &= in - 1;
        return (count);
      }

      
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
          result.check.datatype_op = support[in->cmd.xfer_allreduce.dt][in->cmd.xfer_allreduce.op]?0:1;
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
                                                                     CCMI::ConnectionManager::SimpleConnMgr,
                                                                     1>
        MultiSyncFactory;

        void MsyncBSRMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:MultiSyncComposite:BSR:CAU");
        }
        typedef CCMI::Adaptor::Barrier::BarrierFactory2DeviceMsync < CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
                                                                     MsyncBSRMetaData,
                                                                     CCMI::ConnectionManager::SimpleConnMgr,
                                                                     1>
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
          CCMI::ConnectionManager::SimpleConnMgr,
          1>
        MultiCastFactory;
      };

      namespace Allreduce
      {
        void McombineMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:MultiCombineComposite:SHMEM:CAU");
          m->check_correct.values.alldtop   = 0;
          m->check_fn                       = CAU::op_dt_metadata_function;
          m->check_perf.values.hw_accel     = 1;
          m->range_lo_perf                  = 0;
          m->range_hi_perf                  = 64;
        }
        typedef CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2Device<0>,
                                                                                 McombineMetaData,
                                                                                 CCMI::ConnectionManager::SimpleConnMgr,
                                                                                 1>

        MultiCombineFactory;
      };
      namespace Reduce
      {
        void McombineMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:MultiCombineComposite:SHMEM:CAU");
          m->check_correct.values.alldtop   = 0;
          m->check_fn                       = CAU::op_dt_metadata_function;
          m->check_perf.values.hw_accel     = 1;
          m->range_lo_perf                  = 0;
          m->range_hi_perf                  = 64;
        }
        typedef CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2Device<2>,
                                                                                 McombineMetaData,
                                                                                 CCMI::ConnectionManager::SimpleConnMgr,
                                                                                 1>
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

        typedef struct GeometryInfo
        {
          CAURegistration                   *_registration;
          T_Geometry                        *_geometry;
          T_LocalModel                      *_local_model;
          T_LocalNI_AM                      *_ni;
          PAMI::Device::CAUGeometryInfo     *_cau_info;
          uint64_t                           _cau_mask;
          int                                _cau_group;
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
          _local_devs(ldev),
          _local_devs_bsr(ldevbsr),
          _global_dev(gdev),
          _g_barrier_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _l_barrierbsr_ni(_local_devs_bsr,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _g_broadcast_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _g_allreduce_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _g_reduce_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _barrier_reg(&_sconnmgr,NULL, &_g_barrier_ni),
          _barrierbsr_reg(&_sconnmgr,&_l_barrierbsr_ni, &_g_barrier_ni),
          _broadcast_reg(&_sconnmgr,NULL, false,&_g_broadcast_ni,true),
          _allreduce_reg(&_sconnmgr,NULL, &_g_allreduce_ni),
          _reduce_reg(&_sconnmgr,NULL, &_g_reduce_ni),
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

            //  Populate the initial mask of available CAU indexes
            LapiImpl::Context *cp = (LapiImpl::Context *)_Lapi_port[_lapi_handle];
            TRACE((stderr, "CAU Indexes Mask = %llx\n", cp->nrt[0]->table_info.cau_index_resources));
            
            // To initialize shared memory, we need to provide the task offset into the
            // local nodes, and the total number of nodes we have locally
            size_t                         peer;
            size_t                         numpeers;
            int                            numbits;
            uint64_t                       bitmask = 0x0ULL;
            
            mapping.task2peer(_global_task, peer);            
            mapping.nodePeers(numpeers);

            numbits = countbits(cp->nrt[0]->table_info.cau_index_resources);
            for(int i=0; i<numbits; i++)
              bitmask |= (1ULL<<(i));

            TRACE((stderr, "Initial Bitmask:  %lX, numbits=%d\n", bitmask, numbits));
            
            pami_result_t rc = _csmm.init(peer,numpeers,bitmask);
            PAMI_assertf(rc == PAMI_SUCCESS, "Collective shared memory allocation failed with error %d\n", rc);
          }

        inline pami_result_t analyze_impl(size_t         context_id,
                                          T_Geometry    *geometry,
                                          uint64_t      *inout_val,
                                          int           *inout_nelem,
                                          int            phase)
          {
            if(!_enabled) return PAMI_SUCCESS;
            switch(phase)
            {
                case 0:
                {
                  // Phase 1 of the geometry create:
                  // 1)  Allocate a mask out of shared memory, this should be atomic, and clear the mask
                  //     so that no other tasks will allocate from CAU
                  //     
                  // 2)  Return the mask to the user to be reduced with a bitwise AND operation
                  PAMI::Topology *topo        = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX));
                  uint            numtasks    = topo->size();

                  TRACE((stderr, "Phase 0:  New Geometry: %d size=%d:", geometry->comm(), geometry->size()));
                  for(int vvv=0; vvv<topo->size(); vvv++)
                    TRACE((stderr, "%d ", topo->index2Rank(vvv)));
                  TRACE((stderr, "\n"));
                  
                  // prepare for collshmem device control structure address distribution
                  _csmm.getSGCtrlStrVec(geometry, &inout_val[1]);
                  bool            participant       = geometry->isLocalMasterParticipant();
                  if(participant)
                    inout_val[0] = _csmm.getAndSetKey(0x0ULL);
                  else
                    inout_val[0] = 0xFFFFFFFFFFFFFFFFULL;

                  // Construct the geometry info object, so we can free our allocated objects later
                  GeometryInfo                     *geometryInfo = (GeometryInfo*)_geom_allocator.allocateObject();
                  geometryInfo->_registration                    = this;
                  geometryInfo->_geometry                        = geometry;
                  geometryInfo->_local_model                     = NULL;
                  geometryInfo->_ni                              = NULL;
                  geometryInfo->_cau_info                        = NULL;
                  geometryInfo->_cau_mask                        = inout_val[0];
                  geometryInfo->_cau_group                       = -1;
                  
                  geometry->setKey(Geometry::PAMI_GKEY_GEOMETRYINFO,geometryInfo);                  
                  geometry->setCleanupCallback(cleanupCallback, geometryInfo);

                  TRACE((stderr, "Phase 0:  New Geometry: %d size=%d: contribution=%llx\n",
                          geometry->comm(), geometry->size(),inout_val[0]));

                  inout_nelem[0]    = inout_nelem[0];
                  if(numtasks == 1)
                    inout_val[0] = 0x0ULL;
                  
                  return PAMI_SUCCESS;
                }
                case 1:
                {
                  // Phase 2 of the geometry create:
                  // 1)  Mask has been allocated on all the nodes.  Check the result to make sure that
                  //     all nodes have allocated the mask
                  //
                  // 2)  Allocate the CAU via group_create if this is a leader node
                  //     
                  // 3)  Reset the bitmask to reflect the new value
                  pami_task_t *rl = NULL;
                  PAMI::Topology *local_master_topo = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX));
                  pami_result_t   rc                = local_master_topo->rankList(&rl);
                  uint            num_master_tasks  = local_master_topo->size();
                  uint           *tasks             = (uint*)rl;
                  uint            groupid           = geometry->comm();
                  int             myrc              = 0;
                  bool            participant       = geometry->isLocalMasterParticipant();
                  int             index             = -1; 
                  GeometryInfo   *geometryInfo      = (GeometryInfo*)geometry->getKey(Geometry::PAMI_GKEY_GEOMETRYINFO);

                  TRACE((stderr, "Phase 1:  Geometry: %d size=%d reduction_val=%llx num_master_tasks=%d\n",
                          geometry->comm(), geometry->size(), inout_val[0], num_master_tasks));

                  
                  if(inout_val[0] == 0)
                  {
                    TRACE((stderr, "Phase 1:  P0: Disabling Collectives on Geometry %d inout_val=%llx\n",
                            geometry->comm(), inout_val[0]));
                    inout_val[0] = 0ULL;
                    if(participant)
                    {
                      TRACE((stderr, "Returning Key Geometry=%d  key=%llx\n",
                             geometry->comm(), geometryInfo->_cau_group));
                      _csmm.setKeyOR(geometryInfo->_cau_mask);
                    }
                    TRACE((stderr, "Phase 1: Returning\n"));
                    return PAMI_SUCCESS;
                  }
                  
                  uint master_rank   = ((PAMI::Topology *)geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX))->index2Rank(0);
                  uint master_index  = local_master_topo->rank2Index(master_rank);
                  void *ctrlstr      = (void *)inout_val[master_index+1];
                  if (ctrlstr == NULL)
                    ctrlstr = _csmm.getWGCtrlStr();
                  geometry->setKey(Geometry::GKEY_GEOMETRYCSNI,ctrlstr);

                  if(participant)
                  {
                    index = find_first_bit(inout_val[0])-1;
                    if(index != -1 && num_master_tasks > 1)
                    {
                      myrc = lapi_cau_group_create(_lapi_handle,index,num_master_tasks,tasks);
                      geometryInfo->_cau_mask      &= ~(1ULL<<(index));
                      geometryInfo->_cau_group  = index;
                      _csmm.setKeyOR(geometryInfo->_cau_mask);
                      PAMI_assertf(myrc == 0, "CAU Group Create Failed on geometry=%d with rc=%d using index=%d\n",
                                   geometry->comm(), myrc, index);
                      TRACE((stderr, "Group Creation %d rc=%d, set key to %llx\n",
                              geometry->comm(), myrc, geometryInfo->_cau_group));
                      
                    }
                    else if(num_master_tasks == 1)
                    {
                      TRACE((stderr, "Using SHM Only on Geometry %d 0\n", geometry->comm()));
                      _csmm.setKeyOR(geometryInfo->_cau_mask);
                      inout_val[0] = 0xFFFFFFFFFFFFFFFFULL;
                      return PAMI_SUCCESS;
                    }
                    else
                    {
                      TRACE((stderr, "Disabling Collectives on Geometry %d 0\n", geometry->comm()));
                      _csmm.setKeyOR(geometryInfo->_cau_mask);
                      inout_val[0] = 0ULL;
                    }
                  }
                  else
                    inout_val[0] = 0xFFFFFFFFFFFFFFFFULL;
                  
                  TRACE((stderr, "Phase 1 Geometry %d, index=%d mask=%llx inout_val=%llx:  \n",
                          geometry->comm(),index,_csmm.getKey(),inout_val[0]));
                  return PAMI_SUCCESS;
                }
                case 2:
                {
                  PAMI_assert(context_id == 0);
                  TRACE((stderr, "Phase 2:  Geometry: %d size=%d inout_vec=%llx\n",
                          geometry->comm(), geometry->size(), inout_val[0]));
                  if(inout_val[0]==0)
                  {
                    TRACE((stderr, "CAU collectives disabled in phase 2  Geometry: %d size=%d:\n", geometry->comm(), geometry->size()));
                    return PAMI_SUCCESS;
                  }
                  // This is where we construct the "device specific" information.
                  // The definition of this class is specified in the CAU device.
                  // We populate this device specific information into the geometry
                  // The protocol will query for the device specific information,
                  // and pass this into the M-* api during communication
                  PAMI::Topology *local_master_topo = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX));
                  PAMI::Topology *local_topo        = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX));
                  GeometryInfo   *geometryInfo      = (GeometryInfo*)geometry->getKey(Geometry::PAMI_GKEY_GEOMETRYINFO);
                  PAMI_assert(local_topo->size() != 0);
                  PAMI_assert(local_master_topo->size() != 0);
                  PAMI::Device::CAUGeometryInfo *cau_gi = NULL;

                  if(local_master_topo->size() > 1)
                  {
                    cau_gi = (PAMI::Device::CAUGeometryInfo *)_cau_geom_allocator.allocateObject();
                    new(cau_gi)PAMI::Device::CAUGeometryInfo(geometryInfo->_cau_group,
                                                             geometry->comm(),
                                                             local_master_topo);
                    geometry->setKey(Geometry::GKEY_MCAST_CLASSROUTEID,cau_gi);
                    geometry->setKey(Geometry::GKEY_MCOMB_CLASSROUTEID,cau_gi);
                    geometry->setKey(Geometry::GKEY_MSYNC_CLASSROUTEID,cau_gi);
                  }
                  // Get the topology for the local nodes
                  // and the topology for the "distributed masters" for the global communication

                  T_LocalNI_AM                     *ni = NULL;
                  T_LocalModel                     *local_model=NULL;

                  if(local_topo->size() > 1)
                  {
                    local_model  = (T_LocalModel*)_model_allocator.allocateObject();
                    void                             *csmm_ctrlstr = (void *) geometry->getKey(PAMI::Geometry::GKEY_GEOMETRYCSNI);
                    new(local_model)T_LocalModel(&_local_devs, geometry->comm(), local_topo, &_csmm, csmm_ctrlstr);

                    // Allocate the local native interface
                    ni           = (T_LocalNI_AM*)_ni_allocator.allocateObject();
                    new(ni)T_LocalNI_AM(*local_model,
                                        _client,
                                        _client_id,
                                        _context,
                                        _context_id,
                                        local_topo->rank2Index(_global_task),
                                        local_topo->size());
                  }

                  geometryInfo->_local_model                     = local_model;
                  geometryInfo->_ni                              = ni;
                  geometryInfo->_cau_info                        = cau_gi;

                  _barrier_reg.setNI(geometry, ni, &_g_barrier_ni);
                  _barrierbsr_reg.setNI(geometry, &_l_barrierbsr_ni, &_g_barrier_ni);
                  _broadcast_reg.setNI(geometry, ni, &_g_broadcast_ni);
                  _allreduce_reg.setNI(geometry, ni,&_g_allreduce_ni);
                  _reduce_reg.setNI(geometry, ni, &_g_reduce_ni);


                  TRACE((stderr, "Phase 2:  Enabling Collective on Geometry: %d cau_group=%d size=%d inout_vec=%llx\n",
                          geometry->comm(),geometryInfo->_cau_group ,geometry->size(), inout_val[0]));

                  // Add the geometry info to the geometry
                  geometry->setKey(PAMI::Geometry::GKEY_GEOMETRYCSNI, ni);
                  geometry->addCollective(PAMI_XFER_BARRIER,&_barrier_reg,context_id);

                  if(_local_devs_bsr.isInit())
                    geometry->addCollective(PAMI_XFER_BARRIER,&_barrierbsr_reg,context_id);

                  geometry->addCollective(PAMI_XFER_BROADCAST,&_broadcast_reg,context_id);
                  geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE,&_allreduce_reg,context_id);
                  geometry->addCollectiveCheck(PAMI_XFER_REDUCE,&_reduce_reg,context_id);

                  // Todo:  free the ginfo;
                  return PAMI_SUCCESS;


                }
                default:
                  PAMI_assertf(0, "Unknown Analyze Phase\n");
            }
            return PAMI_SUCCESS;
          }

        inline void freeGroup(T_Geometry *g, int cau_group)
          {
            _csmm.setKeyBit(cau_group);
            int rc = lapi_cau_group_destroy(_lapi_handle, cau_group);
            PAMI_assertf(rc == 0, "CAU Group Destory Failed on geometry=%d index=%d with rc=%d\n", g->comm(), cau_group, rc);
          }
        inline void freeLocalModel(T_LocalModel *m)
          {
            _model_allocator.returnObject(m);
          }
        inline void freeLocalNI(T_LocalNI_AM *ni)
          {
            _ni_allocator.returnObject(ni);
          }
        inline void freeGeomInfo(GeometryInfo *gi)
          {
            _geom_allocator.returnObject(gi);
          }
        inline void freeCauInfo(PAMI::Device::CAUGeometryInfo *cau_gi)
          {
            _cau_geom_allocator.returnObject(cau_gi);
          }


        static inline void cleanupCallback(pami_context_t ctxt, void *data, pami_result_t res)
          {
            GeometryInfo *gi = (GeometryInfo*) data;
            int commid = gi->_geometry->comm();

            if(gi->_geometry->isLocalMasterParticipant() && gi->_cau_info)
              gi->_registration->freeGroup(gi->_geometry, gi->_cau_group);

            if(gi->_ni)
              gi->_registration->freeLocalNI(gi->_ni);
            if(gi->_local_model)
              gi->_registration->freeLocalModel(gi->_local_model);

            if(gi->_cau_info)
              gi->_registration->freeCauInfo(gi->_cau_info);

            gi->_registration->freeGeomInfo(gi);

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

        // Registrations
        Barrier::MultiSyncFactory                                       _barrier_reg;
        Barrier::MultiSyncBSRFactory                                    _barrierbsr_reg;
        Broadcast::MultiCastFactory                                     _broadcast_reg;
        Allreduce::MultiCombineFactory                                  _allreduce_reg;
        Reduce::MultiCombineFactory                                     _reduce_reg;
        // BSR Device
        T_LocalBSRNI                                                    _l_barrierbsr_ni;

        // Factory Allocator
        // and Local NI allocator
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
