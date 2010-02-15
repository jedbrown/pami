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
#include "SysDep.h"

namespace XMI
{
  extern std::map<unsigned, xmi_geometry_t> geometry_map;
  namespace CollRegistration
  {

    extern const char BarrierString[]        = "PGAS_Barrier";
    extern const char AllgatherString[]      = "PGAS_Allgather";
    extern const char AllgathervString[]     = "PGAS_Allgatherv";
    extern const char ScatterString[]        = "PGAS_Scatter";
    extern const char ScattervString[]       = "PGAS_Scatterv";
    extern const char AllreduceString[]      = "PGAS_Allreduce";
    extern const char ShortAllreduceString[] = "PGAS_ShortAllreduce";
    extern const char BroadcastString[]      = "PGAS_Broadcast";


    template <class T_Geometry,
              class T_Mcast,
              class T_Device,
              class T_NBCollMgr>
    class PGASRegistration :
      public CollRegistration<XMI::CollRegistration::PGASRegistration<T_Geometry,
                                                                      T_Mcast,
                                                                      T_Device,
                                                                      T_NBCollMgr>,
                              T_Geometry>
      {
        typedef PGBarrierExec<TSPColl::Barrier<T_Mcast>, T_Mcast, T_Device> BarExec;
        typedef PGAllgatherExec<TSPColl::Allgather<T_Mcast>, T_Mcast, T_Device> AllgatherExec;
        typedef PGAllgathervExec<TSPColl::Allgatherv<T_Mcast>, T_Mcast, T_Device> AllgathervExec;
        typedef PGScatterExec<TSPColl::Scatter<T_Mcast>, T_Mcast, T_Device,TSPColl::Barrier<T_Mcast> > ScatterExec;
        typedef PGScattervExec<TSPColl::Scatterv<T_Mcast>, T_Mcast, T_Device,TSPColl::Barrier<T_Mcast> > ScattervExec;
        typedef PGAllreduceExec<TSPColl::Allreduce::Long<T_Mcast>, T_Mcast, T_Device> AllreduceExec;
        typedef PGAllreduceExec<TSPColl::Allreduce::Short<T_Mcast>, T_Mcast, T_Device> ShortAllreduceExec;
        typedef PGBroadcastExec<TSPColl::BinomBcast<T_Mcast>, T_Mcast, T_Device> BroadcastExec;
        typedef PGFactory<TSPColl::Barrier<T_Mcast>,T_Mcast,T_Device,BarExec,BarrierString> BarrierFactory;
        typedef PGFactory<TSPColl::Allgather<T_Mcast>,T_Mcast,T_Device,AllgatherExec,AllgatherString>  AllgatherFactory;
        typedef PGFactory<TSPColl::Allgatherv<T_Mcast>,T_Mcast,T_Device,AllgathervExec,AllgathervString> AllgathervFactory;
        typedef PGFactory<TSPColl::Scatter<T_Mcast>,T_Mcast,T_Device,ScatterExec,ScatterString, TSPColl::Barrier<T_Mcast> > ScatterFactory;
        typedef PGFactory<TSPColl::Scatterv<T_Mcast>,T_Mcast,T_Device,ScattervExec,ScattervString, TSPColl::Barrier<T_Mcast> > ScattervFactory;
        typedef PGFactory<TSPColl::Allreduce::Long<T_Mcast>,T_Mcast,T_Device,AllreduceExec,AllreduceString>  AllreduceFactory;
        typedef PGFactory<TSPColl::Allreduce::Short<T_Mcast>,T_Mcast,T_Device,ShortAllreduceExec,ShortAllreduceString> ShortAllreduceFactory;
        typedef PGFactory<TSPColl::BinomBcast<T_Mcast>,T_Mcast,T_Device,BroadcastExec,BroadcastString> BroadcastFactory;

        typedef union Factories
        {
          char barrier_blob[sizeof(BarrierFactory)];
          char allgather_blob[sizeof(AllgatherFactory)];
          char allgatherv_blob[sizeof(AllgathervFactory)];
          char scatter_blob[sizeof(ScatterFactory)];
          char scatterv_blob[sizeof(ScattervFactory)];
          char allreduce_blob[sizeof(AllreduceFactory)];
          char shortallreduce_blob[sizeof(ShortAllreduceFactory)];
          char broadcast_blob[sizeof(BroadcastFactory)];
        }Factories;

      public:
      inline PGASRegistration(xmi_client_t       client,
                              xmi_context_t      context,
                              size_t             context_id,
                              T_Device          &dev):
        CollRegistration<XMI::CollRegistration::PGASRegistration<T_Geometry,
                                                                 T_Mcast,
                                                                 T_Device,
                                                                 T_NBCollMgr>,
                         T_Geometry> (),
        _client(client),
        _context(context),
        _context_id(context_id),
        _dev(dev),
        _bcast(_dev),
        _allgather(_dev),
        _allgatherv(_dev),
        _scatter_s(_dev),
        _scatter_b(_dev),
        _scatterv_s(_dev),
        _scatterv_b(_dev),
        _allreduce(_dev),
        _shortallreduce(_dev),
        _barrier(_dev)
          {
            _mgr.initialize();
            _mgr.multisend_reg(TSPColl::BcastTag, &_bcast);
            _mgr.multisend_reg(TSPColl::AllgatherTag, &_allgather);
            _mgr.multisend_reg(TSPColl::AllgathervTag, &_allgatherv);
            _mgr.multisend_reg(TSPColl::ScatterTag, &_scatter_s);
            _mgr.multisend_reg(TSPColl::BarrierTag, &_scatter_b);
            _mgr.multisend_reg(TSPColl::ScattervTag, &_scatterv_s);
            _mgr.multisend_reg(TSPColl::BarrierTag, &_scatterv_b);
            _mgr.multisend_reg(TSPColl::LongAllreduceTag, &_allreduce);
            _mgr.multisend_reg(TSPColl::ShortAllreduceTag, &_shortallreduce);
            _mgr.multisend_reg(TSPColl::BarrierTag, &_barrier);
          }

      inline xmi_result_t analyze_impl(size_t context_id,T_Geometry *geometry)
        {

          _nb_barrier    = (TSPColl::Barrier<T_Mcast>*)_mgr.allocate (geometry, TSPColl::BarrierTag);
          _nb_allgather  = (TSPColl::Allgather<T_Mcast>*)_mgr.allocate (geometry, TSPColl::AllgatherTag);
          _nb_allgatherv = (TSPColl::Allgatherv<T_Mcast>*)_mgr.allocate (geometry, TSPColl::AllgathervTag);
          _nb_bcast      = (TSPColl::BinomBcast<T_Mcast>*)_mgr.allocate (geometry, TSPColl::BcastTag);
          _nb_sar        = (TSPColl::Allreduce::Short<T_Mcast>*)_mgr.allocate (geometry, TSPColl::ShortAllreduceTag);
          _nb_lar        = (TSPColl::Allreduce::Long<T_Mcast>*)_mgr.allocate (geometry, TSPColl::LongAllreduceTag);
          _nb_sct        = (TSPColl::Scatter<T_Mcast>*)_mgr.allocate (geometry, TSPColl::ScatterTag);
          _nb_sctv       = (TSPColl::Scatterv<T_Mcast>*)_mgr.allocate (geometry, TSPColl::ScattervTag);
          // todo:  free these on geometry destroy, maybe use KVS
          BarrierFactory        *_barrier_reg = (BarrierFactory*)_allocator.allocateObject();
          AllgatherFactory      *_allgather_reg = (AllgatherFactory*)_allocator.allocateObject();
          AllgathervFactory     *_allgatherv_reg = (AllgathervFactory*)_allocator.allocateObject();
          ScatterFactory        *_scatter_reg = (ScatterFactory*)_allocator.allocateObject();
          ScattervFactory       *_scatterv_reg = (ScattervFactory*)_allocator.allocateObject();
          AllreduceFactory      *_allreduce_reg = (AllreduceFactory*)_allocator.allocateObject();
          ShortAllreduceFactory *_shortallreduce_reg = (ShortAllreduceFactory*)_allocator.allocateObject();
          BroadcastFactory      *_broadcast_reg = (BroadcastFactory*)_allocator.allocateObject();

          new(_barrier_reg)        BarrierFactory(&_dev, &_barrier, _nb_barrier);
          new(_allgather_reg)      AllgatherFactory(&_dev, &_allgather, _nb_allgather);
          new(_allgatherv_reg)     AllgathervFactory(&_dev, &_allgatherv, _nb_allgatherv);
          new(_scatter_reg)        ScatterFactory(&_dev, &_scatter_s, _nb_sct,_nb_barrier, &_scatter_b);
          new(_scatterv_reg)       ScattervFactory(&_dev, &_scatterv_s, _nb_sctv,_nb_barrier, &_scatter_b);
          new(_allreduce_reg)      AllreduceFactory(&_dev, &_allreduce, _nb_lar);
          new(_shortallreduce_reg) ShortAllreduceFactory(&_dev, &_shortallreduce,_nb_sar);
          new(_broadcast_reg)      BroadcastFactory(&_dev, &_bcast, _nb_bcast);


          geometry->addCollective(XMI_XFER_BARRIER,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_barrier_reg,
                                  _context_id);
          geometry->addCollective(XMI_XFER_ALLGATHER,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_allgather_reg,
                                  _context_id);

          geometry->addCollective(XMI_XFER_ALLGATHERV,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_allgatherv_reg,
                                  _context_id);

          geometry->addCollective(XMI_XFER_SCATTER,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_scatter_reg,
                                  _context_id);

          geometry->addCollective(XMI_XFER_SCATTERV,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_scatterv_reg,
                                  _context_id);

          geometry->addCollective(XMI_XFER_ALLREDUCE,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_allreduce_reg,
                                  _context_id);

          geometry->addCollectiveCheck(XMI_XFER_ALLREDUCE,
                                       (CCMI::Adaptor::CollectiveProtocolFactory*)_shortallreduce_reg,
                                       _context_id);

          geometry->addCollective(XMI_XFER_BROADCAST,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_broadcast_reg,
                                  _context_id);
          return XMI_SUCCESS;
        }

      static xmi_geometry_t mapidtogeometry (int comm)
        {
          xmi_geometry_t g = geometry_map[comm];
          return g;
        }

    public:
      xmi_client_t               _client;
      xmi_context_t              _context;
      size_t                     _context_id;
      T_NBCollMgr                _mgr;

      // Native Interface
      T_Device                  &_dev;
      T_Mcast                    _bcast;
      T_Mcast                    _allgather;
      T_Mcast                    _allgatherv;
      T_Mcast                    _scatter_s;
      T_Mcast                    _scatter_b;
      T_Mcast                    _scatterv_s;
      T_Mcast                    _scatterv_b;
      T_Mcast                    _allreduce;
      T_Mcast                    _shortallreduce;
      T_Mcast                    _barrier;



      XMI::MemoryAllocator<sizeof(Factories),16> _allocator;


      TSPColl::Barrier<T_Mcast>          *_nb_barrier;
      TSPColl::Allgather<T_Mcast>        *_nb_allgather;
      TSPColl::Allgatherv<T_Mcast>       *_nb_allgatherv;
      TSPColl::BinomBcast<T_Mcast>       *_nb_bcast;
      TSPColl::Allreduce::Short<T_Mcast> *_nb_sar;
      TSPColl::Allreduce::Long<T_Mcast>  *_nb_lar;
      TSPColl::Scatter<T_Mcast>          *_nb_sct;
      TSPColl::Scatterv<T_Mcast>         *_nb_sctv;
    };
  };
};
#endif
