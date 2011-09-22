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
#include "algorithms/protocols/barrier/HybridBarrierT.h"
#include "algorithms/protocols/barrier/MultiSyncComposite.h"
#include "algorithms/protocols/broadcast/MultiCastComposite.h"
#include "algorithms/protocols/allreduce/MultiCombineComposite.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"
#include "common/lapiunix/lapifunc.h"


#define CAU_SETUPNI_P2P(NI_PTR) rc =                                    \
    NativeInterfaceCommon::constructNativeInterface                     \
    <T_Allocator, T_NI_P2P, T_Protocol, T_Device_P2P,                   \
     NativeInterfaceCommon::MULTICAST_ONLY>(                            \
       _proto_alloc,                                                    \
       _dev_p2p,                                                        \
       NI_PTR,                                                          \
       _client,                                                         \
       _context,                                                        \
       _context_id,                                                     \
       _client_id,                                                      \
       _dispatch_id)

namespace PAMI
{
  namespace CollRegistration
  {
    namespace CAU
    {

      // Implement Jenkin's one at a time hash
      static inline void init_hash(uint32_t *hash)
      {
        (*hash) = 0;
      }
      static inline void update_hash(uint32_t *hash,
                                     char     *key,
                                     uint32_t len)
      {
        uint32_t   i;
        for (i=0; i<len; ++i)
        {
          (*hash) += key[i];
          (*hash) += ((*hash) << 10);
          (*hash) ^= ((*hash) >> 6);
        }
      }
      static inline void finalize_hash(uint32_t *hash)
      {
        uint32_t mask = 0x1FFFFF;
        (*hash) += ((*hash) << 3);
        (*hash) ^= ((*hash) >> 11);
        (*hash) += ((*hash) << 15);
        // 27 bit value good for CAU group id
        (*hash) = ((*hash) & mask);        
      }

