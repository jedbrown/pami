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

#include "util/ccmi_debug.h" // tracing
#include "util/ccmi_util.h"

#ifndef CCMI_TRACE_ALL
  #undef TRACE_INIT
  #define TRACE_INIT(x) //fprintf x
#endif

namespace PAMI
{
  namespace CollRegistration
  {

    /// 
    /// Metadata functions
    /// 
    // The protocol requires T_Aligned buffers.  Call some other T_Function (maybe check dt/op?), then check the alignment.
    template <unsigned T_Alignment, pami_metadata_function T_Function>
    inline metadata_result_t align_metadata_function(struct pami_xfer_t *in)
    {
        metadata_result_t result = T_Function(in);
        uint64_t  mask  = T_Alignment - 1; 
        result.check.align_send_buffer      |= (((uint64_t)in->cmd.xfer_allreduce.sndbuf & (uint64_t)mask) == 0) ? 0:1;
        result.check.align_send_recv_buffer |= (((uint64_t)in->cmd.xfer_allreduce.rcvbuf & (uint64_t)mask) == 0) ? 0:1;
        return result;
    }

    // The protocol only supports some dt/op's.  
    // Two functions - one for MU dt/ops, one for current SHMEM dt/ops
    namespace MU
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
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_SIGNED_INT
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_UNSIGNED_INT
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_SIGNED_LONG_LONG
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_UNSIGNED_LONG_LONG
          {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_FLOAT
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_DOUBLE
          {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_LONG_DOUBLE
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
        TRACE((stderr, "MU::op_dt_metadata_function(dt %d,op %d) = %s\n", dt, op, support[dt][op] ? "true" : "false"));
        metadata_result_t result = {0};
        result.check.datatype = support[in->cmd.xfer_allreduce.dt][in->cmd.xfer_allreduce.op]?0:1;
        return(result);
      }
    }

#ifdef ENABLE_NEW_SHMEM
    namespace Shmem
    {
      inline metadata_result_t op_dt_metadata_function(struct pami_xfer_t *in)
      {
        TRACE((stderr, "Shmem::op_dt_metadata_function(dt %d,op %d) = %s\n", dt, op, support[dt][op] ? "true" : "false"));
        metadata_result_t result = {0};
        result.check.datatype = ((in->cmd.xfer_allreduce.dt == PAMI_DOUBLE) && (in->cmd.xfer_allreduce.op == PAMI_SUM))?0:1;
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
    void ShmemMsyncMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiSync:SHMEM:-");
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite<>,
    ShmemMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > ShmemMultiSyncFactory;

    void OptBinomialMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:OptBinomial:P2P:P2P");
      m->check_perf.values.hw_accel     = 1;
    }

    bool opt_binomial_analyze (PAMI_GEOMETRY_CLASS *geometry)
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

    //----------------------------------------------------------------------------
    // 'Pure' Shmem allsided multicombine
    //----------------------------------------------------------------------------
    void ShmemMcombMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombine:SHMEM:-");
#ifdef ENABLE_NEW_SHMEM
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->range_hi                       = 64; // Msgs > 64 are pseudo-reduce-only, not allreduce
      m->check_fn                       = Shmem::op_dt_metadata_function;
#endif
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite<>,
    ShmemMcombMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > ShmemMultiCombineFactory;

    //----------------------------------------------------------------------------
    // 'Pure' Shmem allsided multicast
    //----------------------------------------------------------------------------
    void ShmemMcastMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCast:SHMEM:-");
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite<>,
    ShmemMcastMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > ShmemMultiCastFactory;


    //----------------------------------------------------------------------------
    // 'Pure' MU allsided multicast
    //----------------------------------------------------------------------------
    void MUMcastMetaData(pami_metadata_t *m)
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
    void GIMsyncMetaData(pami_metadata_t *m)
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
    void MURectangleMsyncMetaData(pami_metadata_t *m)
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
    void MUMcombMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombine:-:MU");
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->check_fn                       = MU::op_dt_metadata_function;
      m->check_perf.values.hw_accel     = 1;
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite<>,
    MUMcombMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUMultiCombineFactory;

    //----------------------------------------------------------------------------
    // 'Pure' MU allsided dput multicast
    //----------------------------------------------------------------------------
    void MUMcastCollectiveDputMetaData(pami_metadata_t *m)
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
    void MUMcombCollectiveDputMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombineDput:-:MU");
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->check_fn                       = MU::op_dt_metadata_function;
      m->check_perf.values.hw_accel     = 1;
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite<true, MUGlobalDputNI>,
    MUMcombCollectiveDputMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUCollectiveDputMulticombineFactory;

