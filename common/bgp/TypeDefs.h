/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/bgp/TypeDefs.h
 * \brief ???
 */

#ifndef __common_bgp_TypeDefs_h__
#define __common_bgp_TypeDefs_h__

#define ENABLE_SHMEM_DEVICE
//#define ENABLE_MU_DEVICE

#include "algorithms/geometry/Geometry.h"

#include "components/devices/workqueue/LocalBcastWQMessage.h"

#include "components/devices/workqueue/LocalReduceWQMessage.h"

#include "components/atomic/gcc/GccCounter.h"
#include "components/atomic/counter/CounterBarrier.h"
#include "components/devices/misc/AtomicBarrierMsg.h"

#include "common/bgp/NativeInterface.h"


namespace PAMI
{
  typedef Geometry::Common                     BGPGeometry;


  typedef PAMI::Barrier::CounterBarrier<PAMI::Counter::GccNodeCounter> Barrier_Type;

  typedef PAMI::Device::AtomicBarrierMdl<Barrier_Type> Barrier_Model;

  typedef BGPNativeInterfaceAS <Device::LocalBcastWQModel, Barrier_Model,Device::LocalReduceWQModel> AllSidedNI;
}

//#define PAMI_COLL_MCAST_CLASS
//#define PAMI_COLL_M2M_CLASS
//#define PAMI_COLL_SYSDEP_CLASS SysDep
//#define PAMI_NATIVEINTERFACE
#define PAMI_GEOMETRY_CLASS    PAMI::BGPGeometry

#endif
