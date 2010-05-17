/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/bgq/mu2/simple.cc
 * \brief Simple standalone MU software device test.
 */

//#include "common/bgq/Global.h"
//#include "components/memory/MemoryManager.h"
//#include "components/devices/generic/Device.h"

#include "common/bgq/BgqPersonality.h"
#include "common/bgq/Mapping.h"
#include "common/bgq/BgqMapCache.h"

#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/model/DmaModel.h"
#include "components/devices/bgq/mu2/model/DmaModelMemoryFifoCompletion.h"
#include "components/devices/bgq/mu2/model/PacketModel.h"
#include "components/devices/bgq/mu2/model/PacketModelMemoryFifoCompletion.h"
#include "components/devices/bgq/mu2/model/PacketModelDeposit.h"
#include "components/devices/bgq/mu2/model/PacketModelInterrupt.h"

//#include "p2p/protocols/send/eager/Eager.h"

//typedef PAMI::Device::Generic::Device ProgressDevice;

typedef PAMI::Device::MU::Context MuDevice;

//typedef PAMI::Device::MU::PacketModel MuPacketModel;
typedef PAMI::Device::MU::PacketModelMemoryFifoCompletion MuPacketModel;
//typedef PAMI::Device::MU::PacketModelDeposit MuPacketModel;
//typedef PAMI::Device::MU::PacketModelInterrupt MuPacketModel;

//typedef PAMI::Device::MU::DmaModel MuDmaModel;
typedef PAMI::Device::MU::DmaModelMemoryFifoCompletion MuDmaModel;

//typedef PAMI::Protocol::Send::Eager<T_Model, MuDevice> MuEager;

int main(int argc, char ** argv)
{

 // PAMI::Global global;
  //PAMI::Memory::MemoryManager mm;

  PAMI::BgqPersonality personality;
  PAMI::Mapping mapping (personality);
  PAMI::bgq_mapcache_t mapcache;
  mapping.init (mapcache, personality);

//  ProgressDevice progress (0, 0, 1);
//  progress.init (NULL,       // pami_context_t
//                 0,          // id_client
//                 0,          // id_offset
//                 NULL,//&mm,        // not used ???
//                 &progress); // "all generic devices"

  MuDevice mu (mapping, 0, 0, 1);//, progress);
  mu.init (0); // id_client

  pami_result_t result;
  MuPacketModel pkt (mu);
  MuDmaModel dma (mu, result);

#if 0  // use pami protocols .. depends on pami.h types
  pami_result_t result;
  MuEager eager (10,      // dispatch set id
                 recv,    //dispatch function
                 NULL,    // dispatch cookie
                 mu,      // "packet" device reference
                 result);
#endif


  // advance
//  progress.advance ();
  mu.advance ();


  return 0;
}
