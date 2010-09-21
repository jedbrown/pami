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

#ifndef ENABLE_NEW_SHMEM
  #include "components/devices/workqueue/LocalBcastWQMessage.h"
  #include "components/devices/workqueue/LocalAllreduceWQMessage.h"
  #include "components/devices/workqueue/LocalReduceWQMessage.h"
#endif

  #include "components/devices/bgq/mu2/Factory.h"
  #include "components/devices/bgq/mu2/Context.h"
  #include "components/devices/bgq/mu2/model/PacketModel.h"
  #include "components/devices/bgq/mu2/model/DmaModel.h"
  #include "components/devices/bgq/mu2/model/DmaModelMemoryFifoCompletion.h"

  #include "components/devices/bgq/mu2/model/AxialMulticast.h"
  #include "components/devices/bgq/mu2/model/MulticastDmaModel.h"
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

#ifdef ENABLE_NEW_SHMEM
#include "components/devices/shmemcoll/ShmemCollDevice.h"
#include "components/devices/shmemcoll/ShmemCollDesc.h"
#include "components/devices/shmemcoll/ShmemMcombModelWorld.h"
#include "components/devices/shmemcoll/ShmemMcstModelWorld.h"
#endif

#include "components/devices/shmemcoll/ShmemCollDesc.h"
#include "components/devices/shmemcoll/ShmemColorMcstModel.h"

namespace PAMI
{
  typedef Geometry::Common                     BGQGeometry;

  typedef MemoryAllocator<2048, 16> ProtocolAllocator; /// \todo How much do we really need?  Is there a better way?

  typedef Device::MU::Context MUDevice;

  typedef BGQNativeInterface < MUDevice,
                               Device::MU::MulticastModel<false, false>,
                               Device::MU::MultisyncModel<false, false>,
                               Device::MU::MulticombineModel<Device::MU::AllreducePacketModel, false, false> > MUGlobalNI;

  typedef BGQNativeInterface < MUDevice,
                               Device::MU::AxialMulticastModel<false, false>,
                               Device::MU::MultisyncModel<false, false>,
                               Device::MU::MulticombineModel<Device::MU::AllreducePacketModel, false, false> > MUAxialNI;

/*  typedef BGQNativeInterface < MUDevice,
                               Device::MU::AxialMulticastModel<false, false>,
                               Device::MU::MultisyncModel<false, false>,
                               Device::MU::MulticombineModel<Device::MU::AllreducePacketModel, false, false> > MUAxialDputNI;
*/
  typedef BGQNativeInterfaceAS < MUDevice,
                               Device::MU::MulticastDmaModel,
                               Device::MU::MultisyncModel<false, false>,
                               Device::MU::MulticombineModel<Device::MU::AllreducePacketModel, false, false> > MUAxialDputNI;

  typedef PAMI::Device::Shmem::ShmemCollDesc <PAMI::Atomic::GccBuiltin> ShmemCollDesc;
  typedef PAMI::Device::Shmem::ShmemColorMcstModel<PAMI::Device::Generic::Device, ShmemCollDesc> ShaddrMcstModel;
  
  typedef  PAMI::BGQNativeInterfaceASMultiDevice<MUDevice,
    MUDevice,
    Device::MU::MulticastDmaModel,
    ShaddrMcstModel,
    Device::MU::MultisyncModel<false, false>,
    Device::MU::MulticombineModel<PAMI::Device::MU::AllreducePacketModel, false, false> > MUShmemAxialDputNI;

  typedef Fifo::FifoPacket <32, 544> ShmemPacket;
  typedef Fifo::LinearFifo<Counter::GccInPlaceCounter, ShmemPacket, 16> ShmemFifo;
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


  typedef PAMI::Barrier::CounterBarrier<PAMI::Counter::BGQ::L2Counter> Barrier_Type;

  typedef PAMI::Device::AtomicBarrierMdl<Barrier_Type>                           ShmemMsyncModel;

#ifdef ENABLE_NEW_SHMEM
  typedef PAMI::Device::Shmem::ShmemCollDesc <Atomic::GccBuiltin> ShmemCollDesc;
  typedef PAMI::Device::ShmemCollDevice<ShmemCollDesc> ShmemCollDevice;
  typedef PAMI::Device::Shmem::ShmemMcombModelWorld <ShmemCollDevice, ShmemCollDesc> ShmemMcombModel;
  typedef PAMI::Device::Shmem::ShmemMcstModelWorld <ShmemCollDevice, ShmemCollDesc> ShmemMcstModel;
#else
  typedef Device::LocalAllreduceWQModel ShmemMcombModel;
  typedef Device::LocalBcastWQModel ShmemMcstModel;
  typedef ShmemDevice ShmemCollDevice;
#endif

  typedef BGQNativeInterfaceAS <ShmemCollDevice, ShmemMcstModel, ShmemMsyncModel,ShmemMcombModel> AllSidedShmemNI;

}

//#define PAMI_COLL_MCAST_CLASS
//#define PAMI_COLL_M2M_CLASS
//#define PAMI_NATIVEINTERFACE
//#define PAMI_GEOMETRY_CLASS    PAMI::BGQGeometry

#endif
