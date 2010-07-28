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
#include "SysDep.h"

namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;
  namespace CollRegistration
  {

    extern const char BarrierString[]        ;
    extern const char BarrierUEString[]      ;
    extern const char AllgatherString[]      ;
    extern const char AllgathervString[]     ;
    extern const char ScatterString[]        ;
    extern const char ScattervString[]       ;
    extern const char AllreduceString[]      ;
    extern const char ShortAllreduceString[] ;
    extern const char BroadcastString[]      ;


    template <class T_Geometry,
              class T_P2P_NI,
              class T_Allocator,
              class T_Protocol,
              class T_Device,
              class T_NBCollMgr>
    class PGASRegistration :
      public CollRegistration<PAMI::CollRegistration::PGASRegistration<T_Geometry,
                                                                       T_P2P_NI,
                                                                       T_Allocator,
                                                                       T_Protocol,
                                                                       T_Device,
                                                                       T_NBCollMgr>,
                              T_Geometry>
      {
        typedef PGBarrierExec<T_Geometry,TSPColl::Barrier<T_P2P_NI>, T_P2P_NI, T_Device> BarExec;
        typedef PGBarrierExec<T_Geometry,TSPColl::BarrierUE<T_P2P_NI>, T_P2P_NI, T_Device> BarUEExec;
        typedef PGAllgatherExec<T_Geometry,TSPColl::Allgather<T_P2P_NI>, T_P2P_NI, T_Device> AllgatherExec;
        typedef PGAllgathervExec<T_Geometry,TSPColl::Allgatherv<T_P2P_NI>, T_P2P_NI, T_Device> AllgathervExec;
        typedef PGScatterExec<T_Geometry,TSPColl::Scatter<T_P2P_NI>, T_P2P_NI, T_Device,TSPColl::Barrier<T_P2P_NI> > ScatterExec;
        typedef PGScattervExec<T_Geometry,TSPColl::Scatterv<T_P2P_NI>, T_P2P_NI, T_Device,TSPColl::Barrier<T_P2P_NI> > ScattervExec;
        typedef PGAllreduceExec<T_Geometry,TSPColl::Allreduce::Long<T_P2P_NI>, T_P2P_NI, T_Device> AllreduceExec;
        typedef PGAllreduceExec<T_Geometry,TSPColl::Allreduce::Short<T_P2P_NI>, T_P2P_NI, T_Device> ShortAllreduceExec;
        typedef PGBroadcastExec<T_Geometry,TSPColl::BinomBcast<T_P2P_NI>, T_P2P_NI, T_Device> BroadcastExec;

        typedef PGFactory<TSPColl::Barrier<T_P2P_NI>,T_P2P_NI,T_Device,BarExec,BarrierString> BarrierFactory;
        typedef PGFactory<TSPColl::BarrierUE<T_P2P_NI>,T_P2P_NI,T_Device,BarUEExec,BarrierUEString> BarrierUEFactory;
        typedef PGFactory<TSPColl::Allgather<T_P2P_NI>,T_P2P_NI,T_Device,AllgatherExec,AllgatherString>  AllgatherFactory;
        typedef PGFactory<TSPColl::Allgatherv<T_P2P_NI>,T_P2P_NI,T_Device,AllgathervExec,AllgathervString> AllgathervFactory;
        typedef PGFactory<TSPColl::Scatter<T_P2P_NI>,T_P2P_NI,T_Device,ScatterExec,ScatterString, TSPColl::Barrier<T_P2P_NI> > ScatterFactory;
        typedef PGFactory<TSPColl::Scatterv<T_P2P_NI>,T_P2P_NI,T_Device,ScattervExec,ScattervString, TSPColl::Barrier<T_P2P_NI> > ScattervFactory;
        typedef PGFactory<TSPColl::Allreduce::Long<T_P2P_NI>,T_P2P_NI,T_Device,AllreduceExec,AllreduceString>  AllreduceFactory;
        typedef PGFactory<TSPColl::Allreduce::Short<T_P2P_NI>,T_P2P_NI,T_Device,ShortAllreduceExec,ShortAllreduceString> ShortAllreduceFactory;
        typedef PGFactory<TSPColl::BinomBcast<T_P2P_NI>,T_P2P_NI,T_Device,BroadcastExec,BroadcastString> BroadcastFactory;

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
      inline PGASRegistration(pami_client_t       client,
                              pami_context_t      context,
                              size_t              client_id,
                              size_t              context_id,
                              T_Allocator        &proto_alloc,
                              T_Device           &dev):
        CollRegistration<PAMI::CollRegistration::PGASRegistration<T_Geometry,
                                                                  T_P2P_NI,
                                                                  T_Allocator,
                                                                  T_Protocol,
                                                                  T_Device,
                                                                  T_NBCollMgr>,
                         T_Geometry> (),
        _client(client),
        _context(context),
        _client_id(client_id),
        _context_id(context_id),
        _dev(dev),
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


#define SETUPNI(NI) result   = NativeInterfaceCommon::constructNativeInterface             \
                               <T_Allocator, T_P2P_NI, T_Protocol, T_Device>(_proto_alloc, \
                                                                             _dev,         \
                                                                             NI,           \
                                                                             _client,      \
                                                                             _context,     \
                                                                             _context_id,  \
                                                                             _client_id)
            SETUPNI(_bcast);
            SETUPNI(_allgather);
            SETUPNI(_allgatherv);
            SETUPNI(_scatter_s);
            SETUPNI(_scatter_b);
            SETUPNI(_scatterv_s);
            SETUPNI(_scatterv_b);
            SETUPNI(_allreduce);
            SETUPNI(_shortallreduce);
            SETUPNI(_barrier);
            SETUPNI(_barrier_ue);

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

          _nb_barrier    = (TSPColl::Barrier<T_P2P_NI>*)_mgr.allocate (geometry, TSPColl::BarrierTag, geometry->comm());
          _nb_ue_barrier = (TSPColl::BarrierUE<T_P2P_NI>*)_mgr.allocate (geometry, TSPColl::BarrierUETag, geometry->comm());
          _nb_allgather  = (TSPColl::Allgather<T_P2P_NI>*)_mgr.allocate (geometry, TSPColl::AllgatherTag, geometry->comm());
          _nb_allgatherv = (TSPColl::Allgatherv<T_P2P_NI>*)_mgr.allocate (geometry, TSPColl::AllgathervTag, geometry->comm());
          _nb_bcast      = (TSPColl::BinomBcast<T_P2P_NI>*)_mgr.allocate (geometry, TSPColl::BcastTag, geometry->comm());
          _nb_sar        = (TSPColl::Allreduce::Short<T_P2P_NI>*)_mgr.allocate (geometry, TSPColl::ShortAllreduceTag, geometry->comm());
          _nb_lar        = (TSPColl::Allreduce::Long<T_P2P_NI>*)_mgr.allocate (geometry, TSPColl::LongAllreduceTag, geometry->comm());
          _nb_sct        = (TSPColl::Scatter<T_P2P_NI>*)_mgr.allocate (geometry, TSPColl::ScatterTag, geometry->comm());
          _nb_sctv       = (TSPColl::Scatterv<T_P2P_NI>*)_mgr.allocate (geometry, TSPColl::ScattervTag, geometry->comm());
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

          new(_barrier_reg)        BarrierFactory(&_dev, _barrier, _nb_barrier);
          new(_barrier_ue_reg)     BarrierUEFactory(&_dev, _barrier_ue, _nb_ue_barrier);
          new(_allgather_reg)      AllgatherFactory(&_dev, _allgather, _nb_allgather);
          new(_allgatherv_reg)     AllgathervFactory(&_dev, _allgatherv, _nb_allgatherv);
          new(_scatter_reg)        ScatterFactory(&_dev, _scatter_s, _nb_sct,_nb_barrier, _scatter_b);
          new(_scatterv_reg)       ScattervFactory(&_dev, _scatterv_s, _nb_sctv,_nb_barrier, _scatter_b);
          new(_allreduce_reg)      AllreduceFactory(&_dev, _allreduce, _nb_lar);
          new(_shortallreduce_reg) ShortAllreduceFactory(&_dev, _shortallreduce,_nb_sar);
          new(_broadcast_reg)      BroadcastFactory(&_dev, _bcast, _nb_bcast);


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

        inline pami_result_t setGenericDevice(PAMI::Device::Generic::Device *g)
          {
            _mgr.setGenericDevice(g);
            return PAMI_SUCCESS;
          }

      static pami_geometry_t mapidtogeometry (int comm)
        {
          pami_geometry_t g = geometry_map[comm];
          return g;
        }

    public:
      pami_client_t               _client;
      pami_context_t              _context;
      size_t                      _client_id;
      size_t                      _context_id;
      T_NBCollMgr                 _mgr;

      // Native Interface
      T_Device                   &_dev;
      T_Allocator                &_proto_alloc;
      T_P2P_NI                   *_bcast;
      T_P2P_NI                   *_allgather;
      T_P2P_NI                   *_allgatherv;
      T_P2P_NI                   *_scatter_s;
      T_P2P_NI                   *_scatter_b;
      T_P2P_NI                   *_scatterv_s;
      T_P2P_NI                   *_scatterv_b;
      T_P2P_NI                   *_allreduce;
      T_P2P_NI                   *_shortallreduce;
      T_P2P_NI                   *_barrier;
      T_P2P_NI                   *_barrier_ue;



      PAMI::MemoryAllocator<sizeof(Factories),16> _allocator;


      TSPColl::Barrier<T_P2P_NI>          *_nb_barrier;
      TSPColl::BarrierUE<T_P2P_NI>        *_nb_ue_barrier;
      TSPColl::Allgather<T_P2P_NI>        *_nb_allgather;
      TSPColl::Allgatherv<T_P2P_NI>       *_nb_allgatherv;
      TSPColl::BinomBcast<T_P2P_NI>       *_nb_bcast;
      TSPColl::Allreduce::Short<T_P2P_NI> *_nb_sar;
      TSPColl::Allreduce::Long<T_P2P_NI>  *_nb_lar;
      TSPColl::Scatter<T_P2P_NI>          *_nb_sct;
      TSPColl::Scatterv<T_P2P_NI>         *_nb_sctv;
    };
  };
};
#endif