      // 27 bit cau value generator
      // 6 bits job uniqifier
      // 21 bits hash
      static inline uint32_t GenerateUniqueKey(uint32_t        uniqifier,
                                               uint32_t        job_id, 
                                               uint32_t        geometry_id,
                                               PAMI::Topology *topology)
      {
        uint32_t rc  =0;
        uint32_t hash=0;
        init_hash(&hash);
        // We don't really need the job id, but this generates
        // a per-job hash for the low bits
        update_hash(&hash, (char*)&job_id, sizeof(job_id));
        update_hash(&hash, (char*)&geometry_id, sizeof(geometry_id));
        switch (topology->type())
        {
            case PAMI_RANGE_TOPOLOGY:
            {
              pami_task_t rfirst;
              pami_task_t rlast;
              topology->rankRange(&rfirst, &rlast);
              for (uint32_t i = (size_t)rfirst; i <= (uint32_t)rlast; i ++)
                update_hash(&hash, (char*)&i, sizeof(i));
              break;
            }
            default:
            {
              pami_task_t *rlist;
              topology->rankList(&rlist);
              for (int i = 0; i < topology->size(); i ++)
                update_hash(&hash, (char*)&rlist[i], sizeof(rlist[i]));
              break;
            }
        }
        finalize_hash(&hash);
        hash = (uniqifier<<21)|hash;
        return hash;
      }
      static inline int countbits (uint64_t in)
      {
        int count;
        for (count=0; in!=0; count++)
          in &= in - 1;
        return (count);
      }

      
          const bool support[PAMI_DT_COUNT][PAMI_OP_COUNT] =
            {
         //  COPY,             NOOP,      MAX,      MIN,      SUM,      PROD,      LAND,      LOR,      LXOR,      BAND,      BOR,      BXOR,      MAXLOC,      MINLOC
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_BYTE
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_SIGNED_CHAR
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_SIGNED_SHORT
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     true,      true,     true,      false,       false},//PRIMITIVE_TYPE_SIGNED_INT
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     true,      true,     true,      false,       false},//PRIMITIVE_TYPE_SIGNED_LONG
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     true,      true,     true,      false,       false},//PRIMITIVE_TYPE_SIGNED_LONG_LONG
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_UNSIGNED_CHAR
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_UNSIGNED_SHORT
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     true,      true,     true,      false,       false},//PRIMITIVE_TYPE_UNSIGNED_INT
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     true,      true,     true,      false,       false},//PRIMITIVE_TYPE_UNSIGNED_LONG
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     true,      true,     true,      false,       false},//PRIMITIVE_TYPE_UNSIGNED_LONG_LONG
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_FLOAT
            {false,            false,     true,     true,     true,     false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_DOUBLE
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_LONG_DOUBLE
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_LOGICAL
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_SINGLE_COMPLEX
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_DOUBLE_COMPLEX
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_LOC_2INT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_LOC_2FLOAT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_LOC_2DOUBLE
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_LOC_SHORT_INT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//PRIMITIVE_TYPE_LOC_FLOAT_INT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false} //PRIMITIVE_TYPE_LOC_DOUBLE_INT
            };

      template <class T_reduce_type>
      inline metadata_result_t op_dt_metadata_function(struct pami_xfer_t *in)
        {
          T_reduce_type     *reduction = (T_reduce_type*)&in->cmd;
          metadata_result_t  result    = {0};
          uintptr_t          op;
          uintptr_t          dt;
          PAMI::Type::TypeFunc::GetEnums(reduction->stype,
                                         reduction->op,
                                         dt,
                                         op);
          if(op < PAMI_OP_COUNT && dt < PAMI_DT_COUNT)
            result.check.datatype_op = support[dt][op]?0:1;
          else
            result.check.datatype_op = false;

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
          m->check_perf.values.hw_accel     = 1;
        }
        typedef CCMI::Adaptor::Barrier::BarrierFactory2DeviceMsync < CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
                                                                     MsyncMetaData,
                                                                     CCMI::ConnectionManager::SimpleConnMgr,
                                                                     1>
        MultiSyncFactory;

        void MsyncBSRMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:MultiSyncComposite:BSR:CAU");
          m->check_perf.values.hw_accel     = 1;
        }
        typedef CCMI::Adaptor::Barrier::BarrierFactory2DeviceMsync < CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
                                                                     MsyncBSRMetaData,
                                                                     CCMI::ConnectionManager::SimpleConnMgr,
                                                                     1>
        MultiSyncBSRFactory;

        extern inline void HybridBSRMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:Hybrid:BSR:P2P");
        }
        extern inline void HybridSHMEMMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:Hybrid:SHMEM:P2P");
        }
        extern inline void GlobalP2PMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:HybridP2PComponent:SHMEM:P2P");
        }
        extern inline bool hybrid_analyze (PAMI_GEOMETRY_CLASS *geometry)
        {
          return true;
        }

        template <PAMI::Geometry::ckeys_t T_Key>
        class GlobalP2PBarrier
        {
        public:
          typedef CCMI::Adaptor::Barrier::BarrierT
          < CCMI::Schedule::TopoMultinomial,
            Barrier::hybrid_analyze,
            PAMI::Geometry::MASTER_TOPOLOGY_INDEX,
            T_Key >
          GlobalP2PBarrierType;
        };
        
        template <PAMI::Geometry::ckeys_t T_Key>
        class GlobalP2PBarrierFactory
        {
        public:
          typedef CCMI::Adaptor::Barrier::BarrierFactoryT
          < typename Barrier::GlobalP2PBarrier<T_Key>::GlobalP2PBarrierType,
            Barrier::GlobalP2PMetaData,
            CCMI::ConnectionManager::SimpleConnMgr,
            false,
            T_Key >
          GlobalP2PBarrierFactoryType;
        };

        // Since templated typedefs aren't in the c++ compilers we are using
        // We need a templated class to build this type
        template <class T_NativeInterface, PAMI::Geometry::ckeys_t T_Key>
        class HybridBarrier
        {
        public:
          typedef CCMI::Adaptor::Barrier::HybridBarrierCompositeT
          < typename GlobalP2PBarrierFactory<T_Key>::GlobalP2PBarrierFactoryType,
            typename GlobalP2PBarrier<T_Key>::GlobalP2PBarrierType,
            T_NativeInterface>
          HybridBarrierType;
        };
        template<class T_NativeInterface, CCMI::Adaptor::MetaDataFn get_metadata, PAMI::Geometry::ckeys_t T_Key>
        class HybridBarrierFactory
        {
        public:
          typedef CCMI::Adaptor::Barrier::HybridBarrierFactoryT
          <typename HybridBarrier<T_NativeInterface,T_Key>::HybridBarrierType,
           get_metadata,
           typename GlobalP2PBarrierFactory<T_Key>::GlobalP2PBarrierFactoryType,
           T_NativeInterface,
           PAMI::Geometry::GKEY_MSYNC_LOCAL_CLASSROUTEID>
          HybridBarrierFactoryType;
        };
      };

      namespace Broadcast
      {
        void McastMetaData(pami_metadata_t *m)
        {
          new(m) PAMI::Geometry::Metadata("I0:MultiCastComposite:SHMEM:CAU");
          m->check_perf.values.hw_accel     = 1;
        }
        typedef CCMI::Adaptor::Broadcast::MultiCastComposite2DeviceFactoryT
        < CCMI::Adaptor::Broadcast::MultiCastComposite2Device<PAMI_GEOMETRY_CLASS,true,false>,
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
          m->check_fn                       = CAU::op_dt_metadata_function<pami_allreduce_t>;
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
          m->check_fn                       = CAU::op_dt_metadata_function<pami_reduce_t>;
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
                class T_Device_P2P,
                class T_NI_P2P,
                class T_Allocator,
                class T_Protocol,
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
                                                                                 T_Device_P2P,
                                                                                 T_NI_P2P,
                                                                                 T_Allocator,
                                                                                 T_Protocol,
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
          PAMI::Device::BSRGeometryInfo     *_bsr_info;
          CCMI::Executor::Composite         *_bsr_p2p_composite;
          CCMI::Executor::Composite         *_shm_p2p_composite;
          uint32_t                           _unique_key;
          uint64_t                           _ctlstr_offset;
        }GeometryInfo;
        typedef typename Barrier::HybridBarrierFactory<T_LocalBSRNI,
                                                       Barrier::HybridBSRMetaData,
                                                       PAMI::Geometry::CKEY_BARRIERCOMPOSITE2 >::HybridBarrierFactoryType
        HybridBSRBarrierFactory;

        typedef typename Barrier::HybridBarrierFactory<T_LocalNI_AM,
                                                       Barrier::HybridSHMEMMetaData,
                                                       PAMI::Geometry::CKEY_BARRIERCOMPOSITE3>::HybridBarrierFactoryType
        HybridSHMEMBarrierFactory;

        typedef typename Barrier::GlobalP2PBarrierFactory<PAMI::Geometry::CKEY_BARRIERCOMPOSITE2>::GlobalP2PBarrierFactoryType GlobalBsrFactory;
        typedef typename Barrier::GlobalP2PBarrierFactory<PAMI::Geometry::CKEY_BARRIERCOMPOSITE3>::GlobalP2PBarrierFactoryType GlobalSHMEMFactory;

      public:
        inline CAURegistration(pami_client_t                        client,
                               pami_context_t                       context,
                               size_t                               context_id,
                               size_t                               client_id,
                               T_Local_Device                      &ldev,
                               T_Local_DeviceBSR                   &ldevbsr,
                               T_Global_Device                     &gdev,
                               T_Device_P2P                        &dev_p2p,
                               T_Allocator                         &proto_alloc,
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
                                                                            T_Device_P2P,
                                                                            T_NI_P2P,
                                                                            T_Allocator,
                                                                            T_Protocol,
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
          _l_barrierbsr_ni(_local_devs_bsr,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _g_barrier_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _g_broadcast_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _g_allreduce_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _g_reduce_ni(_global_dev,client,context,context_id,client_id,_global_task,_global_size,dispatch_id),
          _dev_p2p(dev_p2p),
          _proto_alloc(proto_alloc),
          _barrierbsrp2p_reg(NULL),
          _barriershmemp2p_reg(NULL),
          _barrier_reg(&_sconnmgr,NULL, &_g_barrier_ni),
          _barrierbsr_reg(&_sconnmgr,&_l_barrierbsr_ni, &_g_barrier_ni),
          _broadcast_reg(&_sconnmgr,NULL, false,&_g_broadcast_ni,false),
          _allreduce_reg(&_sconnmgr,NULL, &_g_allreduce_ni),
          _reduce_reg(&_sconnmgr,NULL, &_g_reduce_ni),
          _csmm(mm),
          _valid_context(true)
          {
            LapiImpl::Context *cp = (LapiImpl::Context *)_Lapi_port[_lapi_handle];

            if(cp->coll_use_shm)
              _enabled = true;
            else
              _enabled = false;

            if(!mm) _enabled=false;

            if(!_enabled) return;

            // To initialize shared memory, we need to provide the task offset into the
            // local nodes, and the total number of nodes we have locally
            size_t                         peer;
            size_t                         numpeers;
            int                            numbits;

            mapping.task2peer(_global_task, peer);            
            mapping.nodePeers(numpeers);

            numbits = countbits(cp->nrt[0]->table_info.cau_index_resources);
            _cau_uniqifier   = _global_dev.getUniqifier();
            pami_result_t rc = _csmm.init(peer,numpeers,numbits);
            if(rc == PAMI_SUCCESS)
            {
              _enabled = true;
              CAU_SETUPNI_P2P(_p2p_ni_bsr);
              CAU_SETUPNI_P2P(_p2p_ni_shmem);

              _globalp2p_barrier_reg_bsr = new(&_globalp2p_barrier_reg_store0[0])
                GlobalBsrFactory(&_sconnmgr,
                                 _p2p_ni_bsr,
                                 GlobalBsrFactory::cb_head);
              _globalp2p_barrier_reg_shmem = new(&_globalp2p_barrier_reg_store1[0])
                GlobalSHMEMFactory(&_sconnmgr,
                                   _p2p_ni_shmem,
                                   GlobalSHMEMFactory::cb_head);
              _globalp2p_barrier_reg_bsr->setMapIdToGeometry(mapidtogeometry);
              _globalp2p_barrier_reg_shmem->setMapIdToGeometry(mapidtogeometry);
              _barrierbsrp2p_reg   = new(&_bsrp2p_reg_store[0])   HybridBSRBarrierFactory();
              _barriershmemp2p_reg = new(&_shmemp2p_reg_store[0]) HybridSHMEMBarrierFactory();
            }
            else
              _enabled = false;
          }

        inline void invalidateContext()
          {
            _valid_context=false;
          }

        
        inline void setup_cau(T_Geometry                     *geometry,
                              PAMI::Device::CAUGeometryInfo **cau_gi)
          {
            PAMI::Topology *master_topo       = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX));
            uint            num_master_tasks  = master_topo->size();
            bool            participant       = geometry->isLocalMasterParticipant();
            GeometryInfo   *geometryInfo      = (GeometryInfo*)geometry->getKey(Geometry::PAMI_GKEY_GEOMETRYINFO);
            pami_task_t    *rl    = NULL;
            pami_result_t   rc    = master_topo->rankList(&rl);
            uint           *tasks = (uint*)rl;
            int             myrc  = lapi_cau_group_create(_lapi_handle,
                                                          geometryInfo->_unique_key,
                                                          num_master_tasks,
                                                          tasks);
            *cau_gi = (PAMI::Device::CAUGeometryInfo *)_cau_geom_allocator.allocateObject();
            new(*cau_gi)PAMI::Device::CAUGeometryInfo(geometryInfo->_unique_key,geometry->comm());
            geometry->setKey(Geometry::GKEY_MCAST_CLASSROUTEID,*cau_gi);
            geometry->setKey(Geometry::GKEY_MCOMB_CLASSROUTEID,*cau_gi);
            geometry->setKey(Geometry::GKEY_MSYNC_CLASSROUTEID,*cau_gi);
          }

        inline void setup_shm(T_Geometry                    *geometry,
                              T_LocalNI_AM                 **ni,
                              T_LocalModel                 **local_model,
                              void                          *ctrlstr)
          
          {
            PAMI::Topology *master_topo   = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX));
            GeometryInfo   *geometryInfo  = (GeometryInfo*)geometry->getKey(Geometry::PAMI_GKEY_GEOMETRYINFO);
            PAMI::Topology *local_topo    = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX));
            *local_model  = (T_LocalModel*)_model_allocator.allocateObject();
            new(*local_model)T_LocalModel(&_local_devs, geometry->comm(), local_topo, &_csmm, ctrlstr);
            *ni           = (T_LocalNI_AM*)_ni_allocator.allocateObject();
            new(*ni)T_LocalNI_AM(**local_model,
                                 _client,
                                 _client_id,
                                 _context,
                                 _context_id,
                                 local_topo->rank2Index(_global_task),
                                 local_topo->size());
            // Add the geometry info to the geometry
            geometry->setKey(PAMI::Geometry::GKEY_GEOMETRYCSNI, ni);
          }
        inline void setup_bsr(T_Geometry                     *geometry,
                              PAMI::Device::BSRGeometryInfo **bsr_gi)
          {
            PAMI::Topology *local_topo   = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX));
            GeometryInfo   *geometryInfo = (GeometryInfo*)geometry->getKey(Geometry::PAMI_GKEY_GEOMETRYINFO);
            *bsr_gi                      = (PAMI::Device::BSRGeometryInfo *)_bsr_geom_allocator.allocateObject();
            new(*bsr_gi)PAMI::Device::BSRGeometryInfo(geometry->comm(),local_topo, geometryInfo->_unique_key);
            geometry->setKey(Geometry::GKEY_MSYNC_LOCAL_CLASSROUTEID,*bsr_gi);
          }

        inline pami_result_t analyze_impl(size_t         context_id,
                                          T_Geometry    *geometry,
                                          uint64_t      *inout_val,
                                          int           *inout_nelem,
                                          int            phase)
          {
            if(!_enabled || geometry->size() == 1)
            {
              if(phase==0)
              {
                ITRC(IT_CAU, "CollReg(phase 0): group_id=%d: Disabled:  flag=%d sz=%d",
                     _enabled,geometry->comm());
              }
              return PAMI_SUCCESS;
            }
            switch(phase)
            {
                case 0:
                {
                  // Phase 1 of the geometry create:
                  // 1)  Generate a unique key from jobid/comm/ranklist and a job uniqifier supplied by the device
                  // 2)  Allocate an index from shared memory (essentially an atomic decrement), disable if no indexes left
                  // 3)  Check for unique key collsions from other geometries, disable if collision
                  // Note:  Disabling means contributing "0" to the allreduce
                  //        non-leader tasks should never disable algorithms
                  PAMI::Topology    *topo              = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX));
                  PAMI::Topology    *master_topo       = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX));
                  uint               numtasks          = topo->size();
                  uint               num_master_tasks  = master_topo->size();
                  LapiImpl::Context *cp                = (LapiImpl::Context *)_Lapi_port[_lapi_handle];
                  bool               participant       = geometry->isLocalMasterParticipant();



                  // Word 0 reduction setup
                  // Generate a unique key and check for collisions
                  uint32_t unique_key=CAU::GenerateUniqueKey(_cau_uniqifier,
                                                             cp->part_id.p_id,
                                                             geometry->comm(),
                                                             topo);
                  typename std::map<uint32_t,uint32_t>::iterator findval;
                  findval=_collision_map.find(unique_key);
                  bool found_key = (findval == _collision_map.end())?false:true;
                  if(found_key == false) _collision_map[unique_key] = 1;
                  else
                  {
                    PAMI_assertf(_collision_map[unique_key]>0,
                                 "Error in geometry collision map\n");
                    _collision_map[unique_key]++;
                  }

                  // Success is 0
                  // Failure: key(cau resource) allocation failure bit 1 set
                  uint64_t result_mask = 0ULL;
                  if(participant)
                    result_mask = (_csmm.allocateKey() == PAMI_SUCCESS)?0ULL:1ULL;

                  // Failure:  no cau uniqifier, bit 2 set
                  if(participant && _cau_uniqifier==0)
                    result_mask |= 2ULL;

                  // Failure:  CAU collision, bit 3, or affinity settings wrong
                  if((participant && _collision_map[unique_key]!=1))
                    result_mask |= 4ULL;

                  // Failure:  BSR Collision, we don't need job unique key
                  if((_collision_map[unique_key]!=1))
                    result_mask |= 8ULL;
                  
                  // Check to see if BSR is enabled on all tasks
                  // in the geometry.  Note that this is a global
                  // setting (currently), because bsr enablement
                  // is tied to affinity settings.  We still
                  // check affinity here
                  if(!_local_devs_bsr.isInit())
                    result_mask |= 16ULL;

                  // invert for bitwise AND
                  inout_val[0] = ~result_mask;

                  // Construct the geometry info object, so we can free our allocated objects later
                  GeometryInfo                     *geometryInfo = (GeometryInfo*)_geom_allocator.allocateObject();
                  memset(geometryInfo, 0, sizeof(*geometryInfo));
                  geometryInfo->_registration                    = this;
                  geometryInfo->_geometry                        = geometry;
                  geometryInfo->_local_model                     = NULL;
                  geometryInfo->_ni                              = NULL;
                  geometryInfo->_bsr_info                        = NULL;
                  geometryInfo->_cau_info                        = NULL;
                  geometryInfo->_bsr_p2p_composite               = NULL;
                  geometryInfo->_shm_p2p_composite               = NULL;
                  geometryInfo->_unique_key                      = unique_key;
                  // Set the CAU registration per geometry info object in the geometry
                  // And the cleanup callback
                  geometry->setKey(Geometry::PAMI_GKEY_GEOMETRYINFO,geometryInfo);
                  geometry->setCleanupCallback(cleanupCallback, geometryInfo);

                  // Word 1-->num_local_tasks setup
                  // prepare for collshmem device control structure address distribution
                  _csmm.getSGCtrlStrVec(geometry,
                                        &inout_val[1],
                                        &geometryInfo->_ctlstr_offset);


                  inout_nelem[0]    = inout_nelem[0];
                  ITRC(IT_CAU, "CollReg(phase 0): group_id=%d, unique_id=0x%x "
                       "reduceMask:0x%lx noGroup=0x%x noJobUniq=0x%x cau_collis=0x%x bsr_collis=0x%lx no_affinity=0x%llx\n",
                       geometry->comm(),
                       unique_key,
                       result_mask,        // result mask
                       result_mask&1ULL,   // key allocated
                       result_mask&2ULL,   // cau uniqifier
                       result_mask&4ULL,   // cau collision
                       result_mask&8ULL,   // bsr collision
                       result_mask&16ULL); // affinity
                  return PAMI_SUCCESS;
                }
                case 1:
                {
                  // Phase 2 of the geometry create:
                  // 1)  Check allreduce results to determine if all leaders have resources
                  // 2)  Allocate the CAU via group_create if this is a leader node
                  // 3)  return the cau resource via returnKey if cau is unavailable across the group
                  PAMI::Topology *master_topo      = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX));
                  PAMI::Topology *local_topo       = (PAMI::Topology *) (geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX));
                  uint            num_tasks        = geometry->size();
                  uint            num_master_tasks = master_topo->size();
                  uint            num_local_tasks  = local_topo->size();
                  uint            groupid          = geometry->comm();
                  bool            participant      = geometry->isLocalMasterParticipant();
                  GeometryInfo   *geometryInfo     = (GeometryInfo*)geometry->getKey(Geometry::PAMI_GKEY_GEOMETRYINFO);
                  uint            master_rank      = ((PAMI::Topology *)geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX))->index2Rank(0);
                  uint            master_index     = master_topo->rank2Index(master_rank);
                  void           *ctrlstr          = (void *)inout_val[master_index+1];
                  uint64_t        result_mask      = ~inout_val[0];
                  bool            singleNode       = (num_local_tasks == num_tasks);
                  bool            useCau           = ((result_mask & (~16ULL)) == 0) && !singleNode;
                  bool            collision        = (result_mask&8ULL);
                  bool            no_affinity      = (result_mask&16ULL);
                  bool            useBsr           = !(collision || no_affinity);
                  
                  ITRC(IT_CAU, "CollReg(phase 1) Receive: group_id=%d:  rmask:0x%lx noGroup=0x%lx "
                       "noUniq=0x%lx cau collis=0x%lx bsr collis=0x%lx no_affinity=0x%lx singleNode=%d\n",
                       groupid,
                       result_mask,        // result mask
                       result_mask&1ULL,   // key allocated
                       result_mask&2ULL,   // cau uniqifier
                       result_mask&4ULL,   // cau collision
                       result_mask&8ULL,   // bsr collision
                       result_mask&16ULL,  // affinity 
                       singleNode);        // single node
                  
                  // Set up required communication channels based on reduction results
                  // Each "gi", or per geometry info represents a communication channel
                  // cau_gi:              global communication channel over CAU network
                  // bsr_gi:              local communication channel over bsr network
                  // ni(and local model): local communication channel over shared memory
                  //
                  // useCau and useBsr are flags indicating whether or not the protocols
                  // should be available across all endpoints in the geometry.
                  //
                  // Shared memory is always available across the endpoints, so there
                  // is no reduction required to determine if shared memory is available
                  // likewise, "P2P" networks are always available and the "Hybrid"
                  // shared memory and P2P protocols will always be available.
                  //
                  // We only set up communication channels for globalB devices if
                  // the task belongs to the master topology.  Also, we only add comm
                  // channels for local devices is there are more than one task.
                  PAMI::Device::CAUGeometryInfo *cau_gi      = NULL;
                  PAMI::Device::BSRGeometryInfo *bsr_gi      = NULL;
                  T_LocalNI_AM                  *ni          = NULL;
                  T_LocalModel                  *local_model = NULL;
                  
                  if(useCau && participant)
                    setup_cau(geometry, &cau_gi);

                  if(useBsr && num_local_tasks>1)
                    setup_bsr(geometry, &bsr_gi);

                  GlobalBsrFactory   *f0 = NULL;
                  GlobalSHMEMFactory *f1 = NULL;
                  if(num_master_tasks>1 && participant)
                  {
                    f0 = _globalp2p_barrier_reg_bsr;
                    f1 = _globalp2p_barrier_reg_shmem;
                  }
                  
                  if(num_local_tasks>1)
                    setup_shm(geometry, &ni, &local_model, ctrlstr);

                  // Set info in the geometry so these can be cleaned up
                  geometryInfo->_local_model                     = local_model;
                  geometryInfo->_ni                              = ni;
                  geometryInfo->_cau_info                        = cau_gi;
                  geometryInfo->_bsr_info                        = bsr_gi;

                  _barrier_reg.setNI(geometry, ni, &_g_barrier_ni);
                  _barrierbsr_reg.setNI(geometry, &_l_barrierbsr_ni, &_g_barrier_ni);
                  _broadcast_reg.setNI(geometry, ni, &_g_broadcast_ni);
                  _allreduce_reg.setNI(geometry, ni,&_g_allreduce_ni);
                  _reduce_reg.setNI(geometry, ni, &_g_reduce_ni);

                  ITRC(IT_CAU, "CollReg(phase 1) Info:group_id=%d: useCau=%d "
                       "useBsr=%d ni=%p cau_gi=%p bsr_gi=%p f0=%p f1=%p lt=%d mt=%d sz=%d\n",
                       groupid, useCau, useBsr, ni, cau_gi, bsr_gi, f0, f1,
                       num_local_tasks, num_master_tasks, num_tasks);
                  
                  if(num_local_tasks > 1 && useBsr)
                    _barrierbsrp2p_reg->setInfo(geometry, &_l_barrierbsr_ni, f0);
                  else
                    _barrierbsrp2p_reg->setInfo(geometry, NULL, f0);
                  if(num_local_tasks > 1)
                    _barriershmemp2p_reg->setInfo(geometry, ni, f1);
                  else
                    _barriershmemp2p_reg->setInfo(geometry, NULL, f1);

                  // Generate barrier composites
                  // These composites are generated
                  // early because global p2p barriers over CCMI
                  // require this generation to handle early arrival
                  // and are cached to avoid re-creation.
                  // Note:  keys must match the template defnition
                  pami_xfer_t xfer = {0};
                  if(f0)
                  {
                    geometryInfo->_bsr_p2p_composite = f0->generate(geometry, &xfer);
                    geometry->setKey(context_id,
                                     PAMI::Geometry::CKEY_BARRIERCOMPOSITE2,
                                     (void*)geometryInfo->_bsr_p2p_composite);
                    PAMI_assert(geometryInfo->_bsr_p2p_composite != NULL);
                  }
                  if(f1)
                  {

                    geometryInfo->_shm_p2p_composite = f1->generate(geometry, &xfer);
                    geometry->setKey(context_id,
                                     PAMI::Geometry::CKEY_BARRIERCOMPOSITE3,
                                     (void*)geometryInfo->_shm_p2p_composite);
                    PAMI_assert(geometryInfo->_shm_p2p_composite != NULL);
                  }

                  // ///////////////////////////////////////////////////////////
                  // Barrier Protocols
                  // ///////////////////////////////////////////////////////////
                  if(useCau)
                  {
                    ITRC(IT_CAU, "CollReg(phase 1): group_id=%d adding CAU/SHM Barrier collective to list\n",groupid);
                    geometry->addCollective(PAMI_XFER_BARRIER,
                                            &_barrier_reg,
                                            context_id);
                  }

                  ITRC(IT_CAU, "CollReg(phase 1): group_id=%d adding SHM/P2P Barrier collective to list\n",groupid);
                  geometry->addCollective(PAMI_XFER_BARRIER,
                                          _barriershmemp2p_reg,
                                          _context_id);

                  if(useBsr && useCau)
                  {
                    ITRC(IT_CAU, "CollReg(phase 1): group_id=%d adding CAU/BSR Barrier collective to list\n", groupid);
                    geometry->addCollective(PAMI_XFER_BARRIER,
                                            &_barrierbsr_reg,
                                            context_id);
                  }

                  if(useBsr)
                  {
                    ITRC(IT_CAU, "CollReg(phase 1): group_id=%d adding BSR/P2P Barrier collective to list\n",groupid);
                    geometry->addCollective(PAMI_XFER_BARRIER,
                                            _barrierbsrp2p_reg,
                                            _context_id);
                  }

                  // ///////////////////////////////////////////////////////////
                  // Broadcast Protocols
                  // ///////////////////////////////////////////////////////////
                  if(useCau)
                  {
                    ITRC(IT_CAU, "CollReg(phase 1): group_id=%d adding CAU/SHM Broadcast collective to list\n", groupid);
                    geometry->addCollective(PAMI_XFER_BROADCAST,&_broadcast_reg,context_id);
                  }
                  else if(singleNode)
                  {
                    ITRC(IT_CAU, "CollReg(phase 1): group_id=%d adding CAU/SHM Broadcast collective to list (singlenode)\n", groupid);
                    geometry->addCollective(PAMI_XFER_BROADCAST,&_broadcast_reg,context_id);

                  }
                  
                  // ///////////////////////////////////////////////////////////
                  // Reduce Protocols
                  // ///////////////////////////////////////////////////////////
                  // If cau is enabled, we need to restrict the operations available by the
                  // CAU/SHM algorithms (cau cannot do all the math, metadata is important)
                  if(useCau)
                  {
                    ITRC(IT_CAU, "CollReg(phase 1): group_id=%d adding CAU/SHM Reduce/Allreduce collective to must-check list\n", groupid);
                    geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE,&_allreduce_reg,context_id);
                    geometry->addCollectiveCheck(PAMI_XFER_REDUCE,&_reduce_reg,context_id);
                  }
                  else if(singleNode)
                  {
                    ITRC(IT_CAU, "CollReg(phase 1): group_id=%d adding CAU/SHM Reduce/Allreduce collective to always works list(singlenode)\n", groupid);
                    geometry->addCollective(PAMI_XFER_ALLREDUCE,&_allreduce_reg,context_id);
                    geometry->addCollective(PAMI_XFER_REDUCE,&_reduce_reg,context_id);
                  }
                  return PAMI_SUCCESS;
                }
                default:
                  PAMI_assertf(0, "Unknown Analyze Phase\n");
            }
            return PAMI_SUCCESS;
          }

        inline void freeGroup(T_Geometry *g, int cau_group)
          {
            ITRC(IT_CAU, "CollReg: Deleting group_id=%d\n", cau_group);
            int rc = 0;
            if(_valid_context)
            {
              rc = lapi_cau_group_destroy(_lapi_handle, cau_group);
              _csmm.returnKey();
            }
            PAMI_assertf(rc == 0, "CAU Group Destory Failed on geometry=%d index=%d with rc=%d\n", g->comm(), cau_group, rc);
          }
        inline void freeLocalModel(T_LocalModel *m)
          {
            m->destroy();
            _model_allocator.returnObject(m);
          }
        inline void freeLocalNI(T_LocalNI_AM *ni)
          {
            ni->destroy();
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
        inline void freeBsrInfo(PAMI::Device::BSRGeometryInfo *bsr_gi)
          {
            bsr_gi->~BSRGeometryInfo();
            _bsr_geom_allocator.returnObject(bsr_gi);
          }
        inline void freeSharedMemory(uint64_t ctlstr_offset)
          {
            _csmm.returnSGCtrlStr(ctlstr_offset);
          }
        inline void freeUniqueKey(uint32_t unique_key)
          {
            typename std::map<uint32_t,uint32_t>::iterator findval;
            findval=_collision_map.find(unique_key);
            bool found_key = (findval == _collision_map.end())?false:true;
            if(found_key)
            {
              _collision_map[unique_key]--;
              if(_collision_map[unique_key]==0)
                _collision_map.erase(unique_key);
            }
          }
        
        static inline void cleanupCallback(pami_context_t ctxt, void *data, pami_result_t res)
          {
            GeometryInfo *gi = (GeometryInfo*) data;


            int commid = gi->_geometry->comm();
            ITRC(IT_CAU, "CollReg:  cleanupCallback:  ctxt=%p geometry id=%d key=%x cau_info=%p\n ",
                 ctxt, commid, gi->_unique_key, gi->_cau_info);
            if(gi->_cau_info)
            {
              if(gi->_geometry->isLocalMasterParticipant())
                gi->_registration->freeGroup(gi->_geometry, gi->_unique_key);

              gi->_registration->freeCauInfo(gi->_cau_info);
            }
            if(gi->_local_model)
              gi->_registration->freeLocalModel(gi->_local_model);

            if(gi->_ni)
              gi->_registration->freeLocalNI(gi->_ni);

            if(gi->_bsr_info)
              gi->_registration->freeBsrInfo(gi->_bsr_info);

            if(gi->_bsr_p2p_composite)
              GlobalBsrFactory::cleanup_done_fn(ctxt,gi->_bsr_p2p_composite,res);

            if(gi->_shm_p2p_composite)
              GlobalSHMEMFactory::cleanup_done_fn(ctxt,gi->_shm_p2p_composite,res);

            gi->_registration->freeUniqueKey(gi->_unique_key);

            gi->_registration->freeSharedMemory(gi->_ctlstr_offset);
            
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
        std::map<uint32_t,uint32_t>                                     _collision_map;
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

        // BSR Device
        T_LocalBSRNI                                                    _l_barrierbsr_ni;

        // Global native interface
        T_GlobalNI_AM                                                   _g_barrier_ni;
        T_GlobalNI_AM                                                   _g_broadcast_ni;
        T_GlobalNI_AM                                                   _g_allreduce_ni;
        T_GlobalNI_AM                                                   _g_reduce_ni;

        // P2P Native Interfaces
        T_Device_P2P                                                   &_dev_p2p;
        T_Allocator                                                    &_proto_alloc;
        T_NI_P2P                                                       *_p2p_ni_bsr;
        T_NI_P2P                                                       *_p2p_ni_shmem;
        
        // Registrations, hybrid first
        GlobalBsrFactory                                               *_globalp2p_barrier_reg_bsr;
        GlobalSHMEMFactory                                             *_globalp2p_barrier_reg_shmem;
        HybridBSRBarrierFactory                                        *_barrierbsrp2p_reg;
        HybridSHMEMBarrierFactory                                      *_barriershmemp2p_reg;

        char                                                            _globalp2p_barrier_reg_store0[sizeof(GlobalBsrFactory)];
        char                                                            _globalp2p_barrier_reg_store1[sizeof(GlobalSHMEMFactory)];
        char                                                            _bsrp2p_reg_store[sizeof(HybridBSRBarrierFactory)];
        char                                                            _shmemp2p_reg_store[sizeof(HybridSHMEMBarrierFactory)];
        
        // Registration, optimized
        Barrier::MultiSyncFactory                                       _barrier_reg;
        Barrier::MultiSyncBSRFactory                                    _barrierbsr_reg;
        Broadcast::MultiCastFactory                                     _broadcast_reg;
        Allreduce::MultiCombineFactory                                  _allreduce_reg;
        Reduce::MultiCombineFactory                                     _reduce_reg;

        // Variable to uniquely identify the job with respect to cau
        unsigned                                                        _cau_uniqifier;

        // Factory Allocator
        // and Local NI allocator
        PAMI::MemoryAllocator<sizeof(T_LocalModel),16>                  _model_allocator;
        PAMI::MemoryAllocator<sizeof(T_LocalNI_AM),16>                  _ni_allocator;
        PAMI::MemoryAllocator<sizeof(GeometryInfo),16>                  _geom_allocator;
        PAMI::MemoryAllocator<sizeof(PAMI::Device::CAUGeometryInfo),16> _cau_geom_allocator;
        PAMI::MemoryAllocator<sizeof(PAMI::Device::BSRGeometryInfo),16> _bsr_geom_allocator;

        // Collective shared memory manager
        T_CSMemoryManager                                               _csmm;

        // Flag to indicate that contexts are still usable
        bool                                                            _valid_context;
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
