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


#define SETUPNI_P2P(NI_PTR) result =                                            \
  NativeInterfaceCommon::constructNativeInterface                               \
  <T_Allocator, T_NI, T_Protocol1, T_Device_P2P,                                \
   NativeInterfaceCommon::P2P_ONLY>(                                            \
                                         _proto_alloc,                          \
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
    static const char BarrierString[]        = "I1:Barrier:P2P:P2P";
    static const char BroadcastString[]      = "I1:Broadcast:P2P:P2P";
    static const char AllgatherString[]      = "I1:Allgather:P2P:P2P";
    static const char AllgathervString[]     = "I1:Allgatherv:P2P:P2P";
    static const char ScatterString[]        = "I1:Scatter:P2P:P2P";
    static const char ScattervString[]       = "I1:Scatterv:P2P:P2P";
    static const char GatherString[]         = "I1:Gather:P2P:P2P";
    static const char AlltoallString[]       = "I1:Alltoall:P2P:P2P";
    static const char AlltoallvString[]      = "I1:Alltoallv:P2P:P2P";
    static const char AllreduceString[]      = "I1:Allreduce:P2P:P2P";
    static const char ScanString[]           = "I1:Scan:P2P:P2P";
    static const char ShortAllreduceString[] = "I1:ShortAllreduce:P2P:P2P";

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
        typedef PGBarrierExec<T_Geometry,xlpgas::Barrier<T_NI>, T_NI, T_Device_P2P> BarExec;
        typedef PGBroadcastExec<T_Geometry,xlpgas::Broadcast<T_NI>, T_NI, T_Device_P2P> BroadcastExec;
        typedef PGAllgatherExec<T_Geometry,xlpgas::Allgather<T_NI>, T_NI, T_Device_P2P> AllgatherExec;
        typedef PGAllgathervExec<T_Geometry,xlpgas::Allgatherv<T_NI>, T_NI, T_Device_P2P> AllgathervExec;
        typedef PGScatterExec<T_Geometry,xlpgas::Scatter<T_NI>, T_NI, T_Device_P2P,xlpgas::Barrier<T_NI> > ScatterExec;
        typedef PGGatherExec<T_Geometry,xlpgas::Gather<T_NI>, T_NI, T_Device_P2P,xlpgas::Barrier<T_NI> > GatherExec;
        typedef PGAlltoallExec<T_Geometry,xlpgas::Alltoall<T_NI>, T_NI, T_Device_P2P,xlpgas::Barrier<T_NI> > AlltoallExec;
        typedef PGAlltoallvExec<T_Geometry,xlpgas::Alltoallv<T_NI>, T_NI, T_Device_P2P,xlpgas::Barrier<T_NI> > AlltoallvExec;
        typedef PGAllreduceExec<T_Geometry,xlpgas::Allreduce::Long<T_NI>, T_NI, T_Device_P2P> AllreduceExec;
        typedef PGScanExec<T_Geometry,xlpgas::PrefixSums<T_NI>, T_NI, T_Device_P2P> ScanExec;
        typedef PGAllreduceExec<T_Geometry,xlpgas::Allreduce::Short<T_NI>, T_NI, T_Device_P2P> ShortAllreduceExec;

        typedef PGFactory<xlpgas::Barrier<T_NI>,T_NI,T_Device_P2P,BarExec> BarrierFactory;
        typedef PGFactory<xlpgas::Broadcast<T_NI>,T_NI,T_Device_P2P,BroadcastExec> BroadcastFactory;
        typedef PGFactory<xlpgas::Allgather<T_NI>,T_NI,T_Device_P2P,AllgatherExec>  AllgatherFactory;
        typedef PGFactory<xlpgas::Allgatherv<T_NI>,T_NI,T_Device_P2P,AllgathervExec> AllgathervFactory;
        typedef PGFactory<xlpgas::Scatter<T_NI>,T_NI,T_Device_P2P,ScatterExec, xlpgas::Barrier<T_NI> > ScatterFactory;
        typedef PGFactory<xlpgas::Gather<T_NI>,T_NI,T_Device_P2P,GatherExec, xlpgas::Barrier<T_NI> > GatherFactory;
        typedef PGFactory<xlpgas::Alltoall<T_NI>,T_NI,T_Device_P2P,AlltoallExec, xlpgas::Barrier<T_NI> > AlltoallFactory;
        typedef PGFactory<xlpgas::Alltoallv<T_NI>,T_NI,T_Device_P2P,AlltoallvExec, xlpgas::Barrier<T_NI> > AlltoallvFactory;
        typedef PGFactory<xlpgas::Allreduce::Long<T_NI>,T_NI,T_Device_P2P,AllreduceExec>  AllreduceFactory;
        typedef PGFactory<xlpgas::PrefixSums<T_NI>,T_NI,T_Device_P2P,ScanExec>  ScanFactory;
        typedef PGFactory<xlpgas::Allreduce::Short<T_NI>,T_NI,T_Device_P2P,ShortAllreduceExec> ShortAllreduceFactory;

        typedef union Factories
        {
          char barrier_blob[sizeof(BarrierFactory)];
          char broadcast_blob[sizeof(BroadcastFactory)];
          char allgather_blob[sizeof(AllgatherFactory)];
          char allgatherv_blob[sizeof(AllgathervFactory)];
          char scatter_blob[sizeof(ScatterFactory)];
          char gather_blob[sizeof(GatherFactory)];
          char alltoall_blob[sizeof(AlltoallFactory)];
          char alltoallv_blob[sizeof(AlltoallvFactory)];
          char allreduce_blob[sizeof(AllreduceFactory)];
          char scan_blob[sizeof(ScanFactory)];
          char shortallreduce_blob[sizeof(ShortAllreduceFactory)];
        }Factories;

      public:
      /// \brief The one device (p2p) pgas constructor.
      /// 
      /// It takes two devices because this class uses references to the device so *something* has
      /// to be passed in even if it's ignored.
      ///
      inline PGASRegistration(pami_client_t                        client,
                              pami_context_t                       context,
                              size_t                               client_id,
                              size_t                               context_id,
                              T_Allocator                         &proto_alloc,
                              T_Device_P2P                        &dev_p2p,
                              T_Device_SHMEM                      &dev_shmem,
                              int                                 *dispatch_id,
                              std::map<unsigned, pami_geometry_t> *geometry_map):
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
        _mgr(_context_id),
        _reduce_val(0),
        _dispatch_id(dispatch_id),
        _geometry_map(geometry_map),
        _dev_p2p(dev_p2p),
        _dev_shmem(dev_shmem),
        _proto_alloc(proto_alloc),
        _allgather(NULL),
        _allgatherv(NULL),
        _scatter_s(NULL),
        _scatter_b(NULL),
        _gather_s(NULL),
        _gather_b(NULL),
        _alltoall_s(NULL),
        _alltoall_b(NULL),
        _alltoallv_s(NULL),
        _alltoallv_b(NULL),
        _allreduce(NULL),
        _shortallreduce(NULL),
        _barrier(NULL),
        _broadcast(NULL)
          {
            pami_result_t       result   = PAMI_SUCCESS;

            SETUPNI_P2P(_allgather);
            SETUPNI_P2P(_allgatherv);
            SETUPNI_P2P(_scatter_s);
            SETUPNI_P2P(_scatter_b);
            SETUPNI_P2P(_gather_s);
            SETUPNI_P2P(_gather_b);
            SETUPNI_P2P(_alltoall_s);
            SETUPNI_P2P(_alltoall_b);
            SETUPNI_P2P(_alltoallv_s);
            SETUPNI_P2P(_alltoallv_b);
            SETUPNI_P2P(_allreduce);
            SETUPNI_P2P(_scan);
            SETUPNI_P2P(_shortallreduce);
            SETUPNI_P2P(_barrier);
            SETUPNI_P2P(_broadcast);
            
            _mgr.Initialize(1); // Initialize for 1 context
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::AllgatherKind,      _allgather);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::AllgathervKind,     _allgatherv);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::ScatterKind,        _scatter_s);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BarrierKind,        _scatter_b);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::GatherKind,         _gather_s);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BarrierKind,        _gather_b);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::AlltoallKind,       _alltoall_s);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BarrierKind,        _alltoall_b);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::AlltoallvKind,      _alltoallv_s);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BarrierKind,        _alltoallv_b);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::LongAllreduceKind,  _allreduce);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::PrefixKind,         _scan);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::ShortAllreduceKind, _shortallreduce);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BarrierKind,        _barrier);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BcastKind,          _broadcast);
          }

      /// \brief The two device (p2p+shmem) pgas constructor with a bool flag to enable it.
      inline PGASRegistration(pami_client_t                        client,
                              pami_context_t                       context,
                              size_t                               client_id,
                              size_t                               context_id,
                              T_Allocator                         &proto_alloc,
                              T_Device_P2P                        &dev_p2p,
                              T_Device_SHMEM                      &dev_shmem,
                              int                                 *dispatch_id,
                              std::map<unsigned, pami_geometry_t> *geometry_map,
                              bool                                 use_shmem):
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
        _mgr(_context_id),
        _reduce_val(0),
        _dispatch_id(dispatch_id),
        _geometry_map(geometry_map),
        _dev_p2p(dev_p2p),
        _dev_shmem(dev_shmem),
        _proto_alloc(proto_alloc),
        _allgather(NULL),
        _allgatherv(NULL),
        _scatter_s(NULL),
        _scatter_b(NULL),
        _gather_s(NULL),
        _gather_b(NULL),
        _alltoall_s(NULL),
        _alltoall_b(NULL),
        _alltoallv_s(NULL),
        _alltoallv_b(NULL),
        _allreduce(NULL),
        _scan(NULL),
        _shortallreduce(NULL),
        _barrier(NULL),
        _broadcast(NULL)
          {
            pami_result_t       result   = PAMI_SUCCESS;

            if(use_shmem == true)
              {
                SETUPNI_P2P_SHMEM(_allgather);
                SETUPNI_P2P_SHMEM(_allgatherv);
                SETUPNI_P2P_SHMEM(_scatter_s);
                SETUPNI_P2P_SHMEM(_scatter_b);
                SETUPNI_P2P_SHMEM(_gather_s);
                SETUPNI_P2P_SHMEM(_gather_b);
                SETUPNI_P2P_SHMEM(_alltoall_s);
                SETUPNI_P2P_SHMEM(_alltoall_b);
                SETUPNI_P2P_SHMEM(_alltoallv_s);
                SETUPNI_P2P_SHMEM(_alltoallv_b);
                SETUPNI_P2P_SHMEM(_allreduce);
                SETUPNI_P2P_SHMEM(_scan);
                SETUPNI_P2P_SHMEM(_shortallreduce);
                SETUPNI_P2P_SHMEM(_barrier);
                SETUPNI_P2P_SHMEM(_broadcast);
              }
            else
              {
                SETUPNI_P2P_P2P(_allgather);
                SETUPNI_P2P_P2P(_allgatherv);
                SETUPNI_P2P_P2P(_scatter_s);
                SETUPNI_P2P_P2P(_scatter_b);
                SETUPNI_P2P_P2P(_gather_s);
                SETUPNI_P2P_P2P(_gather_b);
                SETUPNI_P2P_P2P(_alltoall_s);
                SETUPNI_P2P_P2P(_alltoall_b);
                SETUPNI_P2P_P2P(_alltoallv_s);
                SETUPNI_P2P_P2P(_alltoallv_b);
                SETUPNI_P2P_P2P(_allreduce);
                SETUPNI_P2P_P2P(_scan);
                SETUPNI_P2P_P2P(_shortallreduce);
                SETUPNI_P2P_P2P(_barrier);
                SETUPNI_P2P_P2P(_broadcast);
              }
            
            _mgr.Initialize(1); // Initialize for 1 context
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::AllgatherKind,      _allgather);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::AllgathervKind,     _allgatherv);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::ScatterKind,        _scatter_s);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BarrierKind,        _scatter_b);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::GatherKind,         _gather_s);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BarrierKind,        _gather_b);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::AlltoallKind,       _alltoall_s);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BarrierKind,        _alltoall_b);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::AlltoallvKind,      _alltoallv_s);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BarrierKind,        _alltoallv_b);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::LongAllreduceKind,  _allreduce);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::PrefixKind,         _scan);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::ShortAllreduceKind, _shortallreduce);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BarrierKind,        _barrier);
            _mgr.template multisend_reg<xlpgas::base_coll_defs<T_NI> >(xlpgas::BcastKind,          _broadcast);
          }

      inline pami_result_t analyze_impl(size_t context_id,T_Geometry *geometry, int phase)
        {
          if (phase != 0) return PAMI_SUCCESS;

          _nb_barrier         = (xlpgas::Barrier<T_NI>*)_mgr.template allocate<xlpgas::base_coll_defs<T_NI> > (geometry, xlpgas::BarrierKind, geometry->comm());
          _nb_broadcast       = (xlpgas::Broadcast<T_NI>*)_mgr.template allocate<xlpgas::base_coll_defs<T_NI> > (geometry, xlpgas::BcastKind, geometry->comm());
          _nb_allgather       = (xlpgas::Allgather<T_NI>*)_mgr.template allocate<xlpgas::base_coll_defs<T_NI> > (geometry, xlpgas::AllgatherKind, geometry->comm());
          _nb_allgatherv      = (xlpgas::Allgatherv<T_NI>*)_mgr.template allocate<xlpgas::base_coll_defs<T_NI> > (geometry, xlpgas::AllgathervKind, geometry->comm());
          _nb_short_allreduce = (xlpgas::Allreduce::Short<T_NI>*)_mgr.template allocate<xlpgas::base_coll_defs<T_NI> > (geometry, xlpgas::ShortAllreduceKind, geometry->comm());
          _nb_long_allreduce  = (xlpgas::Allreduce::Long<T_NI>*)_mgr.template allocate<xlpgas::base_coll_defs<T_NI> > (geometry, xlpgas::LongAllreduceKind, geometry->comm());
          _nb_scan            = (xlpgas::PrefixSums<T_NI>*)_mgr.template allocate<xlpgas::base_coll_defs<T_NI> > (geometry, xlpgas::PrefixKind, geometry->comm());
          _nb_scatter         = (xlpgas::Scatter<T_NI>*)_mgr.template allocate<xlpgas::base_coll_defs<T_NI> > (geometry, xlpgas::ScatterKind, geometry->comm());
          _nb_gather          = (xlpgas::Gather<T_NI>*)_mgr.template allocate<xlpgas::base_coll_defs<T_NI> > (geometry, xlpgas::GatherKind, geometry->comm());
          _nb_alltoall        = (xlpgas::Alltoall<T_NI>*)_mgr.template allocate<xlpgas::base_coll_defs<T_NI> > (geometry, xlpgas::AlltoallKind, geometry->comm());
          _nb_alltoallv       = (xlpgas::Alltoallv<T_NI>*)_mgr.template allocate<xlpgas::base_coll_defs<T_NI> > (geometry, xlpgas::AlltoallvKind, geometry->comm());

          // todo:  free these on geometry destroy, maybe use KVS
          BarrierFactory        *_barrier_reg        = (BarrierFactory*)_allocator.allocateObject();
          BroadcastFactory      *_broadcast_reg      = (BroadcastFactory*)_allocator.allocateObject();
          AllgatherFactory      *_allgather_reg      = (AllgatherFactory*)_allocator.allocateObject();
          AllgathervFactory     *_allgatherv_reg     = (AllgathervFactory*)_allocator.allocateObject();
          ScatterFactory        *_scatter_reg        = (ScatterFactory*)_allocator.allocateObject();
          GatherFactory         *_gather_reg         = (GatherFactory*)_allocator.allocateObject();
          AlltoallFactory       *_alltoall_reg       = (AlltoallFactory*)_allocator.allocateObject();
          AlltoallvFactory      *_alltoallv_reg      = (AlltoallvFactory*)_allocator.allocateObject();
          AllreduceFactory      *_allreduce_reg      = (AllreduceFactory*)_allocator.allocateObject();
          ScanFactory           *_scan_reg           = (ScanFactory*)_allocator.allocateObject();
          ShortAllreduceFactory *_shortallreduce_reg = (ShortAllreduceFactory*)_allocator.allocateObject();

          new(_barrier_reg)        BarrierFactory(&_dev_p2p, _barrier, _nb_barrier, BarrierString);
          new(_broadcast_reg)      BroadcastFactory(&_dev_p2p, _broadcast, _nb_broadcast, BroadcastString);
          new(_allgather_reg)      AllgatherFactory(&_dev_p2p, _allgather, _nb_allgather, AllgatherString);
          new(_allgatherv_reg)     AllgathervFactory(&_dev_p2p, _allgatherv, _nb_allgatherv, AllgathervString);
          new(_scatter_reg)        ScatterFactory(&_dev_p2p, _scatter_s, _nb_scatter, ScatterString, _nb_barrier, _scatter_b);
          new(_gather_reg)         GatherFactory(&_dev_p2p, _gather_s, _nb_gather, GatherString, _nb_barrier, _gather_b);
          new(_alltoall_reg)       AlltoallFactory(&_dev_p2p, _alltoall_s, _nb_alltoall, AlltoallString, _nb_barrier, _alltoall_b);
          new(_alltoallv_reg)      AlltoallvFactory(&_dev_p2p, _alltoallv_s, _nb_alltoallv, AlltoallvString, _nb_barrier, _alltoallv_b);
          new(_allreduce_reg)      AllreduceFactory(&_dev_p2p, _allreduce, _nb_long_allreduce, AllreduceString);
          new(_scan_reg)           ScanFactory(&_dev_p2p, _scan, _nb_scan, ScanString);
          new(_shortallreduce_reg) ShortAllreduceFactory(&_dev_p2p, _shortallreduce,_nb_short_allreduce, ShortAllreduceString);

          geometry->addCollective(PAMI_XFER_BARRIER,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_barrier_reg,
                                  _context_id);

          geometry->addCollective(PAMI_XFER_BROADCAST,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_broadcast_reg,
                                  _context_id);

          geometry->addCollective(PAMI_XFER_ALLGATHER,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_allgather_reg,
                                  _context_id);

          geometry->addCollective(PAMI_XFER_ALLGATHERV,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_allgatherv_reg,
                                  _context_id);
#if 0
          // Scatter broken now in pgas rt
          geometry->addCollective(PAMI_XFER_SCATTER,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_scatter_reg,
                                  _context_id);
          // Gather broken now in pgas rt
          geometry->addCollective(PAMI_XFER_GATHER,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_gather_reg,
                                  _context_id);
#endif
          geometry->addCollective(PAMI_XFER_ALLTOALL,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_alltoall_reg,
                                  _context_id);

          geometry->addCollective(PAMI_XFER_ALLTOALLV,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_alltoallv_reg,
                                  _context_id);

          geometry->addCollective(PAMI_XFER_ALLREDUCE,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_allreduce_reg,
                                  _context_id);

          geometry->addCollective(PAMI_XFER_SCAN,
                                  (CCMI::Adaptor::CollectiveProtocolFactory*)_scan_reg,
                                  _context_id);

          geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE,
                                       (CCMI::Adaptor::CollectiveProtocolFactory*)_shortallreduce_reg,
                                       _context_id);

          return PAMI_SUCCESS;
        }

      inline pami_result_t register_local_impl(size_t context_id,T_Geometry *geometry, uint64_t *out, int &n)
          {
	    n = 0;
            //*out = _reduce_val;
            return analyze(context_id, geometry, 0);
          }

      inline pami_result_t receive_global_impl(size_t context_id,T_Geometry *geometry, uint64_t *in, int n)
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
      T_NI                   *_allgather;
      T_NI                   *_allgatherv;
      T_NI                   *_scatter_s;
      T_NI                   *_scatter_b;
      T_NI                   *_gather_s;
      T_NI                   *_gather_b;
      T_NI                   *_alltoall_s;
      T_NI                   *_alltoall_b;
      T_NI                   *_alltoallv_s;
      T_NI                   *_alltoallv_b;
      T_NI                   *_allreduce;
      T_NI                   *_scan;
      T_NI                   *_shortallreduce;
      T_NI                   *_barrier;
      T_NI                   *_broadcast;

      PAMI::MemoryAllocator<sizeof(Factories),16,16>  _allocator;
      xlpgas::Barrier<T_NI>                  *_nb_barrier;
      xlpgas::Broadcast<T_NI>                *_nb_broadcast;
      xlpgas::Allgather<T_NI>                *_nb_allgather;
      xlpgas::Allgatherv<T_NI>               *_nb_allgatherv;
      xlpgas::Allreduce::Short<T_NI>         *_nb_short_allreduce;
      xlpgas::Allreduce::Long<T_NI>          *_nb_long_allreduce;
      xlpgas::PrefixSums<T_NI>               *_nb_scan;
      xlpgas::Scatter<T_NI>                  *_nb_scatter;
      xlpgas::Gather<T_NI>                   *_nb_gather;
      xlpgas::Alltoall<T_NI>                 *_nb_alltoall;
      xlpgas::Alltoallv<T_NI>                *_nb_alltoallv;
    };
  };
};
#endif
