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
#include "TypeDefs.h"
#include "algorithms/geometry/Metadata.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/broadcast/MultiCastComposite.h"
#include "algorithms/protocols/allreduce/MultiCombineComposite.h"
#include "algorithms/protocols/barrier/MultiSyncComposite.h"
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"

#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/connmgr/ColorConnMgr.h"
#include "algorithms/protocols/broadcast/BcastMultiColorCompositeT.h"
#include "algorithms/schedule/MCRect.h"
#include "algorithms/schedule/TorusRect.h"
#include "common/NativeInterface.h"
#include "algorithms/protocols/allgather/AllgatherOnBroadcastT.h"
#include "algorithms/protocols/CachedAllSidedFactoryT.h"

#include "algorithms/geometry/P2PCCMIRegInfo.h"

#include "util/ccmi_debug.h"
#include "util/ccmi_util.h"

#include "util/trace.h"

#ifdef CCMI_TRACE_ALL
  #define DO_TRACE_ENTEREXIT 1
  #define DO_TRACE_DEBUG     1
#else
  #define DO_TRACE_ENTEREXIT 0
  #define DO_TRACE_DEBUG     0
#endif


namespace PAMI
{
  namespace CollRegistration
  {

    /// 
    /// Metadata functions
    /// 
    // The protocol requires T_Aligned buffers.  Call some other T_Function (maybe check dt/op?), then check the alignment.
    template <unsigned T_Send_Alignment_Required, unsigned T_Send_Alignment, unsigned T_Recv_Alignment_Required, unsigned T_Recv_Alignment, pami_metadata_function T_Function>
    inline metadata_result_t align_metadata_function(struct pami_xfer_t *in)
    {
      metadata_result_t result = T_Function(in);
      uint64_t  mask;
      if (T_Send_Alignment_Required)
      {
        mask  = T_Send_Alignment - 1; 
        result.check.align_send_buffer      |= (((uint64_t)in->cmd.xfer_allreduce.sndbuf & (uint64_t)mask) == 0) ? 0:1;
      }
      if (T_Recv_Alignment_Required)
      {
        mask  = T_Recv_Alignment - 1; 
        result.check.align_recv_buffer      |= (((uint64_t)in->cmd.xfer_allreduce.rcvbuf & (uint64_t)mask) == 0) ? 0:1;
      }
      result.check.nonlocal = 1; // because of alignment, this is a non-local result.
      return result;
    }

    // The protocol supports a limited T_Range_Low->T_Range_High.  Call some other T_Function (maybe check dt/op?), then check the range.
    template <size_t T_Range_Low,size_t T_Range_High, pami_metadata_function T_Function>
    inline metadata_result_t range_metadata_function(struct pami_xfer_t *in)
    {
      metadata_result_t result = T_Function(in);

      PAMI::Type::TypeCode * type_obj = (PAMI::Type::TypeCode *)in->cmd.xfer_allreduce.stype;

      /// \todo Support non-contiguous
      assert(type_obj->IsContiguous() &&  type_obj->IsPrimitive());

      size_t dataSent = type_obj->GetAtomSize() * in->cmd.xfer_allreduce.stypecount;
      result.check.range   |= !((dataSent <= T_Range_High) && (dataSent >= T_Range_Low));
      // Is checking rtype really necessary? Eh, why not..
      type_obj = (PAMI::Type::TypeCode *)in->cmd.xfer_allreduce.rtype;

      /// \todo Support non-contiguous
      assert(type_obj->IsContiguous() &&  type_obj->IsPrimitive());

      dataSent = type_obj->GetAtomSize() * in->cmd.xfer_allreduce.rtypecount;
      result.check.range   |= !((dataSent <= T_Range_High) && (dataSent >= T_Range_Low));

      return result;
    }

    // The protocol only supports some dt/op's.  
    // Two functions - one for MU dt/ops, one for current SHMEM dt/ops
    namespace MU
    {
      extern inline metadata_result_t op_dt_metadata_function(struct pami_xfer_t *in)
      {
        const bool support[PAMI_DT_COUNT][PAMI_OP_COUNT] =
        {
          //  PAMI_COPY,    PAMI_NOOP,  PAMI_MAX, PAMI_MIN, PAMI_SUM, PAMI_PROD, PAMI_LAND, PAMI_LOR, PAMI_LXOR, PAMI_BAND, PAMI_BOR, PAMI_BXOR, PAMI_MAXLOC, PAMI_MINLOC,                              
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_BYTE                                     
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_SIGNED_CHAR       
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_SIGNED_SHORT      
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false},//  PAMI_SIGNED_INT        
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false},//  PAMI_SIGNED_LONG       
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false},//  PAMI_SIGNED_LONG_LONG  
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_UNSIGNED_CHAR                             
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_UNSIGNED_SHORT     
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false},//  PAMI_UNSIGNED_INT       
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false},//  PAMI_UNSIGNED_LONG      
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false},//  PAMI_UNSIGNED_LONG_LONG 
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_FLOAT      
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false},//  PAMI_DOUBLE                            
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_LONG_DOUBLE       
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_LOGICAL                  
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_SINGLE_COMPLEX       
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_DOUBLE_COMPLEX                       
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_LOC_2INT       
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_LOC_2FLOAT                            
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_LOC_2DOUBLE        
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_LOC_SHORT_INT      
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false},//  PAMI_LOC_FLOAT_INT     
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false} //  PAMI_LOC_DOUBLE_INT    
        };
        metadata_result_t result = {0};
        uintptr_t op;
        uintptr_t dt;
        TRACE_FN_ENTER();
        if (((uintptr_t)in->cmd.xfer_allreduce.stype >= PAMI::Type::TypeCode::PRIMITIVE_TYPE_COUNT) || ((uintptr_t)in->cmd.xfer_allreduce.op >= PAMI::Type::TypeFunc::PRIMITIVE_FUNC_COUNT))
          result.check.datatype_op = 1; // No user-defined dt/op's

        PAMI::Type::TypeFunc::GetEnums(in->cmd.xfer_allreduce.stype,
                                       in->cmd.xfer_allreduce.op,
                                       dt,
                                       op);
        TRACE_FORMAT("(dt %p/%d,op %p/%d) = %s", in->cmd.xfer_allreduce.stype,(pami_dt)dt, in->cmd.xfer_allreduce.op,(pami_op)op, support[dt][op] ? "true" : "false");
        result.check.datatype_op = support[dt][op]?0:1;
        TRACE_FN_EXIT();
        return(result);
      }
    }

#ifdef PAMI_ENABLE_NEW_SHMEM
    namespace Shmem
    {
      extern inline metadata_result_t op_dt_metadata_function(struct pami_xfer_t *in)
      {
        TRACE_FN_ENTER();
        metadata_result_t result = {0};
        uintptr_t op;
        uintptr_t dt;
        if (((uintptr_t)in->cmd.xfer_allreduce.stype >= PAMI::Type::TypeCode::PRIMITIVE_TYPE_COUNT) || ((uintptr_t)in->cmd.xfer_allreduce.op >= PAMI::Type::TypeFunc::PRIMITIVE_FUNC_COUNT))
          result.check.datatype_op = 1; // No user-defined dt/op's

        PAMI::Type::TypeFunc::GetEnums(in->cmd.xfer_allreduce.stype,
                                       in->cmd.xfer_allreduce.op,
                                       dt,
                                       op);
        pami_dt pdt = (pami_dt) dt;
        pami_op pop = (pami_op) op;
        result.check.datatype_op = ((pdt == PAMI_DOUBLE) && 
                                    ((pop == PAMI_MIN) ||
                                     (pop == PAMI_MAX) ||
                                     (pop == PAMI_SUM))) ?0:1;
        TRACE_FORMAT("(dt %d,op %d) = %s", pdt, pop, result.check.datatype_op ? "true" : "false");
        TRACE_FN_EXIT();
        return(result);
      }
    }
#endif
    //----------------------------------------------------------------------------
    /// Declare our protocol factory templates and their metadata templates
    ///
    /// 'Pure' protocols only work on the specified (Shmem or MU) device.
    ///
    /// 'Composite' protocols combine Shmem/MU devices.
    ///
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    // 'Pure' Shmem allsided multisync
    //----------------------------------------------------------------------------
    extern inline void ShmemMsyncMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiSync:SHMEM:-");
    }

    typedef CCMI::Adaptor::Barrier::BarrierFactoryAllSidedT
    <CCMI::Adaptor::Barrier::MultiSyncComposite<>,
    ShmemMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr,
    PAMI::Geometry::CKEY_BARRIERCOMPOSITE7>
    ShmemMultiSyncFactory;

    //----------------------------------------------------------------------------
    // Optimized (MU) Binomial barrier
    //----------------------------------------------------------------------------
    extern inline void OptBinomialMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:OptBinomial:P2P:P2P");
      m->check_perf.values.hw_accel     = 1;
    }

    extern inline bool opt_binomial_analyze (PAMI_GEOMETRY_CLASS *geometry)
    {
      return true;
    }

    typedef CCMI::Adaptor::Barrier::BarrierT
    < CCMI::Schedule::TopoMultinomial4,
    //CCMI::Schedule::NodeOptTopoMultinomial,
    opt_binomial_analyze,
    PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX,
    PAMI::Geometry::CKEY_BARRIERCOMPOSITE2>
    OptBinomialBarrier;

    typedef CCMI::Adaptor::Barrier::BarrierFactoryT
    < OptBinomialBarrier,
    OptBinomialMetaData,
    CCMI::ConnectionManager::SimpleConnMgr,
    false,
    PAMI::Geometry::CKEY_BARRIERCOMPOSITE2>
    OptBinomialBarrierFactory;


    extern inline void getAlltoallMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:M2MComposite:MU:MU");
      m->check_perf.values.hw_accel     = 1;
    }
    typedef CCMI::Adaptor::All2AllProtocol All2AllProtocol;
    typedef CCMI::Adaptor::All2AllFactoryT <All2AllProtocol, getAlltoallMetaData, CCMI::ConnectionManager::CommSeqConnMgr> All2AllFactory;

    extern inline void getAlltoallvMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:M2MComposite:MU:MU");
      m->check_perf.values.hw_accel     = 1;
    }
    typedef CCMI::Adaptor::All2AllvProtocolLong All2AllProtocolv;
    typedef CCMI::Adaptor::All2AllvFactoryT <All2AllProtocolv, getAlltoallvMetaData, CCMI::ConnectionManager::CommSeqConnMgr> All2AllvFactory;

    typedef CCMI::Adaptor::All2AllvProtocolInt All2AllProtocolv_int;
    typedef CCMI::Adaptor::All2AllvFactoryT <All2AllProtocolv_int, getAlltoallvMetaData, CCMI::ConnectionManager::CommSeqConnMgr> All2AllvFactory_int;

    //----------------------------------------------------------------------------
    // 'Pure' Shmem allsided multicombine
    //----------------------------------------------------------------------------
    extern inline void ShmemMcombMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombine:SHMEM:-");