    //----------------------------------------------------------------------------
    // MU+Shmem allsided dput multicombine
    //----------------------------------------------------------------------------
    void MUShmemMcombCollectiveDputMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombineDput:SHMEM:MU");
#ifdef ENABLE_NEW_SHMEM
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->check_fn                       = align_metadata_function<64,Shmem::op_dt_metadata_function>;
      m->check_perf.values.hw_accel     = 1;
      /// \todo m->check_correct.values.sendminalign = ?   // Need ticket #380 clarification
      /// \todo m->check_correct.values.sendminalign = ?
#else
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->check_fn                       = align_metadata_function<64,MU::op_dt_metadata_function>;
      m->check_perf.values.hw_accel     = 1;
#endif
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite<true, MUShmemGlobalDputNI>,
    MUShmemMcombCollectiveDputMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUShmemCollectiveDputMulticombineFactory;

#ifdef ENABLE_X0_PROTOCOLS
    //----------------------------------------------------------------------------
    // MU allsided multicast built on multicombine (BOR)
    //----------------------------------------------------------------------------
    void MUMcast3MetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("X0:MultiCast_MultiCombine:-:MU");
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
    void Msync2DMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiSync2Device:SHMEM:MU");
      m->check_perf.values.hw_accel     = 1;
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
    Msync2DMetaData, CCMI::ConnectionManager::SimpleConnMgr >  MultiSync2DeviceFactory;

    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multisync
    //----------------------------------------------------------------------------
    void Msync2DGIShmemMetaData(pami_metadata_t *m)
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
    void Msync2DRectangleDputMetaData(pami_metadata_t *m)
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


#ifdef ENABLE_X0_PROTOCOLS
    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multicast
    //----------------------------------------------------------------------------
    void Mcast2DMetaData(pami_metadata_t *m)
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
    void Mcast2DDputMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCast2DeviceDput:SHMEM:MU");
      m->check_perf.values.hw_accel     = 1;
    }
    typedef CCMI::Adaptor::Broadcast::MultiCastComposite2DeviceFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite2DeviceAS<PAMI_GEOMETRY_CLASS>,
    Mcast2DDputMetaData, CCMI::ConnectionManager::SimpleConnMgr > MultiCast2DeviceDputFactory;

#ifdef ENABLE_X0_PROTOCOLS
    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multicombine
    //----------------------------------------------------------------------------
    void Mcomb2DMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("X0:MultiCombine2Device:SHMEM:MU");
#ifdef ENABLE_NEW_SHMEM
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->check_fn                       = Shmem::op_dt_metadata_function;
      m->check_perf.values.hw_accel     = 1;
#else
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
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
    void Mcomb2DDputMetaData(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombine2DeviceDput:SHMEM:MU");
#ifdef ENABLE_NEW_SHMEM
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->check_fn                       = align_metadata_function<32,Shmem::op_dt_metadata_function>;
      m->check_perf.values.hw_accel     = 1;
#else
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->check_fn                       = align_metadata_function<32,MU::op_dt_metadata_function>;
      m->check_perf.values.hw_accel     = 1;
#endif
    }
    typedef CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2Device<0>,
    Mcomb2DDputMetaData, CCMI::ConnectionManager::SimpleConnMgr >    MultiCombine2DeviceDputFactory;


    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multicombine with no pipelining
    //----------------------------------------------------------------------------
    void Mcomb2DMetaDataNP(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombine2DeviceNP:SHMEM:MU");
#ifdef ENABLE_NEW_SHMEM
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->check_fn                       = align_metadata_function<32,Shmem::op_dt_metadata_function>;
      m->check_perf.values.hw_accel     = 1;
#else
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->check_fn                       = align_metadata_function<32,MU::op_dt_metadata_function>;
      m->check_perf.values.hw_accel     = 1;
#endif
    }
    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceNP,
    Mcomb2DMetaDataNP, CCMI::ConnectionManager::SimpleConnMgr > MultiCombine2DeviceFactoryNP;

    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU-DPUT allsided 2 device multicombine with no pipelining
    //----------------------------------------------------------------------------
    void Mcomb2DDputMetaDataNP(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:MultiCombine2DeviceDputNP:SHMEM:MU");
#ifdef ENABLE_NEW_SHMEM
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->check_fn                       = align_metadata_function<32,Shmem::op_dt_metadata_function>;
      m->check_perf.values.hw_accel     = 1;
#else
      m->check_correct.values.alldt     = 0;
      m->check_correct.values.allop     = 0;
      m->check_fn                       = align_metadata_function<32,MU::op_dt_metadata_function>;
      m->check_perf.values.hw_accel     = 1;
#endif
    }
    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceNP,
    Mcomb2DDputMetaDataNP, CCMI::ConnectionManager::SimpleConnMgr > MultiCombine2DeviceDputFactoryNP;

