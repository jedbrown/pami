/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/bgq/TypeDefs.h
 * \brief ???
 */

#ifndef __common_bgq_TypeDefs_h__
#define __common_bgq_TypeDefs_h__

  #include "components/devices/shmem/ShmemDevice.h"
  #include "components/devices/shmem/ShmemPacketModel.h"
  #include "util/fifo/FifoPacket.h"
  #include "util/fifo/LinearFifo.h"
  #include "components/devices/workqueue/LocalBcastWQMessage.h"
//  #include "components/devices/bgq/P2PMcastAM.h"
  #include "components/devices/bgq/mu/MUDevice.h"
  #include "components/devices/bgq/mu/MUPacketModel.h"
  #include "components/devices/bgq/mu/MUInjFifoMessage.h"

  #include "components/devices/bgq/mu/MUCollDevice.h"
  #include "components/devices/bgq/mu/MUMulticastModel.h"
  #include "components/devices/bgq/mu/MUMultisyncModel.h"
  #include "components/devices/bgq/mu/MUMulticombineModel.h"

#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/composite/Composite.h"
#include "p2p/protocols/get/Get.h"

#include "components/atomic/bgq/L2Counter.h"
#include "components/atomic/counter/CounterBarrier.h"
#include "components/devices/misc/AtomicBarrierMsg.h"

#include "common/bgq/NativeInterface.h"

#include "algorithms/geometry/Geometry.h"


namespace PAMI
{
  typedef Geometry::Common                     BGQGeometry;

  typedef Device::MU::MUCollDevice MUDevice;
  typedef BGQNativeInterface < MUDevice,
                               Device::MU::MUMulticastModel,
                               Device::MU::MUMultisyncModel,
                               Device::MU::MUMulticombineModel > MUGlobalNI;

  //typedef Fifo::FifoPacket <32, 992> ShmemPacket;
  //typedef Fifo::LinearFifo<Atomic::BGQ::L2ProcCounter, ShmemPacket, 16> ShmemFifo;
  //typedef Device::Fifo::LinearFifo<Atomic::Pthread,ShmemPacket,16> ShmemFifo;
  //typedef Fifo::LinearFifo<Atomic::BgqAtomic,ShmemPacket,16> ShmemFifo;

  typedef Fifo::FifoPacket <32, 512> ShmemPacket;
  typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 16> ShmemFifo;
  typedef Device::ShmemDevice<ShmemFifo> ShmemDevice;
  typedef Device::Shmem::PacketModel<ShmemDevice> ShmemModel;

  typedef Protocol::Send::Eager <ShmemModel, ShmemDevice> EagerShmem;
  typedef Protocol::Get::Get <ShmemModel, ShmemDevice> GetShmem;

//  typedef Protocol::BGQ::P2PMcastAM<ShmemDevice, EagerShmem, Device::LocalBcastWQModel,Device::LocalBcastWQDevice> ActiveMessageMcast;

  typedef PAMI::Barrier::CounterBarrier<PAMI::Counter::BGQ::L2NodeCounter> Barrier_Type;

  typedef PAMI::Device::AtomicBarrierMdl<Barrier_Type> Barrier_Model;

  /// \todo LocalReduceWQ or LocalAllreduceWQ?
  typedef BGQNativeInterfaceAS <Device::LocalBcastWQModel, Barrier_Model,Device::LocalReduceWQModel> AllSidedShmemNI;

}

//#define PAMI_COLL_MCAST_CLASS
//#define PAMI_COLL_M2M_CLASS
//#define PAMI_COLL_SYSDEP_CLASS SysDep
//#define PAMI_NATIVEINTERFACE
#define PAMI_GEOMETRY_CLASS    PAMI::BGQGeometry

#endif