#ifdef PAMI_ENABLE_NEW_SHMEM
      m->check_correct.values.alldtop     = 0;
      m->check_correct.values.rangeminmax = 1;
      m->range_hi                         = 64; // Msgs > 64 are pseudo-reduce-only, not allreduce
      m->check_fn                         = range_metadata_function<0,64,Shmem::op_dt_metadata_function>;
#endif
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite<>,
    ShmemMcombMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > ShmemMultiCombineFactory;

    //----------------------------------------------------------------------------
    // 'Pure' Shmem allsided multicast
    //----------------------------------------------------------------------------
    extern inline void ShmemMcastMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCast:SHMEM:-");
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite<>,
    ShmemMcastMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > ShmemMultiCastFactory;


    //----------------------------------------------------------------------------
    // 'Pure' MU allsided multicast
    //----------------------------------------------------------------------------
    extern inline void MUMcastMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCast:-:MU");
      m->check_perf.values.hw_accel     = 1;
    }

    // Even though MU Multicast is allsided, it still needs a register call with a dispatch id,
    // so we use the CollectiveProtocolFactoryT instead of the AllSidedCollectiveProtocolFactoryT
    typedef CCMI::Adaptor::CollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite<>,
    MUMcastMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUMultiCastFactory;


    //----------------------------------------------------------------------------
    // 'Pure' MU allsided multisync
    //----------------------------------------------------------------------------
    extern inline void GIMsyncMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiSync:-:GI");
      m->check_perf.values.hw_accel     = 1;
    }

    typedef CCMI::Adaptor::Barrier::BarrierFactoryAllSidedT
    <CCMI::Adaptor::Barrier::MultiSyncComposite<true, MUGlobalDputNI>,
    GIMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr,
    PAMI::Geometry::CKEY_BARRIERCOMPOSITE3>
    GIMultiSyncFactory;

    //----------------------------------------------------------------------------
    // MU rectangle multisync
    //----------------------------------------------------------------------------
    extern inline void MURectangleMsyncMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:RectangleMultiSync:-:MU");
    }

    typedef CCMI::Adaptor::Barrier::BarrierFactoryAllSidedT
    <CCMI::Adaptor::Barrier::MultiSyncComposite<true, MUAxialDputNI,PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX, PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID1>,
    MURectangleMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr,
    PAMI::Geometry::CKEY_BARRIERCOMPOSITE5>
    MURectangleMultiSyncFactory;

    //----------------------------------------------------------------------------
    // 'Pure' MU allsided multicombine
    //----------------------------------------------------------------------------
    extern inline void MUMcombMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombine:-:MU");
      m->check_correct.values.alldtop   = 0;
      m->check_fn                       = MU::op_dt_metadata_function;
      m->check_perf.values.hw_accel     = 1;
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite<>,
    MUMcombMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUMultiCombineFactory;

    //----------------------------------------------------------------------------
    // 'Pure' MU allsided dput multicast
    //----------------------------------------------------------------------------
    extern inline void MUMcastCollectiveDputMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MulticastDput:-:MU");
      m->check_perf.values.hw_accel     = 1;
    }


    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite<true, MUGlobalDputNI>,
    MUMcastCollectiveDputMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUCollectiveDputMulticastFactory;

    //----------------------------------------------------------------------------
    // 'Pure' MU allsided dput multicombine
    //----------------------------------------------------------------------------
    extern inline void MUMcombCollectiveDputMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombineDput:-:MU");
      m->check_correct.values.alldtop   = 0;
      m->check_fn                       = MU::op_dt_metadata_function;
      m->check_perf.values.hw_accel     = 1;
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite<true, MUGlobalDputNI>,
    MUMcombCollectiveDputMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUCollectiveDputMulticombineFactory;

    //----------------------------------------------------------------------------
    // MU+Shmem allsided dput multicombine
    //----------------------------------------------------------------------------
    extern inline void MUShmemMcombCollectiveDputMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombineDput:SHMEM:MU");
#ifdef PAMI_ENABLE_NEW_SHMEM
      m->check_correct.values.alldtop       = 0;
      m->check_correct.values.sendminalign  = 1;
      m->check_correct.values.recvminalign  = 1;
      m->check_correct.values.nonlocal      = 1;
      m->send_min_align                     = 64; 
      m->recv_min_align                     = 64; 
      m->check_fn                           = align_metadata_function<1,64,1,64,Shmem::op_dt_metadata_function>;
      m->check_perf.values.hw_accel         = 1;
#else
      m->check_correct.values.alldtop       = 0;
      m->check_correct.values.sendminalign  = 1;
      m->check_correct.values.recvminalign  = 1;
      m->check_correct.values.nonlocal      = 1;
      m->send_min_align                     = 64; 
      m->recv_min_align                     = 64; 
      m->check_fn                           = align_metadata_function<1,64,1,64,MU::op_dt_metadata_function>;
      m->check_perf.values.hw_accel         = 1;
#endif
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite<true, MUShmemGlobalDputNI>,
    MUShmemMcombCollectiveDputMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUShmemCollectiveDputMulticombineFactory;

#ifdef PAMI_ENABLE_X0_PROTOCOLS
    //----------------------------------------------------------------------------
    // MU allsided multicast built on multicombine (BOR)
    //----------------------------------------------------------------------------
    extern inline void MUMcast3MetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("X0:MultiCast_MultiCombine:-:MU");
      m->check_correct.values.rangeminmax = 1;
      m->range_hi = 1024*1024; /// \todo arbitrary hack for now - it core dumps > 1M
      m->check_perf.values.hw_accel     = 1;

    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite3,
    MUMcast3MetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUMultiCast3Factory;
#endif
    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multisync
    //----------------------------------------------------------------------------
    extern inline void Msync2DMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiSync2Device:SHMEM:MU");
      m->check_perf.values.hw_accel     = 1;
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
    Msync2DMetaData, CCMI::ConnectionManager::SimpleConnMgr >  MultiSync2DeviceFactory;

    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multisync
    //----------------------------------------------------------------------------
    extern inline void Msync2DGIShmemMetaData(pami_metadata_t *m)
    {
      // Apparently MU dput actually uses GI for msync
      new(m) PAMI::Geometry::Metadata("I0:MultiSync2Device:SHMEM:GI");
      m->check_perf.values.hw_accel     = 1;
    }

    typedef CCMI::Adaptor::Barrier::BarrierFactoryAllSidedT
    <CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
    Msync2DGIShmemMetaData,
    CCMI::ConnectionManager::SimpleConnMgr,
    PAMI::Geometry::CKEY_BARRIERCOMPOSITE4>
    MultiSync2DeviceGIShmemFactory;

    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multisync
    //----------------------------------------------------------------------------
    extern inline void Msync2DRectangleDputMetaData(pami_metadata_t *m)
    {
      // Apparently MU dput actually uses GI for msync
      new(m) PAMI::Geometry::Metadata("I0:RectangleMultiSync2Device:SHMEM:MU");
    }

    typedef CCMI::Adaptor::Barrier::BarrierFactoryAllSidedT
    <CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
    Msync2DRectangleDputMetaData,
    CCMI::ConnectionManager::SimpleConnMgr,
    PAMI::Geometry::CKEY_BARRIERCOMPOSITE6>
    MultiSync2DeviceRectangleFactory;


#ifdef PAMI_ENABLE_X0_PROTOCOLS
    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multicast
    //----------------------------------------------------------------------------
    extern inline void Mcast2DMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("X0:MultiCast2Device:SHMEM:MU");
      m->check_perf.values.hw_accel     = 1;
    }
    typedef CCMI::Adaptor::Broadcast::MultiCastComposite2DeviceFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite2Device<PAMI_GEOMETRY_CLASS, true, false>,
    Mcast2DMetaData, CCMI::ConnectionManager::SimpleConnMgr > MultiCast2DeviceFactory;
#endif
    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU-DPUT allsided 2 device multicast
    //----------------------------------------------------------------------------
    extern inline void Mcast2DDputMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCast2DeviceDput:SHMEM:MU");
      m->check_perf.values.hw_accel     = 1;
    }
    typedef CCMI::Adaptor::Broadcast::MultiCastComposite2DeviceFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite2DeviceAS<PAMI_GEOMETRY_CLASS>,
    Mcast2DDputMetaData, CCMI::ConnectionManager::SimpleConnMgr > MultiCast2DeviceDputFactory;

#ifdef PAMI_ENABLE_X0_PROTOCOLS
    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multicombine
    //----------------------------------------------------------------------------
    extern inline void Mcomb2DMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("X0:MultiCombine2Device:SHMEM:MU");