    //----------------------------------------------------------------------------
    // Rectangle broadcast
    //----------------------------------------------------------------------------
    void get_colors (PAMI::Topology             * t,
                     unsigned                    bytes,
                     unsigned                  * colors,
                     unsigned                  & ncolors)
    {
      TRACE_INIT((stderr, "get_colors\n"));
      ncolors = 1;
      colors[0] = CCMI::Schedule::TorusRect::NO_COLOR;
    }

    void get_rect_colors (PAMI::Topology             * t,
                          unsigned                    bytes,
                          unsigned                  * colors,
                          unsigned                  & ncolors)
    {

      unsigned max = 0, ideal = 0;
      unsigned _colors[10];
      CCMI::Schedule::TorusRect::getColors (t, ideal, max, _colors);
      TRACE_INIT((stderr, "get_rect_colors() bytes %u, ncolors %u, ideal %u, max %u\n", bytes, ncolors, ideal, max));

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

      TRACE_INIT((stderr, "get_rect_colors() ncolors %u, ideal %u\n", ncolors, ideal));
      memcpy (colors, _colors, ncolors * sizeof(int));
    }

    void rectangle_dput_1color_broadcast_metadata(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:RectangleDput1Color:SHMEM:MU");
      m->check_perf.values.hw_accel     = 1;
    }

    void mu_rectangle_dput_1color_broadcast_metadata(pami_metadata_t *m)
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

    void rectangle_dput_broadcast_metadata(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:RectangleDput:SHMEM:MU");
      m->check_perf.values.hw_accel     = 1;
    }

    void mu_rectangle_dput_broadcast_metadata(pami_metadata_t *m)
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

    void rectangle_dput_allgather_metadata(pami_metadata_t *m)
    {
      new(m) PAMI::Geometry::Metadata("I0:RectangleDput:SHMEM:MU");
      m->check_perf.values.hw_accel     = 1;
    }

