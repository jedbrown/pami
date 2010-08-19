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

#include "util/ccmi_debug.h"

  #include "components/devices/shmem/ShmemDevice.h"
  #include "components/devices/shmem/ShmemPacketModel.h"
  #include "components/devices/shmem/ShmemDmaModel.h"
  #include "components/devices/shmem/shaddr/BgqShaddrReadOnly.h"
  #include "util/fifo/FifoPacket.h"
  #include "util/fifo/LinearFifo.h"
  #include "components/devices/workqueue/LocalBcastWQMessage.h"
  #include "components/devices/workqueue/LocalAllreduceWQMessage.h"
  #include "components/devices/workqueue/LocalReduceWQMessage.h"
  #include "components/devices/workqueue/LocalBcastWQMessage.h"

  #include "components/devices/bgq/mu2/Factory.h"
  #include "components/devices/bgq/mu2/Context.h"
  #include "components/devices/bgq/mu2/model/PacketModel.h"
  #include "components/devices/bgq/mu2/model/DmaModel.h"
  #include "components/devices/bgq/mu2/model/DmaModelMemoryFifoCompletion.h"

  #include "components/devices/bgq/mu2/model/Multicast.h"
  #include "components/devices/bgq/mu2/model/Multicombine.h"
  #include "components/devices/bgq/mu2/model/Multisync.h"
  #include "components/devices/bgq/mu2/model/AllreducePacketModel.h"

//  #include "components/devices/bgq/mu/MUCollDevice.h"
//  #include "components/devices/bgq/mu/MUMulticastModel.h"
//  #include "components/devices/bgq/mu/MUMultisyncModel.h"
//  #include "components/devices/bgq/mu/MUMulticombineModel.h"

#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/composite/Composite.h"
#include "p2p/protocols/rget/GetRdma.h"
#include "p2p/protocols/rput/PutRdma.h"
#include "p2p/protocols/SendPWQ.h"

#include "components/atomic/bgq/L2Counter.h"
#include "components/atomic/counter/CounterBarrier.h"
#include "components/devices/misc/AtomicBarrierMsg.h"

#include "common/NativeInterface.h"
#include "common/bgq/NativeInterface.h"

#include "algorithms/geometry/Geometry.h"

#define PAMI_GEOMETRY_CLASS    PAMI::Geometry::Common

#include "algorithms/geometry/PGASCollRegistration.h"

namespace PAMI
{
  typedef Geometry::Common                     BGQGeometry;

  typedef MemoryAllocator<2048, 16> ProtocolAllocator; /// \todo How much do we really need?  Is there a better way?

  typedef Device::MU::Context MUDevice;
  //typedef Device::MU::MUCollDevice MUDevice;
  typedef BGQNativeInterface < MUDevice,
                               Device::MU::MulticastModel<false, false>,
                               Device::MU::MultisyncModel<false, false>,
                               Device::MU::MulticombineModel<Device::MU::AllreducePacketModel, false, false> > MUGlobalNI;

  typedef Fifo::FifoPacket <32, 544> ShmemPacket;
  typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 16> ShmemFifo;
  typedef Device::ShmemDevice<ShmemFifo,Device::Shmem::BgqShaddrReadOnly> ShmemDevice;
  typedef Device::Shmem::PacketModel<ShmemDevice> ShmemPacketModel;
  typedef Device::Shmem::DmaModel<ShmemDevice> ShmemDmaModel;

  typedef Protocol::Send::Eager <ShmemPacketModel, ShmemDevice> ShmemEagerBase;
  typedef PAMI::Protocol::Send::SendPWQ < ShmemEagerBase > ShmemEager;

  // shmem active message over p2p eager
  typedef PAMI::NativeInterfaceActiveMessage<ShmemEager> ShmemNI_AM;
  // shmem allsided over p2p eager
  typedef PAMI::NativeInterfaceAllsided<ShmemEager> ShmemNI_AS;

  typedef Protocol::Send::Eager <Device::MU::PacketModel, MUDevice, true> MUEagerBase;
  typedef PAMI::Protocol::Send::SendPWQ < MUEagerBase > MUEager;

  // MU active message over p2p eager
  typedef PAMI::NativeInterfaceActiveMessage<MUEager> MUNI_AM;
  // MU allsided over p2p eager
  typedef PAMI::NativeInterfaceAllsided<MUEager> MUNI_AS;

  // PGAS over MU
  typedef TSPColl::NBCollManager<MUNI_AM> MU_NBCollManager;
  typedef CollRegistration::PGASRegistration<BGQGeometry,
                                             MUNI_AM,
                                             ProtocolAllocator,
                                             MUEager,
                                             MUDevice,
                                             MU_NBCollManager> MU_PGASCollreg;
  // PGAS over Shmem
  typedef TSPColl::NBCollManager<ShmemNI_AM> Shmem_NBCollManager;
  typedef CollRegistration::PGASRegistration<BGQGeometry,
                                             ShmemNI_AM,
                                             ProtocolAllocator,
                                             ShmemEager,
                                             ShmemDevice,
                                             Shmem_NBCollManager> Shmem_PGASCollreg;

  // shmem + MU composite active message over p2p eager
  typedef PAMI::NativeInterfaceActiveMessage< PAMI::Protocol::Send::SendPWQ< Protocol::Send::Send> > CompositeNI_AM;
  // shmem + MU composite allsided over p2p eager
  typedef PAMI::NativeInterfaceAllsided< PAMI::Protocol::Send::SendPWQ< Protocol::Send::Send> > CompositeNI_AS;


  typedef PAMI::Barrier::CounterBarrier<PAMI::Counter::BGQ::L2NodeCounter> Barrier_Type;

  typedef PAMI::Device::AtomicBarrierMdl<Barrier_Type> Barrier_Model;

  typedef BGQNativeInterfaceAS <ShmemDevice, Device::LocalBcastWQModel, Barrier_Model,Device::LocalAllreduceWQModel> AllSidedShmemNI;

}

//#define PAMI_COLL_MCAST_CLASS
//#define PAMI_COLL_M2M_CLASS
//#define PAMI_NATIVEINTERFACE
//#define PAMI_GEOMETRY_CLASS    PAMI::BGQGeometry

#endif