#ifdef PAMI_ENABLE_NEW_SHMEM
      m->check_correct.values.alldtop   = 0;
      m->check_fn                       = Shmem::op_dt_metadata_function;
      m->check_perf.values.hw_accel     = 1;
#else
      m->check_correct.values.alldtop   = 0;
      m->check_fn                       = MU::op_dt_metadata_function;
      m->check_perf.values.hw_accel     = 1;
#endif
    }
    typedef CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2Device<0>,
    Mcomb2DMetaData, CCMI::ConnectionManager::SimpleConnMgr >    MultiCombine2DeviceFactory;
#endif
    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU-DPUT allsided 2 device multicombine
    //----------------------------------------------------------------------------
    extern inline void Mcomb2DDputMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombine2DeviceDput:SHMEM:MU");
#ifdef PAMI_ENABLE_NEW_SHMEM
      m->check_correct.values.alldtop       = 0;
      m->check_correct.values.sendminalign  = 1;
      m->check_correct.values.recvminalign  = 1;
      m->check_correct.values.nonlocal      = 1;
      m->send_min_align                     = 32; 
      m->recv_min_align                     = 32; 
      m->check_fn                           = align_metadata_function<1,32,1,32,Shmem::op_dt_metadata_function>;
      m->check_perf.values.hw_accel         = 1;
#else
      m->check_correct.values.alldtop       = 0;
      m->check_correct.values.sendminalign  = 1;
      m->check_correct.values.recvminalign  = 1;
      m->check_correct.values.nonlocal      = 1;
      m->send_min_align                     = 32; 
      m->recv_min_align                     = 32; 
      m->check_fn                           = align_metadata_function<1,32,1,32,MU::op_dt_metadata_function>;
      m->check_perf.values.hw_accel         = 1;
#endif
    }
    typedef CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2Device<0>,
    Mcomb2DDputMetaData, CCMI::ConnectionManager::SimpleConnMgr >    MultiCombine2DeviceDputFactory;


#ifdef PAMI_ENABLE_X0_PROTOCOLS
    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multicombine with no pipelining
    //----------------------------------------------------------------------------
    extern inline void Mcomb2DMetaDataNP(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("X0:MultiCombine2DeviceNP:SHMEM:MU");
#ifdef PAMI_ENABLE_NEW_SHMEM
      m->check_correct.values.alldtop       = 0;
      m->check_correct.values.sendminalign  = 1;
      m->check_correct.values.recvminalign  = 1;
      m->check_correct.values.nonlocal      = 1;
      m->send_min_align                     = 32; 
      m->recv_min_align                     = 32; 
      m->check_fn                           = align_metadata_function<1,32,1,32,Shmem::op_dt_metadata_function>;
      m->check_perf.values.hw_accel         = 1;
#else
      m->check_correct.values.alldtop       = 0;
      m->check_correct.values.sendminalign  = 1;
      m->check_correct.values.recvminalign  = 1;
      m->check_correct.values.nonlocal      = 1;
      m->send_min_align                     = 32; 
      m->recv_min_align                     = 32; 
      m->check_fn                           = align_metadata_function<1,32,1,32,MU::op_dt_metadata_function>;
      m->check_perf.values.hw_accel         = 1;
#endif
    }
    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceNP,
    Mcomb2DMetaDataNP, CCMI::ConnectionManager::SimpleConnMgr > MultiCombine2DeviceFactoryNP;
#endif
#ifdef PAMI_ENABLE_X0_PROTOCOLS
    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU-DPUT allsided 2 device multicombine with no pipelining
    //----------------------------------------------------------------------------
    extern inline void Mcomb2DDputMetaDataNP(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("X0:MultiCombine2DeviceDputNP:SHMEM:MU");
#ifdef PAMI_ENABLE_NEW_SHMEM
      m->check_correct.values.alldtop       = 0;
      m->check_correct.values.sendminalign  = 1;
      m->check_correct.values.recvminalign  = 1;
      m->check_correct.values.nonlocal      = 1;
      m->send_min_align                     = 32; 
      m->recv_min_align                     = 32; 
      m->check_fn                           = align_metadata_function<1,32,1,32,Shmem::op_dt_metadata_function>;
      m->check_perf.values.hw_accel         = 1;
#else
      m->check_correct.values.alldtop       = 0;
      m->check_correct.values.sendminalign  = 1;
      m->check_correct.values.recvminalign  = 1;
      m->check_correct.values.nonlocal      = 1;
      m->send_min_align                     = 32; 
      m->recv_min_align                     = 32; 
      m->check_fn                           = align_metadata_function<1,32,1,32,MU::op_dt_metadata_function>;
      m->check_perf.values.hw_accel         = 1;
#endif
    }
    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceNP,
    Mcomb2DDputMetaDataNP, CCMI::ConnectionManager::SimpleConnMgr > MultiCombine2DeviceDputFactoryNP;
