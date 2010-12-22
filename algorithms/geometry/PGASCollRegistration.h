/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/PGASCollRegistration.h
 * \brief ???
 */

#ifndef __algorithms_geometry_PGASCollRegistration_h__
#define __algorithms_geometry_PGASCollRegistration_h__

#include <map>
#include <vector>
#include "algorithms/interfaces/CollRegistrationInterface.h"
#include "algorithms/geometry/PGASWrapper.h"
#include "common/NativeInterface.h"

                

#define SETUPNI_P2P_SHMEM(NI_PTR) result =                                      \
  NativeInterfaceCommon::constructNativeInterface                               \
  <T_Allocator, T_NI, T_Protocol1, T_Device_P2P, T_Protocol2, T_Device_SHMEM,   \
   NativeInterfaceCommon::P2P_ONLY>(                                            \
                                         _proto_alloc,                          \
                                         _dev_p2p,                              \
                                         _dev_shmem,                            \
                                         NI_PTR,                                \
                                         _client,                               \
                                         _context,                              \
                                         _context_id,                           \
                                         _client_id,                            \
                                         _dispatch_id)

#define SETUPNI_P2P_P2P(NI_PTR) result =                                        \
  NativeInterfaceCommon::constructNativeInterface                               \
  <T_Allocator, T_NI, T_Protocol1, T_Device_P2P, T_Protocol1, T_Device_P2P,     \
   NativeInterfaceCommon::P2P_ONLY>(                                            \
                                         _proto_alloc,                          \
                                         _dev_p2p,                              \
                                         _dev_p2p,                              \
                                         NI_PTR,                                \
                                         _client,                               \
                                         _context,                              \
                                         _context_id,                           \
                                         _client_id,                            \
                                         _dispatch_id)


namespace PAMI
{
  namespace CollRegistration
  {
    char BarrierString[]        = "PGAS_Barrier";
    char BarrierUEString[]      = "PGAS_UEBarrier";
    char AllgatherString[]      = "PGAS_Allgather";
    char AllgathervString[]     = "PGAS_Allgatherv";
    char ScatterString[]        = "PGAS_Scatter";
    char ScattervString[]       = "PGAS_Scatterv";
    char AllreduceString[]      = "PGAS_Allreduce";
    char ShortAllreduceString[] = "PGAS_ShortAllreduce";
    char BroadcastString[]      = "PGAS_Broadcast";