    void get_rect_allgv_colors (PAMI::Topology             * t,
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
    template <class T_Geometry, class T_ShmemDevice, class T_ShmemNativeInterface, class T_MUDevice, class T_MUNativeInterface, class T_AxialNativeInterface, class T_AxialDputNativeInterface, class T_AxialShmemDputNativeInterface>
    class BGQMultiRegistration :
    public CollRegistration<PAMI::CollRegistration::BGQMultiRegistration<T_Geometry, T_ShmemDevice, T_ShmemNativeInterface, T_MUDevice, T_MUNativeInterface, T_AxialNativeInterface, T_AxialDputNativeInterface, T_AxialShmemDputNativeInterface>, T_Geometry>
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
      CollRegistration<PAMI::CollRegistration::BGQMultiRegistration<T_Geometry, T_ShmemDevice, T_ShmemNativeInterface, T_MUDevice, T_MUNativeInterface, T_AxialNativeInterface, T_AxialDputNativeInterface, T_AxialShmemDputNativeInterface>, T_Geometry> (),
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
      _axial_mu_ni(NULL),
      _axial_mu_1_ni(NULL),
      _axial_mu_dput_ni(NULL),
      //_axial_dput_mu_1_ni(NULL),
      _gi_msync_factory(NULL),
      _mu_mcast_factory(NULL),
#ifdef ENABLE_X0_PROTOCOLS
      _mu_mcast3_factory(NULL),
#endif
      _mu_mcomb_factory(NULL),
      _msync2d_composite_factory(NULL),
      _msync2d_gishm_composite_factory(NULL),
#ifdef ENABLE_X0_PROTOCOLS
      _mcast2d_composite_factory(NULL),
      _mcomb2d_composite_factory(NULL),
#endif
      _mcomb2dNP_composite_factory(NULL),
      _mcomb2d_dput_composite_factory(NULL),
      _mcomb2dNP_dput_composite_factory(NULL),
      _mu_rectangle_1color_dput_broadcast_factory(NULL),
      _mu_rectangle_dput_broadcast_factory(NULL),
      _mu_rectangle_dput_allgather_factory(NULL),
      _shmem_mu_rectangle_1color_dput_broadcast_factory(NULL),
      _shmem_mu_rectangle_dput_broadcast_factory(NULL),
      _shmem_mu_rectangle_dput_allgather_factory(NULL)
      {
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration()\n", this));
        //DO_DEBUG((templateName<T_Geometry>()));
        //DO_DEBUG((templateName<T_ShmemNativeInterface>()));
        //DO_DEBUG((templateName<T_MUDevice>()));
        //DO_DEBUG((templateName<T_MUNativeInterface>()));
        //DO_DEBUG((templateName<T_AxialNativeInterface>()));
        //DO_DEBUG((templateName<T_AxialDputNativeInterface>()));

        //set the mapid functions
        if (__global.useshmem())// && (__global.topology_local.size() > 1))
        {
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration() useshmem\n", this));
          _shmem_msync_factory.setMapIdToGeometry(mapidtogeometry);
        }

        if (__global.useMU())
        {
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration() usemu\n", this));

          _mu_ni_msync          = new (_mu_ni_msync_storage         ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcomb          = new (_mu_ni_mcomb_storage         ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcast          = new (_mu_ni_mcast_storage         ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcast3         = new (_mu_ni_mcast3_storage        ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_msync2d        = new (_mu_ni_msync2d_storage       ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcast2d        = new (_mu_ni_mcast2d_storage       ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcomb2d        = new (_mu_ni_mcomb2d_storage       ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _mu_ni_mcomb2dNP      = new (_mu_ni_mcomb2dNP_storage     ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);
          _axial_mu_ni          = new (_axial_mu_ni_storage         ) T_AxialNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);

          _axial_mu_dput_ni     = new (_axial_mu_dput_ni_storage    ) T_AxialDputNativeInterface(_mu_device, client, context, context_id, client_id, _dispatch_id);

          if (_axial_mu_dput_ni->status() != PAMI_SUCCESS) _axial_mu_dput_ni = NULL; // Not enough resources?

          if (__global.mapping.t() == 0)
            //We can now construct this on any process (as long as
            //process 0 on each node in the job also calls it
            _mu_global_dput_ni    = new (_mu_global_dput_ni_storage) MUGlobalDputNI (_mu_device, client, context, context_id, client_id, _dispatch_id);

          _mu_shmem_global_dput_ni    = new (_mu_shmem_global_dput_ni_storage) MUShmemGlobalDputNI (_mu_device, client, context, context_id, client_id, _dispatch_id);

          _mu_ammulticast_ni    = new (_mu_ammulticast_ni_storage) MUAMMulticastNI (_mu_device, client, context, context_id, client_id, _dispatch_id);

          if (_mu_ammulticast_ni->status() != PAMI_SUCCESS) _mu_ammulticast_ni = NULL;

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

          _mucollectivedputmulticastfactory    = new (_mucollectivedputmulticaststorage ) MUCollectiveDputMulticastFactory(&_sconnmgr, _mu_global_dput_ni);
          _mucollectivedputmulticombinefactory    = new (_mucollectivedputmulticombinestorage ) MUCollectiveDputMulticombineFactory(&_sconnmgr, _mu_global_dput_ni);        

          _mushmemcollectivedputmulticombinefactory    = new (_mushmemcollectivedputmulticombinestorage ) MUShmemCollectiveDputMulticombineFactory(&_sconnmgr, _mu_shmem_global_dput_ni);       

          if (_axial_shmem_mu_dput_ni)
          {
            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration()  RectangleDput1ColorBroadcastFactory\n", this));
            _shmem_mu_rectangle_1color_dput_broadcast_factory = new (_shmem_mu_rectangle_1color_dput_broadcast_factory_storage) RectangleDput1ColorBroadcastFactory(&_color_connmgr, _axial_shmem_mu_dput_ni);

            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration()  RectangleDputBroadcastFactory\n", this));
            _shmem_mu_rectangle_dput_broadcast_factory = new (_shmem_mu_rectangle_dput_broadcast_factory_storage) RectangleDputBroadcastFactory(&_color_connmgr, _axial_shmem_mu_dput_ni);

            _shmem_mu_rectangle_dput_allgather_factory = new (_shmem_mu_rectangle_dput_allgather_factory_storage) RectangleDputAllgatherFactory(&_color_connmgr, _axial_shmem_mu_dput_ni);
          }

          if (_axial_mu_dput_ni)
          {
            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration()  MURectangleDput1ColorBroadcastFactory\n", this));
            _mu_rectangle_1color_dput_broadcast_factory = new (_mu_rectangle_1color_dput_broadcast_factory_storage) MURectangleDput1ColorBroadcastFactory(&_color_connmgr, _axial_mu_dput_ni);

            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration()  MURectangleDputBroadcastFactory\n", this));
            _mu_rectangle_dput_broadcast_factory = new (_mu_rectangle_dput_broadcast_factory_storage) MURectangleDputBroadcastFactory(&_color_connmgr, _axial_mu_dput_ni);

            _mu_rectangle_dput_allgather_factory = new (_mu_rectangle_dput_allgather_factory_storage) RectangleDputAllgatherFactory(&_color_connmgr, _axial_mu_dput_ni);
          }

          if (_gi_msync_factory)
            _gi_msync_factory->setMapIdToGeometry(mapidtogeometry);

          // Can't be ctor'd unless the NI was created
          _mu_mcast_factory  = new (_mu_mcast_factory_storage ) MUMultiCastFactory(&_sconnmgr, _mu_ni_mcast);
#ifdef ENABLE_X0_PROTOCOLS
          _mu_mcast3_factory = new (_mu_mcast3_factory_storage) MUMultiCast3Factory(&_sconnmgr, _mu_ni_mcast3);
#endif
        }

//          if ((__global.useMU()) && (__global.useshmem()))
        {
          _ni_array[0] = _shmem_ni;
          _ni_array[1] = _mu_ni_msync2d;
          _ni_array[2] = _shmem_ni;
          _ni_array[3] = _mu_ni_mcomb2dNP;
          _msync2d_composite_factory = new (_msync2d_composite_factory_storage) MultiSync2DeviceFactory(&_sconnmgr, (CCMI::Interfaces::NativeInterface*)&_ni_array[0]);
          _msync2d_composite_factory->setMapIdToGeometry(mapidtogeometry);

          _ni_array[4] = _shmem_ni;
          _ni_array[5] = _mu_global_dput_ni;
          _msync2d_gishm_composite_factory = new (_msync2d_gishm_composite_factory_storage) MultiSync2DeviceGIShmemFactory(&_sconnmgr, (CCMI::Interfaces::NativeInterface*)&_ni_array[4]);
          _msync2d_gishm_composite_factory->setMapIdToGeometry(mapidtogeometry);

          _ni_array[6] = _shmem_ni;
          _ni_array[7] = _axial_mu_dput_ni;
          _msync2d_rectangle_composite_factory = new (_msync2d_rectangle_composite_factory_storage) MultiSync2DeviceRectangleFactory(&_sconnmgr, (CCMI::Interfaces::NativeInterface*)&_ni_array[6]);
          _msync2d_rectangle_composite_factory->setMapIdToGeometry(mapidtogeometry);

          _mcomb2d_dput_composite_factory = new (_mcomb2d_dput_composite_factory_storage) MultiCombine2DeviceDputFactory(&_sconnmgr, _shmem_ni, _mu_global_dput_ni);
          _mcomb2dNP_dput_composite_factory = new (_mcomb2dNP_dput_composite_factory_storage) MultiCombine2DeviceDputFactoryNP(&_sconnmgr,  (CCMI::Interfaces::NativeInterface*)&_ni_array[4]);

#ifdef ENABLE_X0_PROTOCOLS
          _mcast2d_composite_factory = new (_mcast2d_composite_factory_storage) MultiCast2DeviceFactory(&_sconnmgr, _shmem_ni, false, _mu_ni_mcast2d,  _mu_ni_mcast2d ? true : false);
#endif
          _mcast2d_dput_composite_factory = new (_mcast2d_dput_composite_factory_storage) MultiCast2DeviceDputFactory(&_sconnmgr, _shmem_ni, false, _mu_global_dput_ni,  false);

#ifdef ENABLE_X0_PROTOCOLS
          _mcomb2d_composite_factory = new (_mcomb2d_composite_factory_storage) MultiCombine2DeviceFactory(&_sconnmgr, _shmem_ni, _mu_ni_mcomb2d);
#endif
          _mcomb2dNP_composite_factory = new (_mcomb2dNP_composite_factory_storage) MultiCombine2DeviceFactoryNP(&_sconnmgr,  (CCMI::Interfaces::NativeInterface*)&_ni_array[2]);
        }

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
              //fprintf (stderr, "Calling configure with class route %ld, in 0x%lx\n", i, result);
              _axial_mu_dput_ni->getMsyncModel().configureClassRoute(i, (PAMI::Topology *)geometry->getTopology(PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX));
              geometry->setKey (PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID1, (void*)(i+1));
              break;
            }
        }

        return PAMI_SUCCESS;
      }

      inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
      {
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() phase %d, context_id %zu, geometry %p, msync %p, mcast %p, mcomb %p\n", this, phase, context_id, geometry, &_shmem_msync_factory, &_shmem_mcast_factory, &_shmem_mcomb_factory));
        pami_xfer_t xfer = {0};
        // BE CAREFUL! It's not ok to make registration decisions based on local topology unless you know that all nodes will make the same decision.
        // We use local_sub_topology on single node registrations 
        // and on MU/Classroute registrations (because they are rectangular so all nodes have the same local subtopology).
        PAMI::Topology * topology = (PAMI::Topology*) geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX);
        PAMI::Topology * local_sub_topology = (PAMI::Topology*) geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX);
        PAMI::Topology * master_sub_topology = (PAMI::Topology*) geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() topology: size() %zu, isLocal() %u/%zu, isGlobal #u/%zu\n", this, topology->size(),  topology->isLocalToMe(), local_sub_topology->size(), master_sub_topology->size()));//,  topology->isGlobal()));

        //DO_DEBUG(for (unsigned i = 0; i < topology->size(); ++i) fprintf(stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() topology[%u] = %u\n", this, i, topology->index2Rank(i)););

        if (phase == 0)
        {

          if (_binomial_barrier_factory)
          {
            //Set optimized barrier to binomial. May override optimized barrier later
            pami_xfer_t xfer = {0};
            OptBinomialBarrier *opt_binomial = (OptBinomialBarrier *)
                                               _binomial_barrier_factory->generate(geometry, &xfer);
            geometry->setKey(context_id, PAMI::Geometry::CKEY_OPTIMIZEDBARRIERCOMPOSITE,
                             (void*)opt_binomial);
          }

          if ((__global.useshmem())  && (__global.topology_local.size() > 1)
#ifndef ENABLE_SHMEM_SUBNODE
              && (__global.topology_local.size() == local_sub_topology->size()) /// \todo might ease this restriction later - when shmem supports it
#endif
             )
          {
            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Shmem local barrier\n", this));

            // If the geometry is all local nodes, we can use pure shmem composites.
            if (topology->isLocalToMe())
            {
              TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Local Shmem factories\n", this));
              _shmem_barrier_composite = _shmem_msync_factory.generate(geometry, &xfer);
              // Add Barriers
              geometry->addCollective(PAMI_XFER_BARRIER, &_shmem_msync_factory, _context_id);

              // Add Broadcasts
                geometry->addCollective(PAMI_XFER_BROADCAST, &_shmem_mcast_factory, _context_id);

              // Add Allreduces
#ifdef ENABLE_NEW_SHMEM   // limited support - 4/8/16 processes only
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
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Analyze Rectangle factories %p/%p, %p/%p, isLocal? %u\n", this,
                      _mu_rectangle_1color_dput_broadcast_factory, _mu_rectangle_dput_broadcast_factory,
                      _shmem_mu_rectangle_1color_dput_broadcast_factory, _shmem_mu_rectangle_dput_broadcast_factory,
                      topology->isLocal()));

          // Is there a coordinate topology? Try rectangle protocols
          PAMI::Topology * rectangle = (PAMI::Topology*)geometry->getTopology(PAMI::Geometry::COORDINATE_TOPOLOGY_INDEX);

          if (rectangle->type() == PAMI_COORD_TOPOLOGY) // could be EMPTY if not valid on this geometry
          {
            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Rectangle\n", this));
            if (_mu_rectangle_msync_factory && __global.topology_local.size() == 1 &&
                geometry->getKey(PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID1))
            {
              //Set optimized barrier to rectangle. May override optimized barrier later
              pami_xfer_t xfer = {0};
              CCMI::Executor::Composite *opt_composite =  _mu_rectangle_msync_factory->generate(geometry, &xfer); //_gi_msync_factory->generate(geometry, &xfer);               
              geometry->setKey(context_id, PAMI::Geometry::CKEY_OPTIMIZEDBARRIERCOMPOSITE,
                               (void*)opt_composite);
            }
            else if (_msync2d_rectangle_composite_factory && __global.topology_local.size() > 1 && 
                     __global.useMU() && __global.useshmem() &&               
                     geometry->getKey(PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID1))
            {
              //Set optimized barrier to rectangle. May override optimized barrier later
              pami_xfer_t xfer = {0};
              CCMI::Executor::Composite *opt_composite =  _msync2d_rectangle_composite_factory->generate(geometry, &xfer); 
              geometry->setKey(context_id, PAMI::Geometry::CKEY_OPTIMIZEDBARRIERCOMPOSITE,
                               (void*)opt_composite);
            }

            // Add rectangle protocols:
            if ((_shmem_mu_rectangle_1color_dput_broadcast_factory)
#ifndef ENABLE_SHMEM_SUBNODE
                && (__global.topology_local.size() == local_sub_topology->size()) /// \todo might ease this restriction later - when shmem supports it
#endif
               )
              geometry->addCollective(PAMI_XFER_BROADCAST,  _shmem_mu_rectangle_1color_dput_broadcast_factory, _context_id);
            if (_mu_rectangle_1color_dput_broadcast_factory)
              geometry->addCollective(PAMI_XFER_BROADCAST,  _mu_rectangle_1color_dput_broadcast_factory, _context_id);

            if ((_shmem_mu_rectangle_dput_broadcast_factory) 
#ifndef ENABLE_SHMEM_SUBNODE
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

#if 0  // allgatherv hangs 

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
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() usePureMu = %u (size %zu/%zu)\n", this, usePureMu, topology->size(), master_sub_topology->size()));

          // Add optimized binomial barrier
          if (_binomial_barrier_factory)
            geometry->addCollective(PAMI_XFER_BARRIER, _binomial_barrier_factory, _context_id);

#if 0 // test a query barrier protocol
          if (_binomial_barrier_factory)
            geometry->addCollectiveCheck(PAMI_XFER_BARRIER, _binomial_barrier_factory, _context_id);
#endif

          // Check for class routes before enabling MU collective network protocols
          void *val;
          val = geometry->getKey(PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID);
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() GKEY_MSYNC_CLASSROUTEID %p\n", this, val));

          if ((val && val != PAMI_CR_GKEY_FAIL) || // We have a class route or
              (topology->isLocalToMe()))           // It's all local - we might use 2 device protocol in shmem-only mode
          {
            // If we can use pure MU composites, add them
            if (usePureMu)
            {
              if (_gi_msync_factory)
              {
                TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register MU barrier\n", this));
                geometry->setKey(context_id, PAMI::Geometry::CKEY_BARRIERCOMPOSITE3, NULL);
                _gi_barrier_composite = _gi_msync_factory->generate(geometry, &xfer);
                geometry->setKey(context_id, PAMI::Geometry::CKEY_BARRIERCOMPOSITE3,
                                 (void*)_gi_barrier_composite);

                // Add Barriers
                geometry->addCollective(PAMI_XFER_BARRIER, _gi_msync_factory, _context_id);
              }

              if (_mu_rectangle_msync_factory  && geometry->getKey(PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID1)) // \todo PAMI_CR_GKEY_FAIL?
              {
                _mu_rectangle_barrier_composite = _mu_rectangle_msync_factory->generate(geometry, &xfer);
                // Add Barriers
                geometry->addCollective(PAMI_XFER_BARRIER, _mu_rectangle_msync_factory, _context_id); 
              }
            }

            // Add 2 device composite protocols
#ifndef ENABLE_SHMEM_SUBNODE
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
                geometry->setKey(context_id, PAMI::Geometry::CKEY_BARRIERCOMPOSITE4,
                                 (void*)_msync2d_gishm_composite);
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
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() GKEY_MCAST_CLASSROUTEID %p\n", this, val));

          if ((val && val != PAMI_CR_GKEY_FAIL) || // We have a class route or
              (topology->isLocalToMe()))           // It's all local - we might use 2 device protocol in shmem-only mode
          {
            // If we can use pure MU composites, add them
            if (usePureMu)
            {
              TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register MU bcast\n", this));
              // Add Broadcasts
              geometry->addCollective(PAMI_XFER_BROADCAST,  _mu_mcast_factory,  _context_id);
#ifdef ENABLE_X0_PROTOCOLS
              geometry->addCollectiveCheck(PAMI_XFER_BROADCAST,  _mu_mcast3_factory, _context_id);
#endif
            }

            // Add 2 device composite protocols
#ifndef ENABLE_NEW_SHMEM
            // Default Shmem doesn't work with 2 device protocol right now
            if (local_sub_topology->size() == 1)
#endif
#ifndef ENABLE_SHMEM_SUBNODE
            if (__global.topology_local.size() == local_sub_topology->size()) /// \todo might ease this restriction later - when shmem supports it
#endif
            {
#ifdef ENABLE_X0_PROTOCOLS
              if (_mcast2d_composite_factory)
              {
                TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register mcast 2D\n", this));
                geometry->addCollectiveCheck(PAMI_XFER_BROADCAST, _mcast2d_composite_factory, _context_id);
              }
#endif
              if (_mcast2d_dput_composite_factory)
              {
                TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register mcast dput 2D\n", this));
                geometry->addCollective(PAMI_XFER_BROADCAST, _mcast2d_dput_composite_factory, _context_id);
              }
            }
          }

          val = geometry->getKey(PAMI::Geometry::GKEY_MCOMB_CLASSROUTEID);
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() GKEY_MCOMB_CLASSROUTEID %p\n", this, val));

          if ((val && val != PAMI_CR_GKEY_FAIL) || // We have a class route or
              (topology->isLocalToMe()))           // It's all local - we might use 2 device protocol in shmem-only mode
          {
            // If we can use pure MU composites, add them
            if (usePureMu)
            {
              // Direct MU allreduce only on one context per node (lowest T, context 0)
              if ((__global.mapping.isLowestT()) && (_context_id == 0))
              {
                // Add Allreduces
                TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register MU allreduce\n", this));
                geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE, _mu_mcomb_factory, _context_id);
              }
            }

            // Add 2 device composite protocols
#ifdef ENABLE_NEW_SHMEM   // limited support - 4/8/16 processes only
            if ((__global.topology_local.size() ==  4) ||  
                (__global.topology_local.size() ==  8) ||
                (__global.topology_local.size() == 16))
#endif
#ifndef ENABLE_SHMEM_SUBNODE
            if (__global.topology_local.size() == local_sub_topology->size()) /// \todo might ease this restriction later - when shmem supports it
#endif
            {
              // New optimized MU+Shmem protocol requires a class route
              if ((_mushmemcollectivedputmulticombinefactory) && (val && val != PAMI_CR_GKEY_FAIL))
                geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE,  _mushmemcollectivedputmulticombinefactory, _context_id);

              // NP (non-pipelining) 2 device protocols
              if ((_mcomb2dNP_dput_composite_factory) && (master_sub_topology->size() > 1))  // \todo Simple NP protocol doesn't like 1 master - fix it later
                geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2dNP_dput_composite_factory, _context_id);

              if ((_mcomb2dNP_composite_factory) && (master_sub_topology->size() > 1))  // \todo Simple NP protocol doesn't like 1 master - fix it later
                geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2dNP_composite_factory, _context_id);

              //  2 device protocols
              if (_mcomb2d_dput_composite_factory)
                geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2d_dput_composite_factory, _context_id);

#ifdef ENABLE_X0_PROTOCOLS
              if (_mcomb2d_composite_factory)
                geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE, _mcomb2d_composite_factory, _context_id);
#endif
            }
          }
        }
        else if (phase == -1)
        {
          /// \todo remove MU collectives algorithms... TBD
          geometry->rmCollective(PAMI_XFER_BROADCAST, _mu_mcast_factory,  _context_id);
          geometry->rmCollective(PAMI_XFER_ALLREDUCE, _mu_mcomb_factory, _context_id);
          geometry->rmCollective(PAMI_XFER_BARRIER, _gi_msync_factory, _context_id);
        }

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
      T_AxialNativeInterface                         *_axial_mu_ni;
      uint8_t                                         _axial_mu_ni_storage[sizeof(T_AxialNativeInterface)];
      T_AxialNativeInterface                         *_axial_mu_1_ni;
      uint8_t                                         _axial_mu_1_ni_storage[sizeof(T_AxialNativeInterface)];

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
#ifdef ENABLE_X0_PROTOCOLS
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

#ifdef ENABLE_X0_PROTOCOLS
      MultiCast2DeviceFactory                        *_mcast2d_composite_factory;
      uint8_t                                         _mcast2d_composite_factory_storage[sizeof(MultiCast2DeviceFactory)];
#endif
      MultiCast2DeviceDputFactory                    *_mcast2d_dput_composite_factory;
      uint8_t                                         _mcast2d_dput_composite_factory_storage[sizeof(MultiCast2DeviceDputFactory)];

#ifdef ENABLE_X0_PROTOCOLS
      MultiCombine2DeviceFactory                     *_mcomb2d_composite_factory;
      uint8_t                                         _mcomb2d_composite_factory_storage[sizeof(MultiCombine2DeviceFactory)];
#endif
      MultiCombine2DeviceFactoryNP                   *_mcomb2dNP_composite_factory;
      uint8_t                                         _mcomb2dNP_composite_factory_storage[sizeof(MultiCombine2DeviceFactoryNP)];

      MultiCombine2DeviceDputFactory                 *_mcomb2d_dput_composite_factory;
      uint8_t                                         _mcomb2d_dput_composite_factory_storage[sizeof(MultiCombine2DeviceDputFactory)];

      MultiCombine2DeviceDputFactoryNP               *_mcomb2dNP_dput_composite_factory;
      uint8_t                                         _mcomb2dNP_dput_composite_factory_storage[sizeof(MultiCombine2DeviceDputFactoryNP)];

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
    };



  };
};

#ifndef CCMI_TRACE_ALL
  #undef TRACE_INIT
  #define TRACE_INIT(x)
#endif

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