#endif
    //----------------------------------------------------------------------------
    // Rectangle broadcast
    //----------------------------------------------------------------------------
    extern inline void get_colors (PAMI::Topology             * t,
                                   unsigned                    bytes,
                                   unsigned                  * colors,
                                   unsigned                  & ncolors)
    {
      TRACE_FN_ENTER();
      ncolors = 1;
      colors[0] = CCMI::Schedule::TorusRect::NO_COLOR;
      TRACE_FN_EXIT();
    }

    extern inline void get_rect_colors (PAMI::Topology             * t,
                                        unsigned                    bytes,
                                        unsigned                  * colors,
                                        unsigned                  & ncolors)
    {
      TRACE_FN_ENTER();
      unsigned max = 0, ideal = 0;
      unsigned _colors[10];
      CCMI::Schedule::TorusRect::getColors (t, ideal, max, _colors);
      TRACE_FORMAT("bytes %u, ncolors %u, ideal %u, max %u", bytes, ncolors, ideal, max);

      if (bytes <= 4096) //16 packets
        ideal = 1;
      else if (bytes <= 16384 && ideal >= 2)
        ideal = 2;
      else if (bytes <= 65536 && ideal >= 3)
        ideal = 3;
      else if (bytes <= 262144 && ideal >= 5)
        ideal = 5;
      else if (bytes <= 1048576 && ideal >= 7)
        ideal = 7;

      if (ideal < ncolors)
        ncolors = ideal;  //Reduce the number of colors to the relavant colors

      TRACE_FORMAT("ncolors %u, ideal %u", ncolors, ideal);
      memcpy (colors, _colors, ncolors * sizeof(int));
      TRACE_FN_EXIT();
    }

    extern inline void rectangle_dput_1color_broadcast_metadata(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:RectangleDput1Color:SHMEM:MU");
      m->check_perf.values.hw_accel     = 1;
    }

    extern inline void mu_rectangle_dput_1color_broadcast_metadata(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:RectangleDput1Color:MU:MU");
      m->check_perf.values.hw_accel     = 1;
    }

    typedef CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT
    < 1,
    CCMI::Schedule::TorusRect,
    CCMI::ConnectionManager::ColorConnMgr,
    get_rect_colors,
    PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX >
    RectangleDput1ColorBroadcastComposite;

    // The only difference between RectangleDput1ColorBroadcastFactory and MURectangleDput1ColorBroadcastFactory
    // is the metadata. Clumsy way to differentiate shmem+mu from mu-only.
    // The real differentiator is which NI gets used, but metadata doesn't know that.
    typedef CCMI::Adaptor::CachedAllSidedFactoryT
    < RectangleDput1ColorBroadcastComposite,
    rectangle_dput_1color_broadcast_metadata,
    CCMI::ConnectionManager::ColorConnMgr,
    PAMI::Geometry::CKEY_BCASTCOMPOSITE2 >
    RectangleDput1ColorBroadcastFactory;

    typedef CCMI::Adaptor::CachedAllSidedFactoryT
    < RectangleDput1ColorBroadcastComposite,
    mu_rectangle_dput_1color_broadcast_metadata,
    CCMI::ConnectionManager::ColorConnMgr,
    PAMI::Geometry::CKEY_BCASTCOMPOSITE3 >
    MURectangleDput1ColorBroadcastFactory;

    extern inline void rectangle_dput_broadcast_metadata(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:RectangleDput:SHMEM:MU");
      m->check_perf.values.hw_accel     = 1;
    }

    extern inline void mu_rectangle_dput_broadcast_metadata(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:RectangleDput:MU:MU");
      m->check_perf.values.hw_accel     = 1;
    }

    typedef CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT
    < 10,
    CCMI::Schedule::TorusRect,
    CCMI::ConnectionManager::ColorConnMgr,
    get_rect_colors,
    PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX >
    RectangleDputBroadcastComposite;

    // The only difference between RectangleDputBroadcastFactory and MURectangleDputBroadcastFactory 
    // is the metadata. Clumsy way to differentiate shmem+mu from mu-only.
    // The real differentiator is which NI gets used, but metadata doesn't know that.
    typedef CCMI::Adaptor::CachedAllSidedFactoryT
    < RectangleDputBroadcastComposite,
    rectangle_dput_broadcast_metadata,
    CCMI::ConnectionManager::ColorConnMgr,
    PAMI::Geometry::CKEY_BCASTCOMPOSITE0 >
    RectangleDputBroadcastFactory;

    typedef CCMI::Adaptor::CachedAllSidedFactoryT
    < RectangleDputBroadcastComposite,
    mu_rectangle_dput_broadcast_metadata,
    CCMI::ConnectionManager::ColorConnMgr,
    PAMI::Geometry::CKEY_BCASTCOMPOSITE1 >
    MURectangleDputBroadcastFactory;

    extern inline void rectangle_dput_allgather_metadata(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:RectangleDput:SHMEM:MU");
      m->check_perf.values.hw_accel     = 1;
    }

    extern inline void get_rect_allgv_colors (PAMI::Topology             * t,
                                              unsigned                    bytes,
                                              unsigned                  * colors,
                                              unsigned                  & ncolors)
    {
      unsigned max = 0, ideal = 0;
      unsigned _colors[10];
      CCMI::Schedule::TorusRect::getColors (t, ideal, max, _colors);

      //if (bytes <= 8192) //16 packets
      //ideal = 1;

      if (bytes <= 8192) //16 packets
        ideal = 1;
      else if (bytes <= 65536 && ideal >= 2)
        ideal = 2;
      else if (bytes <= 262144 && ideal >= 3)
        ideal = 3;
      else if (bytes <= 1048576 && ideal >= 5)
        ideal = 5;
      else if (bytes <= 4194304 && ideal >= 7)
        ideal = 7;

      if (ncolors > ideal)
        ncolors = ideal;

      memcpy (colors, _colors, ncolors * sizeof(int));
    }

    typedef CCMI::Adaptor::Allgather::AllgatherOnBroadcastT < 1, 10,
    CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT
    < 10,
    CCMI::Schedule::TorusRect,
    CCMI::ConnectionManager::ColorConnMgr,
    get_rect_allgv_colors,
    PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX > ,
    CCMI::ConnectionManager::ColorConnMgr,
    PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX > RectangleDputAllgather;

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT
    < RectangleDputAllgather,
    rectangle_dput_allgather_metadata,
    CCMI::ConnectionManager::ColorConnMgr >
    RectangleDputAllgatherFactory;

    //----------------------------------------------------------------------------
    /// \brief The BGQ Multi* registration class for Shmem and MU.
    //----------------------------------------------------------------------------
    template <class T_Geometry, class T_ShmemDevice, class T_ShmemNativeInterface, class T_MUDevice, class T_MUNativeInterface, class T_AxialDputNativeInterface, class T_AxialShmemDputNativeInterface>
    class BGQMultiRegistration :
    public CollRegistration<PAMI::CollRegistration::BGQMultiRegistration<T_Geometry, T_ShmemDevice, T_ShmemNativeInterface, T_MUDevice, T_MUNativeInterface, T_AxialDputNativeInterface, T_AxialShmemDputNativeInterface>, T_Geometry>
    {

    public:
      inline BGQMultiRegistration(T_ShmemNativeInterface              *shmem_ni,
                                  T_ShmemDevice                       &shmem_device,
                                  T_MUDevice                          &mu_device,
                                  pami_client_t                        client,
                                  pami_context_t                       context,
                                  size_t                               context_id,
                                  size_t                               client_id,
                                  int                                 *dispatch_id,
                                  std::map<unsigned, pami_geometry_t> *geometry_map):
      CollRegistration<PAMI::CollRegistration::BGQMultiRegistration<T_Geometry, T_ShmemDevice, T_ShmemNativeInterface, T_MUDevice, T_MUNativeInterface, T_AxialDputNativeInterface, T_AxialShmemDputNativeInterface>, T_Geometry> (),
      _client(client),
      _context(context),
      _context_id(context_id),
      _dispatch_id(dispatch_id),
      _geometry_map(geometry_map),
      _sconnmgr(65535),
      _csconnmgr(),
      _cg_connmgr(65535),
      _color_connmgr(),
      _shmem_barrier_composite(NULL),
      _gi_barrier_composite(NULL),
      _mu_rectangle_barrier_composite(NULL),
      _msync_composite(NULL),
      _msync2d_composite(NULL),
      _msync2d_gishm_composite(NULL),
      _msync2d_rectangle_composite(NULL), 
      _shmem_device(shmem_device),
      _shmem_ni(shmem_ni),
      _shmem_msync_factory(&_sconnmgr, _shmem_ni),
      _shmem_mcast_factory(&_sconnmgr, _shmem_ni),
      _shmem_mcomb_factory(&_sconnmgr, _shmem_ni),
      _mu_device(mu_device),
      _mu_ni_msync(NULL),
      _mu_ni_mcomb(NULL),
      _mu_ni_mcast(NULL),
      _mu_ni_mcast3(NULL),
      _mu_ni_msync2d(NULL),
      _mu_ni_mcast2d(NULL),
      _mu_ni_mcomb2d(NULL),
      _mu_ni_mcomb2dNP(NULL),
      _axial_mu_dput_ni(NULL),
      //_axial_dput_mu_1_ni(NULL),
      _gi_msync_factory(NULL),
      _mu_mcast_factory(NULL),
#ifdef PAMI_ENABLE_X0_PROTOCOLS
      _mu_mcast3_factory(NULL),
#endif
      _mu_mcomb_factory(NULL),
      _msync2d_composite_factory(NULL),
      _msync2d_gishm_composite_factory(NULL),
#ifdef PAMI_ENABLE_X0_PROTOCOLS
      _mcast2d_composite_factory(NULL),
      _mcomb2d_composite_factory(NULL),
      _mcomb2dNP_composite_factory(NULL),
#endif
      _mcomb2d_dput_composite_factory(NULL),
#ifdef PAMI_ENABLE_X0_PROTOCOLS
      _mcomb2dNP_dput_composite_factory(NULL),
#endif
      _mu_rectangle_1color_dput_broadcast_factory(NULL),
      _mu_rectangle_dput_broadcast_factory(NULL),
      _mu_rectangle_dput_allgather_factory(NULL),
      _shmem_mu_rectangle_1color_dput_broadcast_factory(NULL),
      _shmem_mu_rectangle_dput_broadcast_factory(NULL),
      _shmem_mu_rectangle_dput_allgather_factory(NULL)
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT("<%p>", this);
        //DO_DEBUG((templateName<T_Geometry>()));
        //DO_DEBUG((templateName<T_ShmemNativeInterface>()));
        //DO_DEBUG((templateName<T_MUDevice>()));
        //DO_DEBUG((templateName<T_MUNativeInterface>()));
        //DO_DEBUG((templateName<T_AxialDputNativeInterface>()));

        //set the mapid functions
        if (__global.useshmem())// && (__global.topology_local.size() > 1))
        {
          TRACE_FORMAT("<%p> useshmem", this);
          _shmem_msync_factory.setMapIdToGeometry(mapidtogeometry);
        }

        if (__global.useMU())
        {
          TRACE_FORMAT("<%p> usemu", this);

          _mu_ni_msync          = new (_mu_ni_msync_storage         ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcomb          = new (_mu_ni_mcomb_storage         ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcast          = new (_mu_ni_mcast_storage         ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcast3         = new (_mu_ni_mcast3_storage        ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_msync2d        = new (_mu_ni_msync2d_storage       ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcast2d        = new (_mu_ni_mcast2d_storage       ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcomb2d        = new (_mu_ni_mcomb2d_storage       ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcomb2dNP      = new (_mu_ni_mcomb2dNP_storage     ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);

          _axial_mu_dput_ni     = new (_axial_mu_dput_ni_storage    ) T_AxialDputNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);

          if (_axial_mu_dput_ni->status() != PAMI_SUCCESS) _axial_mu_dput_ni = NULL; // Not enough resources?

          if (__global.mapping.t() == 0)
            //We can now construct this on any process (as long as
            //process 0 on each node in the job also calls it
            _mu_global_dput_ni    = new (_mu_global_dput_ni_storage) MUGlobalDputNI (_mu_device, client, context, context_id, client_id, _dispatch_id);

          _mu_shmem_global_dput_ni    = new (_mu_shmem_global_dput_ni_storage) MUShmemGlobalDputNI (_mu_device, client, context, context_id, client_id, _dispatch_id);

          _mu_ammulticast_ni    = new (_mu_ammulticast_ni_storage) MUAMMulticastNI (_mu_device, client, context, context_id, client_id, _dispatch_id);

          _mu_m2m_single_ni      = new (_mu_m2m_single_ni_storage) M2MNISingle (_mu_device, client, context, context_id, client_id, _dispatch_id);

          _mu_m2m_vector_long_ni = new (_mu_m2m_vector_long_ni_storage) M2MNIVectorLong (_mu_device, client, context, context_id, client_id, _dispatch_id);

          _mu_m2m_vector_int_ni = new (_mu_m2m_vector_int_ni_storage) M2MNIVectorInt (_mu_device, client, context, context_id, client_id, _dispatch_id);


          if (_mu_ammulticast_ni->status() != PAMI_SUCCESS) _mu_ammulticast_ni = NULL;

          if (_mu_m2m_single_ni->status() != PAMI_SUCCESS)      _mu_m2m_single_ni = NULL;
          if (_mu_m2m_vector_long_ni->status() != PAMI_SUCCESS) _mu_m2m_vector_long_ni = NULL;
          if (_mu_m2m_vector_int_ni->status() != PAMI_SUCCESS) _mu_m2m_vector_int_ni = NULL;

          if (__global.useshmem())
          {
            _axial_shmem_mu_dput_ni     = new (_axial_shmem_mu_dput_ni_storage    ) T_AxialShmemDputNativeInterface(_mu_device, _shmem_device, client, context, context_id, client_id, _dispatch_id);

            if (_axial_shmem_mu_dput_ni->status() != PAMI_SUCCESS) _axial_shmem_mu_dput_ni = NULL; // Not enough resources?
          }

          _gi_msync_factory = NULL;
          if (_mu_global_dput_ni)
            _gi_msync_factory     = new (_gi_msync_factory_storage    ) GIMultiSyncFactory(&_sconnmgr, _mu_global_dput_ni);
          _mu_mcomb_factory     = new (_mu_mcomb_factory_storage    ) MUMultiCombineFactory(&_sconnmgr, _mu_ni_mcomb);

          _mu_rectangle_msync_factory = NULL;
          if (_axial_mu_dput_ni)
            _mu_rectangle_msync_factory = new (_mu_rectangle_msync_factory_storage) MURectangleMultiSyncFactory(&_sconnmgr, _axial_mu_dput_ni);

          _binomial_barrier_factory = NULL;
          if (_mu_ammulticast_ni)
          {
            _binomial_barrier_factory = new (_binomial_barrier_factory_storage)  OptBinomialBarrierFactory (&_sconnmgr, _mu_ammulticast_ni, OptBinomialBarrierFactory::cb_head);
            _binomial_barrier_factory->setMapIdToGeometry(mapidtogeometry);
          }

          _alltoall_factory = NULL;
          if (_mu_m2m_single_ni)
          {
            _alltoall_factory = new (_alltoall_factory_storage) All2AllFactory(&_csconnmgr, _mu_m2m_single_ni);
            _alltoall_factory->setMapIdToGeometry(mapidtogeometry);
          }

          _alltoallv_factory = NULL;
          if (_mu_m2m_vector_long_ni)
          {
            _alltoallv_factory = new (_alltoallv_factory_storage) All2AllvFactory(&_csconnmgr, _mu_m2m_vector_long_ni);
            _alltoallv_factory->setMapIdToGeometry(mapidtogeometry);
          }

          _alltoallv_int_factory = NULL;
          if (_mu_m2m_vector_int_ni)
          {
            _alltoallv_int_factory = new (_alltoallv_int_factory_storage) All2AllvFactory_int(&_csconnmgr, _mu_m2m_vector_int_ni);
            _alltoallv_int_factory->setMapIdToGeometry(mapidtogeometry);
          }

          _mucollectivedputmulticastfactory    = new (_mucollectivedputmulticaststorage ) MUCollectiveDputMulticastFactory(&_sconnmgr, _mu_global_dput_ni);
          _mucollectivedputmulticombinefactory    = new (_mucollectivedputmulticombinestorage ) MUCollectiveDputMulticombineFactory(&_sconnmgr, _mu_global_dput_ni);        

          _mushmemcollectivedputmulticombinefactory    = new (_mushmemcollectivedputmulticombinestorage ) MUShmemCollectiveDputMulticombineFactory(&_sconnmgr, _mu_shmem_global_dput_ni);       

          if (_axial_shmem_mu_dput_ni)
          {
            TRACE_FORMAT("<%p>  RectangleDput1ColorBroadcastFactory", this);
            _shmem_mu_rectangle_1color_dput_broadcast_factory = new (_shmem_mu_rectangle_1color_dput_broadcast_factory_storage) RectangleDput1ColorBroadcastFactory(&_color_connmgr, _axial_shmem_mu_dput_ni);

            TRACE_FORMAT("<%p>  RectangleDputBroadcastFactory", this);
            _shmem_mu_rectangle_dput_broadcast_factory = new (_shmem_mu_rectangle_dput_broadcast_factory_storage) RectangleDputBroadcastFactory(&_color_connmgr, _axial_shmem_mu_dput_ni);

            _shmem_mu_rectangle_dput_allgather_factory = new (_shmem_mu_rectangle_dput_allgather_factory_storage) RectangleDputAllgatherFactory(&_color_connmgr, _axial_shmem_mu_dput_ni);
          }

          if (_axial_mu_dput_ni)
          {
            TRACE_FORMAT("<%p> MURectangleDput1ColorBroadcastFactory", this);
            _mu_rectangle_1color_dput_broadcast_factory = new (_mu_rectangle_1color_dput_broadcast_factory_storage) MURectangleDput1ColorBroadcastFactory(&_color_connmgr, _axial_mu_dput_ni);

            TRACE_FORMAT("<%p>  MURectangleDputBroadcastFactory", this);
            _mu_rectangle_dput_broadcast_factory = new (_mu_rectangle_dput_broadcast_factory_storage) MURectangleDputBroadcastFactory(&_color_connmgr, _axial_mu_dput_ni);

            _mu_rectangle_dput_allgather_factory = new (_mu_rectangle_dput_allgather_factory_storage) RectangleDputAllgatherFactory(&_color_connmgr, _axial_mu_dput_ni);
          }

          if (_gi_msync_factory)
            _gi_msync_factory->setMapIdToGeometry(mapidtogeometry);

          // Can't be ctor'd unless the NI was created
          _mu_mcast_factory  = new (_mu_mcast_factory_storage ) MUMultiCastFactory(&_sconnmgr, _mu_ni_mcast);
#ifdef PAMI_ENABLE_X0_PROTOCOLS
          _mu_mcast3_factory = new (_mu_mcast3_factory_storage) MUMultiCast3Factory(&_sconnmgr, _mu_ni_mcast3);
#endif
        }

//          if ((__global.useMU()) && (__global.useshmem()))
        {
          _ni_array[0] = _shmem_ni;
          _ni_array[1] = _mu_ni_msync2d;
          _ni_array[2] = _shmem_ni;
          _ni_array[3] = _mu_ni_mcomb2dNP;
          _msync2d_composite_factory = new (_msync2d_composite_factory_storage) MultiSync2DeviceFactory(&_sconnmgr, &_ni_array[0]);
          _msync2d_composite_factory->setMapIdToGeometry(mapidtogeometry);

          _ni_array[4] = _shmem_ni;
          _ni_array[5] = _mu_global_dput_ni;
          _msync2d_gishm_composite_factory = new (_msync2d_gishm_composite_factory_storage) MultiSync2DeviceGIShmemFactory(&_sconnmgr, &_ni_array[4]);
          _msync2d_gishm_composite_factory->setMapIdToGeometry(mapidtogeometry);

          _ni_array[6] = _shmem_ni;
          _ni_array[7] = _axial_mu_dput_ni;
          _msync2d_rectangle_composite_factory = new (_msync2d_rectangle_composite_factory_storage) MultiSync2DeviceRectangleFactory(&_sconnmgr, &_ni_array[6]);
          _msync2d_rectangle_composite_factory->setMapIdToGeometry(mapidtogeometry);

          _mcomb2d_dput_composite_factory = new (_mcomb2d_dput_composite_factory_storage) MultiCombine2DeviceDputFactory(&_sconnmgr, _shmem_ni, _mu_global_dput_ni);
#ifdef PAMI_ENABLE_X0_PROTOCOLS
          _mcomb2dNP_dput_composite_factory = new (_mcomb2dNP_dput_composite_factory_storage) MultiCombine2DeviceDputFactoryNP(&_sconnmgr,  &_ni_array[4]);
#endif

#ifdef PAMI_ENABLE_X0_PROTOCOLS
          _mcast2d_composite_factory = new (_mcast2d_composite_factory_storage) MultiCast2DeviceFactory(&_sconnmgr, _shmem_ni, false, _mu_ni_mcast2d,  _mu_ni_mcast2d ? true : false);
#endif
          _mcast2d_dput_composite_factory = new (_mcast2d_dput_composite_factory_storage) MultiCast2DeviceDputFactory(&_sconnmgr, _shmem_ni, false, _mu_global_dput_ni,  false);

#ifdef PAMI_ENABLE_X0_PROTOCOLS
          _mcomb2d_composite_factory = new (_mcomb2d_composite_factory_storage) MultiCombine2DeviceFactory(&_sconnmgr, _shmem_ni, _mu_ni_mcomb2d);
#endif
#ifdef PAMI_ENABLE_X0_PROTOCOLS
          _mcomb2dNP_composite_factory = new (_mcomb2dNP_composite_factory_storage) MultiCombine2DeviceFactoryNP(&_sconnmgr,  &_ni_array[2]);
#endif
        }

        TRACE_FN_EXIT();
      }

      inline pami_result_t register_local_impl (size_t context_id, T_Geometry *geometry, uint64_t *in, int &n) 
      {
        if (_axial_mu_dput_ni == NULL)
          return PAMI_SUCCESS;

        if ((_mu_rectangle_msync_factory && __global.topology_local.size() == 1) || 
            (_msync2d_rectangle_composite_factory && __global.topology_local.size() > 1 &&
             __global.useMU() && __global.useshmem()))
        {
          *in = _axial_mu_dput_ni->getMsyncModel().getAllocationVector();
          n  = 1;
        }
        return PAMI_SUCCESS;
      }    

      inline pami_result_t receive_global_impl (size_t context_id, T_Geometry *geometry, uint64_t *in, int n) 
      {
        if (_axial_mu_dput_ni == NULL)
          return PAMI_SUCCESS;

        PAMI_assert (n == 1);

        if ((((PAMI::Topology *)geometry->getTopology(PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX))->type() == PAMI_COORD_TOPOLOGY)
            &&
            ((_mu_rectangle_msync_factory && __global.topology_local.size() == 1) ||
             (_msync2d_rectangle_composite_factory && __global.topology_local.size() > 1 && 
              __global.useMU() && __global.useshmem())
            )
           )
        {
          uint64_t result = *in;
          for (size_t i = 0; i < 64; ++i)
            if ((result & (0x1 << i)) != 0)
            {
              //fprintf (stderr, "Calling configure with class route %ld, in 0x%lx", i, result);
              _axial_mu_dput_ni->getMsyncModel().configureClassRoute(i, (PAMI::Topology *)geometry->getTopology(PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX));
              geometry->setKey (PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID1, (void*)(i+1));
              break;
            }
        }

        return PAMI_SUCCESS;
      }

      inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT("<%p>phase %d, context_id %zu, geometry %p, msync %p, mcast %p, mcomb %p", this, phase, context_id, geometry, &_shmem_msync_factory, &_shmem_mcast_factory, &_shmem_mcomb_factory);
        if (geometry->size() == 1) // Disable BGQ protocols on 1 task geometries.
        {
          TRACE_FN_EXIT();
          return PAMI_SUCCESS;
        }
        pami_xfer_t xfer = {0};
        // BE CAREFUL! It's not ok to make registration decisions based on local topology unless you know that all nodes will make the same decision.
        // We use local_sub_topology on single node registrations 
        // and on MU/Classroute registrations (because they are rectangular so all nodes have the same local subtopology).
        PAMI::Topology * topology = (PAMI::Topology*) geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX);
        PAMI::Topology * local_sub_topology = (PAMI::Topology*) geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX);
        PAMI::Topology * master_sub_topology = (PAMI::Topology*) geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
        TRACE_FORMAT("<%p>topology: size() %zu, isLocal() %u/%zu, isGlobal #u/%zu", this, topology->size(),  topology->isLocalToMe(), local_sub_topology->size(), master_sub_topology->size());//,  topology->isGlobal()));

        //DO_DEBUG(for (unsigned i = 0; i < topology->size(); ++i) fprintf(stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() topology[%u] = %u", this, i, topology->index2Rank(i)););

        if (phase == 0)
        {

          if (_binomial_barrier_factory)
          {
            //Set optimized barrier to binomial. May override optimized barrier later
            pami_xfer_t xfer = {0};
            OptBinomialBarrier *opt_binomial = (OptBinomialBarrier *)
                                               _binomial_barrier_factory->generate(geometry, &xfer);
            PAMI_assert(geometry->getKey(context_id, PAMI::Geometry::CKEY_BARRIERCOMPOSITE2)==opt_binomial);
            opt_binomial->getExecutor()->setContext(_context);
            geometry->setKey(context_id, PAMI::Geometry::CKEY_OPTIMIZEDBARRIERCOMPOSITE,
                             (void*)opt_binomial);
          }


          if ((__global.useshmem())  && (__global.topology_local.size() > 1)
#ifndef PAMI_ENABLE_SHMEM_SUBNODE
              && (__global.topology_local.size() == local_sub_topology->size()) /// \todo might ease this restriction later - when shmem supports it
#endif
             )
          {
            TRACE_FORMAT("<%p>Register Shmem local barrier", this);

            // If the geometry is all local nodes, we can use pure shmem composites.
            if (topology->isLocalToMe())
            {
              TRACE_FORMAT("<%p>Register Local Shmem factories", this);
              _shmem_barrier_composite = _shmem_msync_factory.generate(geometry, &xfer);
              PAMI_assert(geometry->getKey(context_id, PAMI::Geometry::CKEY_BARRIERCOMPOSITE7)==_shmem_barrier_composite);
              // Add Barriers
              geometry->addCollective(PAMI_XFER_BARRIER, &_shmem_msync_factory, _context_id);

              // Add Broadcasts
              geometry->addCollective(PAMI_XFER_BROADCAST, &_shmem_mcast_factory, _context_id);

              // Add Allreduces
#ifdef PAMI_ENABLE_NEW_SHMEM   // limited support - 4/8/16 processes only
              if ((__global.topology_local.size() ==  4) ||  
                  (__global.topology_local.size() ==  8) ||
                  (__global.topology_local.size() == 16))
                geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE, &_shmem_mcomb_factory, _context_id);
#else
              geometry->addCollective(PAMI_XFER_ALLREDUCE, &_shmem_mcomb_factory, _context_id);
#endif
            }
          }

          // (Maybe) Add rectangle broadcasts
          TRACE_FORMAT("<%p>Analyze Rectangle factories %p/%p, %p/%p, isLocal? %u", this,
                      _mu_rectangle_1color_dput_broadcast_factory, _mu_rectangle_dput_broadcast_factory,
                      _shmem_mu_rectangle_1color_dput_broadcast_factory, _shmem_mu_rectangle_dput_broadcast_factory,
                      topology->isLocal());

          // Is there a coordinate topology? Try rectangle protocols
          PAMI::Topology * rectangle = (PAMI::Topology*)geometry->getTopology(PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX);

          if (rectangle->type() == PAMI_COORD_TOPOLOGY) // could be EMPTY if not valid on this geometry
          {
            TRACE_FORMAT("<%p>Register Rectangle", this);
            if (_mu_rectangle_msync_factory && __global.topology_local.size() == 1 &&
                geometry->getKey(PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID1))
            {
              //Set optimized barrier to rectangle. May override optimized barrier later
              pami_xfer_t xfer = {0};
              CCMI::Executor::Composite *opt_composite =  _mu_rectangle_msync_factory->generate(geometry, &xfer); 
              PAMI_assert(geometry->getKey(context_id, PAMI::Geometry::CKEY_BARRIERCOMPOSITE5)==opt_composite);
              geometry->setKey(context_id, PAMI::Geometry::CKEY_OPTIMIZEDBARRIERCOMPOSITE,
                               (void*)opt_composite);
            }
            else if (_msync2d_rectangle_composite_factory && __global.topology_local.size() > 1 && 
                     __global.useMU() && __global.useshmem() &&               
                     geometry->getKey(PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID1))
            {
              //Set optimized barrier to rectangle. May override optimized barrier later
              pami_xfer_t xfer = {0};
              CCMI::Executor::Composite *opt_composite;
              opt_composite = _msync2d_rectangle_composite_factory->generate(geometry, &xfer); 
              PAMI_assert(geometry->getKey(context_id, PAMI::Geometry::CKEY_BARRIERCOMPOSITE6)==opt_composite);
//            geometry->setKey(context_id, PAMI::Geometry::CKEY_OPTIMIZEDBARRIERCOMPOSITE,
//                             (void*)opt_composite);
            }

            // Add rectangle protocols:
            if ((_shmem_mu_rectangle_1color_dput_broadcast_factory)
#ifndef PAMI_ENABLE_SHMEM_SUBNODE
                && (__global.topology_local.size() == local_sub_topology->size()) /// \todo might ease this restriction later - when shmem supports it
#endif
               )
              geometry->addCollective(PAMI_XFER_BROADCAST,  _shmem_mu_rectangle_1color_dput_broadcast_factory, _context_id);
            if (_mu_rectangle_1color_dput_broadcast_factory)
              geometry->addCollective(PAMI_XFER_BROADCAST,  _mu_rectangle_1color_dput_broadcast_factory, _context_id);

            if ((_shmem_mu_rectangle_dput_broadcast_factory) 
#ifndef PAMI_ENABLE_SHMEM_SUBNODE
                && (__global.topology_local.size() == local_sub_topology->size()) /// \todo might ease this restriction later - when shmem supports it
#endif
               )
              geometry->addCollective(PAMI_XFER_BROADCAST,  _shmem_mu_rectangle_dput_broadcast_factory, _context_id);
            if (_mu_rectangle_dput_broadcast_factory)
              geometry->addCollective(PAMI_XFER_BROADCAST,  _mu_rectangle_dput_broadcast_factory, _context_id);

            if (_mucollectivedputmulticastfactory && __global.topology_local.size() == 1)
              geometry->addCollective(PAMI_XFER_BROADCAST,  _mucollectivedputmulticastfactory, _context_id);

            if (_mucollectivedputmulticombinefactory && __global.topology_local.size() == 1)
              geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE,  _mucollectivedputmulticombinefactory, _context_id);

#if 1  // allgatherv hangs 

            if (((master_sub_topology->size() == 1) || (local_sub_topology->size() < 32)) && (_shmem_mu_rectangle_dput_allgather_factory))
              geometry->addCollective(PAMI_XFER_ALLGATHERV,  _shmem_mu_rectangle_dput_allgather_factory, _context_id);
            else if (_mu_rectangle_dput_allgather_factory)
              geometry->addCollective(PAMI_XFER_ALLGATHERV,  _mu_rectangle_dput_allgather_factory, _context_id);

#endif
          }

        }
        else if (phase == 1)
        {
          /// A simple check (of sizes) to see if this subgeometry is all global,
          /// then the geometry topology is usable by 'pure' MU protocols
          bool usePureMu = topology->size() == master_sub_topology->size() ? true : false;
          usePureMu = usePureMu && !topology->isLocalToMe();
          TRACE_FORMAT("<%p>usePureMu = %u (size %zu/%zu)", this, usePureMu, topology->size(), master_sub_topology->size());

          // Add optimized binomial barrier
          if (_binomial_barrier_factory)
            geometry->addCollective(PAMI_XFER_BARRIER, _binomial_barrier_factory, _context_id);

          if (_alltoall_factory)
            geometry->addCollective(PAMI_XFER_ALLTOALL, _alltoall_factory, _context_id);

          if (_alltoallv_factory)
            geometry->addCollective(PAMI_XFER_ALLTOALLV, _alltoallv_factory, _context_id);

          if (_alltoallv_int_factory)
            geometry->addCollective(PAMI_XFER_ALLTOALLV_INT, _alltoallv_int_factory, _context_id);

          // Check for class routes before enabling MU collective network protocols
          void *val;
          val = geometry->getKey(PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID);
          TRACE_FORMAT("<%p>GKEY_MSYNC_CLASSROUTEID %p", this, val);

          if (val && val != PAMI_CR_GKEY_FAIL)// We have a class route
          {
            // If we can use pure MU composites, add them
            if (usePureMu)
            {
              // Direct MU/GI only on one context per node (lowest T, context 0) lowest T is guaranteed by classroute code
              if ((_context_id == 0) // (__global.mapping.isLowestT())
                  && (_gi_msync_factory))
              {
                TRACE_FORMAT("<%p>Register MU barrier", this);
                geometry->setKey(context_id, PAMI::Geometry::CKEY_BARRIERCOMPOSITE3, NULL);
                _gi_barrier_composite = _gi_msync_factory->generate(geometry, &xfer);
                PAMI_assert(geometry->getKey(context_id, PAMI::Geometry::CKEY_BARRIERCOMPOSITE3)==_gi_barrier_composite);

                // Add Barriers
                geometry->addCollective(PAMI_XFER_BARRIER, _gi_msync_factory, _context_id);
              }

              if (_mu_rectangle_msync_factory)
              {
                _mu_rectangle_barrier_composite = _mu_rectangle_msync_factory->generate(geometry, &xfer);
                // Add Barriers
                geometry->addCollective(PAMI_XFER_BARRIER, _mu_rectangle_msync_factory, _context_id); 
              }
            }
          }
          if ((val && val != PAMI_CR_GKEY_FAIL) || // We have a class route or
              (topology->isLocalToMe()))           // It's all local - we might use 2 device protocol in shmem-only mode
          {
            // Add 2 device composite protocols
#ifndef PAMI_ENABLE_SHMEM_SUBNODE
            if (__global.topology_local.size() == local_sub_topology->size()) /// \todo might ease this restriction later - when shmem supports it
#endif
            {
              if (_msync2d_composite_factory)
              {
                _msync2d_composite = _msync2d_composite_factory->generate(geometry, &xfer);
                geometry->addCollective(PAMI_XFER_BARRIER, _msync2d_composite_factory, _context_id);
              }

              if (_msync2d_gishm_composite_factory)
              {
                geometry->setKey(context_id, PAMI::Geometry::CKEY_BARRIERCOMPOSITE4, NULL);
                _msync2d_gishm_composite = _msync2d_gishm_composite_factory->generate(geometry, &xfer);
                geometry->addCollective(PAMI_XFER_BARRIER, _msync2d_gishm_composite_factory, _context_id);
                PAMI_assert(geometry->getKey(context_id, PAMI::Geometry::CKEY_BARRIERCOMPOSITE4)==_msync2d_gishm_composite);
              }

              Topology master = *master_sub_topology;
              master.convertTopology(PAMI_COORD_TOPOLOGY);
              if ((master.type() == PAMI_COORD_TOPOLOGY) && 
                  (_msync2d_rectangle_composite_factory) && 
                  (geometry->getKey(PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID1)) // \todo PAMI_CR_GKEY_FAIL?
                 )
              {
                _msync2d_rectangle_composite = _msync2d_rectangle_composite_factory->generate(geometry, &xfer);
                geometry->addCollective(PAMI_XFER_BARRIER, _msync2d_rectangle_composite_factory, _context_id);
              }
            }
          }

          val = geometry->getKey(PAMI::Geometry::GKEY_MCAST_CLASSROUTEID);
          TRACE_FORMAT("<%p>GKEY_MCAST_CLASSROUTEID %p", this, val);

          if ((val && val != PAMI_CR_GKEY_FAIL) || // We have a class route or
              (topology->isLocalToMe()))           // It's all local - we might use 2 device protocol in shmem-only mode
          {
            // If we can use pure MU composites, add them
            if (usePureMu)
            {
              // Direct MU only on one context per node (lowest T, context 0) lowest T is guaranteed by classroute code
              if (_context_id == 0) // (__global.mapping.isLowestT())
              {
                TRACE_FORMAT("<%p>Register MU bcast", this);
                // Add Broadcasts
                geometry->addCollective(PAMI_XFER_BROADCAST,  _mu_mcast_factory,  _context_id);
#ifdef PAMI_ENABLE_X0_PROTOCOLS
                geometry->addCollectiveCheck(PAMI_XFER_BROADCAST,  _mu_mcast3_factory, _context_id);
#endif
              }
            }
            // Add 2 device composite protocols
#ifndef PAMI_ENABLE_NEW_SHMEM
            // Default Shmem doesn't work with 2 device protocol right now
            if (local_sub_topology->size() == 1)
#endif
#ifndef PAMI_ENABLE_SHMEM_SUBNODE
              if (__global.topology_local.size() == local_sub_topology->size()) /// \todo might ease this restriction later - when shmem supports it
#endif
              {
#ifdef PAMI_ENABLE_X0_PROTOCOLS
                if (_mcast2d_composite_factory)
                {
                  TRACE_FORMAT("<%p>Register mcast 2D", this);
                  geometry->addCollectiveCheck(PAMI_XFER_BROADCAST, _mcast2d_composite_factory, _context_id);
                }
#endif
                if (_mcast2d_dput_composite_factory)
                {
                  TRACE_FORMAT("<%p>Register mcast dput 2D", this);
                  geometry->addCollective(PAMI_XFER_BROADCAST, _mcast2d_dput_composite_factory, _context_id);
                }
              }
          }

          val = geometry->getKey(PAMI::Geometry::GKEY_MCOMB_CLASSROUTEID);
          TRACE_FORMAT("<%p>GKEY_MCOMB_CLASSROUTEID %p", this, val);

          if ((val && val != PAMI_CR_GKEY_FAIL) || // We have a class route or
              (topology->isLocalToMe()))           // It's all local - we might use 2 device protocol in shmem-only mode
          {
            // If we can use pure MU composites, add them
            if (usePureMu)
            {
              // Direct MU allreduce only on one context per node (lowest T, context 0) lowest T is guaranteed by classroute code
              if (_context_id == 0) // (__global.mapping.isLowestT())
              {
                // Add Allreduces
                TRACE_FORMAT("<%p>Register MU allreduce", this);
                geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE, _mu_mcomb_factory, _context_id);
              }
            }

            // Add 2 device composite protocols
#ifdef PAMI_ENABLE_NEW_SHMEM   // limited support - 4/8/16 processes only
            if ((__global.topology_local.size() ==  4) ||  
                (__global.topology_local.size() ==  8) ||
                (__global.topology_local.size() == 16))
#endif
#ifndef PAMI_ENABLE_SHMEM_SUBNODE
              if (__global.topology_local.size() == local_sub_topology->size()) /// \todo might ease this restriction later - when shmem supports it
#endif
              {
                // New optimized MU+Shmem protocol requires a class route
                if ((_mushmemcollectivedputmulticombinefactory) && (val && val != PAMI_CR_GKEY_FAIL))
                  geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE,  _mushmemcollectivedputmulticombinefactory, _context_id);

#ifdef PAMI_ENABLE_X0_PROTOCOLS
                // NP (non-pipelining) 2 device protocols
                if ((_mcomb2dNP_dput_composite_factory) && (master_sub_topology->size() > 1))  // \todo Simple NP protocol doesn't like 1 master - fix it later
                  geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2dNP_dput_composite_factory, _context_id);
#endif
#ifdef PAMI_ENABLE_X0_PROTOCOLS
                if ((_mcomb2dNP_composite_factory) && (master_sub_topology->size() > 1))  // \todo Simple NP protocol doesn't like 1 master - fix it later
                  geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2dNP_composite_factory, _context_id);
#endif
                //  2 device protocols
                if (_mcomb2d_dput_composite_factory)
                  geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2d_dput_composite_factory, _context_id);