    template <class T_Geometry,
              class T_NI,
              class T_Allocator,
              class T_Protocol1,
              class T_Protocol2,
              class T_Device_P2P,
              class T_Device_SHMEM,
              class T_NBCollMgr>
    class PGASRegistration :
      public CollRegistration<PAMI::CollRegistration::PGASRegistration<T_Geometry,
                                                                       T_NI,
                                                                       T_Allocator,
                                                                       T_Protocol1,
                                                                       T_Protocol2,
                                                                       T_Device_P2P,
                                                                       T_Device_SHMEM,
                                                                       T_NBCollMgr>,
                              T_Geometry>
      {
        typedef PGBarrierExec<T_Geometry,TSPColl::Barrier<T_NI>, T_NI, T_Device_P2P> BarExec;
        typedef PGBarrierExec<T_Geometry,TSPColl::BarrierUE<T_NI>, T_NI, T_Device_P2P> BarUEExec;
        typedef PGAllgatherExec<T_Geometry,TSPColl::Allgather<T_NI>, T_NI, T_Device_P2P> AllgatherExec;
        typedef PGAllgathervExec<T_Geometry,TSPColl::Allgatherv<T_NI>, T_NI, T_Device_P2P> AllgathervExec;
        typedef PGScatterExec<T_Geometry,TSPColl::Scatter<T_NI>, T_NI, T_Device_P2P,TSPColl::Barrier<T_NI> > ScatterExec;
        typedef PGScattervExec<T_Geometry,TSPColl::Scatterv<T_NI>, T_NI, T_Device_P2P,TSPColl::Barrier<T_NI> > ScattervExec;
        typedef PGAllreduceExec<T_Geometry,TSPColl::Allreduce::Long<T_NI>, T_NI, T_Device_P2P> AllreduceExec;
        typedef PGAllreduceExec<T_Geometry,TSPColl::Allreduce::Short<T_NI>, T_NI, T_Device_P2P> ShortAllreduceExec;
        typedef PGBroadcastExec<T_Geometry,TSPColl::BinomBcast<T_NI>, T_NI, T_Device_P2P> BroadcastExec;

        typedef PGFactory<TSPColl::Barrier<T_NI>,T_NI,T_Device_P2P,BarExec,BarrierString> BarrierFactory;
        typedef PGFactory<TSPColl::BarrierUE<T_NI>,T_NI,T_Device_P2P,BarUEExec,BarrierUEString> BarrierUEFactory;
        typedef PGFactory<TSPColl::Allgather<T_NI>,T_NI,T_Device_P2P,AllgatherExec,AllgatherString>  AllgatherFactory;
        typedef PGFactory<TSPColl::Allgatherv<T_NI>,T_NI,T_Device_P2P,AllgathervExec,AllgathervString> AllgathervFactory;
        typedef PGFactory<TSPColl::Scatter<T_NI>,T_NI,T_Device_P2P,ScatterExec,ScatterString, TSPColl::Barrier<T_NI> > ScatterFactory;
        typedef PGFactory<TSPColl::Scatterv<T_NI>,T_NI,T_Device_P2P,ScattervExec,ScattervString, TSPColl::Barrier<T_NI> > ScattervFactory;
        typedef PGFactory<TSPColl::Allreduce::Long<T_NI>,T_NI,T_Device_P2P,AllreduceExec,AllreduceString>  AllreduceFactory;
        typedef PGFactory<TSPColl::Allreduce::Short<T_NI>,T_NI,T_Device_P2P,ShortAllreduceExec,ShortAllreduceString> ShortAllreduceFactory;
        typedef PGFactory<TSPColl::BinomBcast<T_NI>,T_NI,T_Device_P2P,BroadcastExec,BroadcastString> BroadcastFactory;

        typedef union Factories
        {
          char barrier_blob[sizeof(BarrierFactory)];
          char barrier_ue_blob[sizeof(BarrierUEFactory)];
          char allgather_blob[sizeof(AllgatherFactory)];
          char allgatherv_blob[sizeof(AllgathervFactory)];
          char scatter_blob[sizeof(ScatterFactory)];
          char scatterv_blob[sizeof(ScattervFactory)];
          char allreduce_blob[sizeof(AllreduceFactory)];
          char shortallreduce_blob[sizeof(ShortAllreduceFactory)];
          char broadcast_blob[sizeof(BroadcastFactory)];
        }Factories;

      public:
      inline PGASRegistration(pami_client_t                        client,
                              pami_context_t                       context,
                              size_t                               client_id,
                              size_t                               context_id,
                              T_Allocator                         &proto_alloc,
                              T_Device_P2P                        &dev_p2p,
                              T_Device_SHMEM                      &dev_shmem,
                              int                                 *dispatch_id,
                              std::map<unsigned, pami_geometry_t> *geometry_map,
                              bool                                 use_shmem=false):
        CollRegistration<PAMI::CollRegistration::PGASRegistration<T_Geometry,
                                                                  T_NI,
                                                                  T_Allocator,
                                                                  T_Protocol1,
                                                                  T_Protocol2,
                                                                  T_Device_P2P,
                                                                  T_Device_SHMEM,
                                                                  T_NBCollMgr>,
                         T_Geometry> (),
        _client(client),
        _context(context),
        _client_id(client_id),
        _context_id(context_id),
        _reduce_val(0),
        _dispatch_id(dispatch_id),
        _geometry_map(geometry_map),
        _dev_p2p(dev_p2p),
        _dev_shmem(dev_shmem),
        _proto_alloc(proto_alloc),
        _bcast(NULL),
        _allgather(NULL),
        _allgatherv(NULL),
        _scatter_s(NULL),
        _scatter_b(NULL),
        _scatterv_s(NULL),
        _scatterv_b(NULL),
        _allreduce(NULL),
        _shortallreduce(NULL),
        _barrier(NULL),
        _barrier_ue(NULL)
          {
            pami_result_t       result   = PAMI_SUCCESS;

            if(use_shmem == true)
              {
                SETUPNI_P2P_SHMEM(_bcast);
                SETUPNI_P2P_SHMEM(_allgather);
                SETUPNI_P2P_SHMEM(_allgatherv);
                SETUPNI_P2P_SHMEM(_scatter_s);
                SETUPNI_P2P_SHMEM(_scatter_b);
                SETUPNI_P2P_SHMEM(_scatterv_s);
                SETUPNI_P2P_SHMEM(_scatterv_b);
                SETUPNI_P2P_SHMEM(_allreduce);
                SETUPNI_P2P_SHMEM(_shortallreduce);
                SETUPNI_P2P_SHMEM(_barrier);
                SETUPNI_P2P_SHMEM(_barrier_ue);
              }
            else
              {
                SETUPNI_P2P_P2P(_bcast);
                SETUPNI_P2P_P2P(_allgather);
                SETUPNI_P2P_P2P(_allgatherv);
                SETUPNI_P2P_P2P(_scatter_s);
                SETUPNI_P2P_P2P(_scatter_b);
                SETUPNI_P2P_P2P(_scatterv_s);
                SETUPNI_P2P_P2P(_scatterv_b);
                SETUPNI_P2P_P2P(_allreduce);
                SETUPNI_P2P_P2P(_shortallreduce);
                SETUPNI_P2P_P2P(_barrier);
                SETUPNI_P2P_P2P(_barrier_ue);
              }
            
            _mgr.initialize();
            _mgr.multisend_reg(TSPColl::BcastTag,          _bcast);
            _mgr.multisend_reg(TSPColl::AllgatherTag,      _allgather);
            _mgr.multisend_reg(TSPColl::AllgathervTag,     _allgatherv);
            _mgr.multisend_reg(TSPColl::ScatterTag,        _scatter_s);
            _mgr.multisend_reg(TSPColl::BarrierTag,        _scatter_b);
            _mgr.multisend_reg(TSPColl::ScattervTag,       _scatterv_s);
            _mgr.multisend_reg(TSPColl::BarrierTag,        _scatterv_b);
            _mgr.multisend_reg(TSPColl::LongAllreduceTag,  _allreduce);
            _mgr.multisend_reg(TSPColl::ShortAllreduceTag, _shortallreduce);
            _mgr.multisend_reg(TSPColl::BarrierTag,        _barrier);
            _mgr.multisend_reg(TSPColl::BarrierUETag,      _barrier_ue);
          }

      inline pami_result_t analyze_impl(size_t context_id,T_Geometry *geometry, int phase)
        {
    if (phase != 0) return PAMI_SUCCESS;

          _nb_barrier    = (TSPColl::Barrier<T_NI>*)_mgr.allocate (geometry, TSPColl::BarrierTag, geometry->comm());
          _nb_ue_barrier = (TSPColl::BarrierUE<T_NI>*)_mgr.allocate (geometry, TSPColl::BarrierUETag, geometry->comm());
          _nb_allgather  = (TSPColl::Allgather<T_NI>*)_mgr.allocate (geometry, TSPColl::AllgatherTag, geometry->comm());
          _nb_allgatherv = (TSPColl::Allgatherv<T_NI>*)_mgr.allocate (geometry, TSPColl::AllgathervTag, geometry->comm());
          _nb_bcast      = (TSPColl::BinomBcast<T_NI>*)_mgr.allocate (geometry, TSPColl::BcastTag, geometry->comm());
          _nb_sar        = (TSPColl::Allreduce::Short<T_NI>*)_mgr.allocate (geometry, TSPColl::ShortAllreduceTag, geometry->comm());
          _nb_lar        = (TSPColl::Allreduce::Long<T_NI>*)_mgr.allocate (geometry, TSPColl::LongAllreduceTag, geometry->comm());
          _nb_sct        = (TSPColl::Scatter<T_NI>*)_mgr.allocate (geometry, TSPColl::ScatterTag, geometry->comm());
          _nb_sctv       = (TSPColl::Scatterv<T_NI>*)_mgr.allocate (geometry, TSPColl::ScattervTag, geometry->comm());
          // todo:  free these on geometry destroy, maybe use KVS
          BarrierFactory        *_barrier_reg        = (BarrierFactory*)_allocator.allocateObject();
          BarrierUEFactory      *_barrier_ue_reg     = (BarrierUEFactory*)_allocator.allocateObject();
          AllgatherFactory      *_allgather_reg      = (AllgatherFactory*)_allocator.allocateObject();
          AllgathervFactory     *_allgatherv_reg     = (AllgathervFactory*)_allocator.allocateObject();
          ScatterFactory        *_scatter_reg        = (ScatterFactory*)_allocator.allocateObject();
          ScattervFactory       *_scatterv_reg       = (ScattervFactory*)_allocator.allocateObject();
          AllreduceFactory      *_allreduce_reg      = (AllreduceFactory*)_allocator.allocateObject();
          ShortAllreduceFactory *_shortallreduce_reg = (ShortAllreduceFactory*)_allocator.allocateObject();
          BroadcastFactory      *_broadcast_reg      = (BroadcastFactory*)_allocator.allocateObject();

          new(_barrier_reg)        BarrierFactory(&_dev_p2p, _barrier, _nb_barrier);
          new(_barrier_ue_reg)     BarrierUEFactory(&_dev_p2p, _barrier_ue, _nb_ue_barrier);
          new(_allgather_reg)      AllgatherFactory(&_dev_p2p, _allgather, _nb_allgather);
          new(_allgatherv_reg)     AllgathervFactory(&_dev_p2p, _allgatherv, _nb_allgatherv);
          new(_scatter_reg)        ScatterFactory(&_dev_p2p, _scatter_s, _nb_sct,_nb_barrier, _scatter_b);
          new(_scatterv_reg)       ScattervFactory(&_dev_p2p, _scatterv_s, _nb_sctv,_nb_barrier, _scatter_b);
          new(_allreduce_reg)      AllreduceFactory(&_dev_p2p, _allreduce, _nb_lar);
          new(_shortallreduce_reg) ShortAllreduceFactory(&_dev_p2p, _shortallreduce,_nb_sar);
          new(_broadcast_reg)      BroadcastFactory(&_dev_p2p, _bcast, _nb_bcast);


          geometry->addCollective(PAMI_XFER_BARRIER,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_barrier_reg,
                                  _context_id);
          geometry->addCollective(PAMI_XFER_BARRIER,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_barrier_ue_reg,
                                  _context_id);

          geometry->addCollective(PAMI_XFER_ALLGATHER,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_allgather_reg,
                                  _context_id);

          geometry->addCollective(PAMI_XFER_ALLGATHERV,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_allgatherv_reg,
                                  _context_id);

          geometry->addCollective(PAMI_XFER_SCATTER,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_scatter_reg,
                                  _context_id);

          geometry->addCollective(PAMI_XFER_SCATTERV,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_scatterv_reg,
                                  _context_id);

          geometry->addCollective(PAMI_XFER_ALLREDUCE,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_allreduce_reg,
                                  _context_id);

          geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE,
                                       (CCMI::Adaptor::CollectiveProtocolFactory*)_shortallreduce_reg,
                                       _context_id);

          geometry->addCollective(PAMI_XFER_BROADCAST,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_broadcast_reg,
                                  _context_id);

          geometry->setUEBarrier(_barrier_ue_reg);

          return PAMI_SUCCESS;
        }