#ifdef PAMI_ENABLE_X0_PROTOCOLS
                if (_mcomb2d_composite_factory)
                  geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2d_composite_factory, _context_id);
#endif
              }
          }
        }
        else if (phase == -1)
        {
          /// \todo remove MU collectives algorithms... TBD... only remove phase 1 algorithms??
          geometry->rmCollective(PAMI_XFER_ALLREDUCE,      _mu_mcomb_factory,                         _context_id);

          geometry->rmCollective(PAMI_XFER_ALLTOALLV_INT,  _alltoallv_int_factory,                    _context_id);
          geometry->rmCollective(PAMI_XFER_ALLTOALLV,      _alltoallv_factory,                        _context_id);
          geometry->rmCollective(PAMI_XFER_ALLTOALL,       _alltoall_factory,                         _context_id);

          geometry->rmCollective(PAMI_XFER_BROADCAST,      _mu_mcast_factory,                         _context_id);
          geometry->rmCollective(PAMI_XFER_BROADCAST,      _mcast2d_dput_composite_factory,           _context_id);

          geometry->rmCollective(PAMI_XFER_BARRIER,        _mu_rectangle_msync_factory,               _context_id); 
          geometry->rmCollective(PAMI_XFER_BARRIER,        _gi_msync_factory,                         _context_id);
          geometry->rmCollective(PAMI_XFER_BARRIER,        _binomial_barrier_factory,                 _context_id);
          geometry->rmCollective(PAMI_XFER_BARRIER,        _msync2d_composite_factory,                _context_id);
          geometry->rmCollective(PAMI_XFER_BARRIER,        _msync2d_gishm_composite_factory,          _context_id);
          geometry->rmCollective(PAMI_XFER_BARRIER,        _msync2d_rectangle_composite_factory,      _context_id);

          geometry->rmCollectiveCheck(PAMI_XFER_ALLREDUCE, _mushmemcollectivedputmulticombinefactory, _context_id);
          geometry->rmCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2d_dput_composite_factory,           _context_id);

#ifdef PAMI_ENABLE_X0_PROTOCOLS
          geometry->rmCollectiveCheck(PAMI_XFER_BROADCAST, _mu_mcast3_factory,                _context_id);
          geometry->rmCollectiveCheck(PAMI_XFER_BROADCAST, _mcast2d_composite_factory,        _context_id);

          geometry->rmCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2dNP_dput_composite_factory, _context_id);
          geometry->rmCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2d_composite_factory,        _context_id);
          geometry->rmCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2dNP_composite_factory,      _context_id);
#endif

        }

        TRACE_FN_EXIT();
        return PAMI_SUCCESS;
      }
    public:
      pami_client_t                                   _client;
      pami_context_t                                  _context;
      size_t                                          _context_id;
      // This is a pointer to the current dispatch id of the context
      // This will be decremented by the ConstructNativeInterface routines
      int                                            *_dispatch_id;
      std::map<unsigned, pami_geometry_t>            *_geometry_map;

      // CCMI Connection Manager Class
      CCMI::ConnectionManager::SimpleConnMgr          _sconnmgr;
      CCMI::ConnectionManager::CommSeqConnMgr         _csconnmgr;
      CCMI::ConnectionManager::ColorGeometryConnMgr   _cg_connmgr;
      CCMI::ConnectionManager::ColorConnMgr           _color_connmgr;

      // Barrier Storage
      CCMI::Executor::Composite                      *_shmem_barrier_composite;
      CCMI::Executor::Composite                      *_gi_barrier_composite;
      CCMI::Executor::Composite                      *_mu_rectangle_barrier_composite;
      CCMI::Executor::Composite                      *_msync_composite;
      CCMI::Executor::Composite                      *_msync2d_composite;
      CCMI::Executor::Composite                      *_msync2d_gishm_composite;
      CCMI::Executor::Composite                      *_msync2d_rectangle_composite;

      //* SHMEM interfaces:
      // Shmem Device
      T_ShmemDevice                                  &_shmem_device;

      // Native Interface
      T_ShmemNativeInterface                         *_shmem_ni;

      // CCMI Barrier Interface
      ShmemMultiSyncFactory                           _shmem_msync_factory;

      // CCMI Broadcast Interfaces
      ShmemMultiCastFactory                           _shmem_mcast_factory;

      // CCMI Allreduce Interface
      ShmemMultiCombineFactory                        _shmem_mcomb_factory;

      // MU Device
      T_MUDevice                                     &_mu_device;

      // MU Native Interface

      T_MUNativeInterface                            *_mu_ni_msync;
      uint8_t                                         _mu_ni_msync_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_mcomb;
      uint8_t                                         _mu_ni_mcomb_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_mcast;
      uint8_t                                         _mu_ni_mcast_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_mcast3;
      uint8_t                                         _mu_ni_mcast3_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_msync2d;
      uint8_t                                         _mu_ni_msync2d_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_mcast2d;
      uint8_t                                         _mu_ni_mcast2d_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_mcomb2d;
      uint8_t                                         _mu_ni_mcomb2d_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_mcomb2dNP;
      uint8_t                                         _mu_ni_mcomb2dNP_storage[sizeof(T_MUNativeInterface)];

      T_AxialDputNativeInterface                     *_axial_mu_dput_ni;
      uint8_t                                         _axial_mu_dput_ni_storage[sizeof(T_AxialDputNativeInterface)];

      T_AxialShmemDputNativeInterface                *_axial_shmem_mu_dput_ni;
      uint8_t                                         _axial_shmem_mu_dput_ni_storage[sizeof(T_AxialShmemDputNativeInterface)];

      MUGlobalDputNI                                 *_mu_global_dput_ni;
      uint8_t                                         _mu_global_dput_ni_storage [sizeof(MUGlobalDputNI)];

      MUShmemGlobalDputNI                            *_mu_shmem_global_dput_ni;
      uint8_t                                         _mu_shmem_global_dput_ni_storage [sizeof(MUShmemGlobalDputNI)];

      MUAMMulticastNI                                 *_mu_ammulticast_ni;
      uint8_t                                         _mu_ammulticast_ni_storage [sizeof(MUAMMulticastNI)];

      M2MNISingle                                    *_mu_m2m_single_ni;
      uint8_t                                         _mu_m2m_single_ni_storage [sizeof(M2MNISingle)];

      M2MNIVectorLong                                *_mu_m2m_vector_long_ni;
      uint8_t                                         _mu_m2m_vector_long_ni_storage [sizeof(M2MNIVectorLong)];

      M2MNIVectorInt                                 *_mu_m2m_vector_int_ni;
      uint8_t                                         _mu_m2m_vector_int_ni_storage [sizeof(M2MNIVectorInt)];

      // Barrier factories
      GIMultiSyncFactory                             *_gi_msync_factory;
      uint8_t                                         _gi_msync_factory_storage[sizeof(GIMultiSyncFactory)];

      MURectangleMultiSyncFactory                    *_mu_rectangle_msync_factory;
      uint8_t                                         _mu_rectangle_msync_factory_storage[sizeof(MURectangleMultiSyncFactory)];

      OptBinomialBarrierFactory                      *_binomial_barrier_factory;
      uint8_t                                         _binomial_barrier_factory_storage[sizeof(OptBinomialBarrierFactory)];

      // Broadcast factories
      MUMultiCastFactory                             *_mu_mcast_factory;
      uint8_t                                         _mu_mcast_factory_storage[sizeof(MUMultiCastFactory)];