        inline pami_result_t analyze_local_impl(size_t context_id,T_Geometry *geometry, uint64_t *out)
          {
            *out = _reduce_val;
            return analyze(context_id, geometry, 0);
          }

        inline pami_result_t analyze_global_impl(size_t context_id,T_Geometry *geometry, uint64_t *in)
          {
            return PAMI_SUCCESS;
          }


        inline pami_result_t setGenericDevice(PAMI::Device::Generic::Device *g)
          {
            _mgr.setGenericDevice(g);
            return PAMI_SUCCESS;
          }

    public:
      pami_client_t               _client;
      pami_context_t              _context;
      size_t                      _client_id;
      size_t                      _context_id;
      T_NBCollMgr                 _mgr;
      uint64_t                    _reduce_val;
      int                        *_dispatch_id;

      // Map of geometry id's to geometry for this client
      std::map<unsigned, pami_geometry_t> *_geometry_map; 
 
      // Native Interface
      T_Device_P2P               &_dev_p2p;
      T_Device_SHMEM             &_dev_shmem;
      T_Allocator                &_proto_alloc;
      T_NI                   *_bcast;
      T_NI                   *_allgather;
      T_NI                   *_allgatherv;
      T_NI                   *_scatter_s;
      T_NI                   *_scatter_b;
      T_NI                   *_scatterv_s;
      T_NI                   *_scatterv_b;
      T_NI                   *_allreduce;
      T_NI                   *_shortallreduce;
      T_NI                   *_barrier;
      T_NI                   *_barrier_ue;


      PAMI::MemoryAllocator<sizeof(Factories),16>  _allocator;
      TSPColl::Barrier<T_NI>                  *_nb_barrier;
      TSPColl::BarrierUE<T_NI>                *_nb_ue_barrier;
      TSPColl::Allgather<T_NI>                *_nb_allgather;
      TSPColl::Allgatherv<T_NI>               *_nb_allgatherv;
      TSPColl::BinomBcast<T_NI>               *_nb_bcast;
      TSPColl::Allreduce::Short<T_NI>         *_nb_sar;
      TSPColl::Allreduce::Long<T_NI>          *_nb_lar;
      TSPColl::Scatter<T_NI>                  *_nb_sct;
      TSPColl::Scatterv<T_NI>                 *_nb_sctv;
    };
  };
};
#endif