#ifdef PAMI_ENABLE_X0_PROTOCOLS
      MUMultiCast3Factory                            *_mu_mcast3_factory;
      uint8_t                                         _mu_mcast3_factory_storage[sizeof(MUMultiCast3Factory)];
#endif
      // Allreduce factories
      MUMultiCombineFactory                          *_mu_mcomb_factory;
      uint8_t                                         _mu_mcomb_factory_storage[sizeof(MUMultiCombineFactory)];

      // Barrier factories

      // 2 device composite factories
      CCMI::Interfaces::NativeInterface              *_ni_array[8];
      MultiSync2DeviceFactory                        *_msync2d_composite_factory;
      uint8_t                                         _msync2d_composite_factory_storage[sizeof(MultiSync2DeviceFactory)];

      MultiSync2DeviceGIShmemFactory                 *_msync2d_gishm_composite_factory;
      uint8_t                                         _msync2d_gishm_composite_factory_storage[sizeof(MultiSync2DeviceGIShmemFactory)];

      MultiSync2DeviceRectangleFactory               *_msync2d_rectangle_composite_factory;
      uint8_t                                         _msync2d_rectangle_composite_factory_storage[sizeof(MultiSync2DeviceRectangleFactory)];

#ifdef PAMI_ENABLE_X0_PROTOCOLS
      MultiCast2DeviceFactory                        *_mcast2d_composite_factory;
      uint8_t                                         _mcast2d_composite_factory_storage[sizeof(MultiCast2DeviceFactory)];
#endif
      MultiCast2DeviceDputFactory                    *_mcast2d_dput_composite_factory;
      uint8_t                                         _mcast2d_dput_composite_factory_storage[sizeof(MultiCast2DeviceDputFactory)];

#ifdef PAMI_ENABLE_X0_PROTOCOLS
      MultiCombine2DeviceFactory                     *_mcomb2d_composite_factory;
      uint8_t                                         _mcomb2d_composite_factory_storage[sizeof(MultiCombine2DeviceFactory)];
#endif
#ifdef PAMI_ENABLE_X0_PROTOCOLS
      MultiCombine2DeviceFactoryNP                   *_mcomb2dNP_composite_factory;
      uint8_t                                         _mcomb2dNP_composite_factory_storage[sizeof(MultiCombine2DeviceFactoryNP)];
#endif
      MultiCombine2DeviceDputFactory                 *_mcomb2d_dput_composite_factory;
      uint8_t                                         _mcomb2d_dput_composite_factory_storage[sizeof(MultiCombine2DeviceDputFactory)];

#ifdef PAMI_ENABLE_X0_PROTOCOLS
      MultiCombine2DeviceDputFactoryNP               *_mcomb2dNP_dput_composite_factory;
      uint8_t                                         _mcomb2dNP_dput_composite_factory_storage[sizeof(MultiCombine2DeviceDputFactoryNP)];
#endif

      MUCollectiveDputMulticastFactory               *_mucollectivedputmulticastfactory;
      uint8_t                                         _mucollectivedputmulticaststorage[sizeof(MUCollectiveDputMulticastFactory)];

      MUCollectiveDputMulticombineFactory            *_mucollectivedputmulticombinefactory;
      uint8_t                                         _mucollectivedputmulticombinestorage[sizeof(MUCollectiveDputMulticombineFactory)];

      MUShmemCollectiveDputMulticombineFactory            *_mushmemcollectivedputmulticombinefactory;
      uint8_t                                         _mushmemcollectivedputmulticombinestorage[sizeof(MUShmemCollectiveDputMulticombineFactory)];

      MURectangleDput1ColorBroadcastFactory          *_mu_rectangle_1color_dput_broadcast_factory;
      uint8_t                                         _mu_rectangle_1color_dput_broadcast_factory_storage[sizeof(MURectangleDput1ColorBroadcastFactory)];

      MURectangleDputBroadcastFactory                *_mu_rectangle_dput_broadcast_factory;
      uint8_t                                         _mu_rectangle_dput_broadcast_factory_storage[sizeof(MURectangleDputBroadcastFactory)];

      RectangleDputAllgatherFactory                  *_mu_rectangle_dput_allgather_factory;
      uint8_t                                         _mu_rectangle_dput_allgather_factory_storage[sizeof(RectangleDputAllgatherFactory)];

      RectangleDput1ColorBroadcastFactory            *_shmem_mu_rectangle_1color_dput_broadcast_factory;
      uint8_t                                         _shmem_mu_rectangle_1color_dput_broadcast_factory_storage[sizeof(RectangleDput1ColorBroadcastFactory)];

      RectangleDputBroadcastFactory                  *_shmem_mu_rectangle_dput_broadcast_factory;
      uint8_t                                         _shmem_mu_rectangle_dput_broadcast_factory_storage[sizeof(RectangleDputBroadcastFactory)];

      RectangleDputAllgatherFactory                  *_shmem_mu_rectangle_dput_allgather_factory;
      uint8_t                                         _shmem_mu_rectangle_dput_allgather_factory_storage[sizeof(RectangleDputAllgatherFactory)];

      // Alltoall
      All2AllFactory                                *_alltoall_factory;
      uint8_t                                        _alltoall_factory_storage[sizeof(All2AllFactory)];

      All2AllvFactory                               *_alltoallv_factory;
      uint8_t                                        _alltoallv_factory_storage[sizeof(All2AllvFactory)];

      All2AllvFactory_int                           *_alltoallv_int_factory;
      uint8_t                                        _alltoallv_int_factory_storage[sizeof(All2AllvFactory_int)];
    };



  };
};

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
