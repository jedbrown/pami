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

#include "sys/pami.h"

#include "common/bgq/Global.h"
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

#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/rput/PutRdma.h"
#include "p2p/protocols/rget/GetRdma.h"

//typedef PAMI::Device::Generic::Device ProgressDevice;

typedef PAMI::Device::MU::Context MuContext;

//typedef PAMI::Device::MU::PacketModel MuPacketModel;
typedef PAMI::Device::MU::PacketModelMemoryFifoCompletion MuPacketModel;
//typedef PAMI::Device::MU::PacketModelDeposit MuPacketModel;
//typedef PAMI::Device::MU::PacketModelInterrupt MuPacketModel;

//typedef PAMI::Device::MU::DmaModel MuDmaModel;
typedef PAMI::Device::MU::DmaModelMemoryFifoCompletion MuDmaModel;

typedef PAMI::Protocol::Send::Eager<MuPacketModel, MuContext> MuEager;
typedef PAMI::Protocol::Put::PutRdma<MuDmaModel, MuContext> MuPut;
typedef PAMI::Protocol::Get::GetRdma<MuDmaModel, MuContext> MuGet;


#define TEST_EAGER_PROTOCOL
#define TEST_PUT_PROTOCOL

#ifdef TEST_EAGER_PROTOCOL
static void recv_fn (
    pami_context_t       context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    const void         * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    const void         * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_endpoint_t      origin,
    pami_recv_t        * recv)        /**< OUT: receive message structure */
{
  fprintf (stderr, "Called recv_fn.  cookie = %p, header_size = %zu, pipe_size = %zu\n", cookie, header_size, pipe_size);

  return;
}
#endif

PAMI::Global __myGlobal;

int main(int argc, char ** argv)
{

  fprintf (stderr, "In main\n");

 // PAMI::Global global;
  //PAMI::Memory::MemoryManager mm;

//   PAMI::BgqPersonality personality;
//   PAMI::Mapping mapping (personality);
//   PAMI::bgq_mapcache_t mapcache;

//   // Need to clear the mapcache here to avoid fatal warnings that it is being used uninitialized.
//   memset(&mapcache,0x00,sizeof(mapcache));
//   mapping.init (mapcache, personality);

//   fprintf (stderr, "After mapping init\n");

//  ProgressDevice progress (0, 0, 1);
//  progress.init (NULL,       // pami_context_t
//                 0,          // id_client
//                 0,          // id_offset
//                 NULL,//&mm,        // not used ???
//                 &progress); // "all generic devices"

  MuContext mu (__myGlobal.mapping, 0, 0, 1);
  mu.init (0); // id_client
  fprintf (stderr, "After mu init\n");

  pami_endpoint_t self = PAMI_ENDPOINT_INIT(0,__myGlobal.mapping.task(),0);


  pami_result_t result;

  MuPacketModel pkt (mu);
  fprintf (stderr, "After MuPacketModel init\n");

  MuDmaModel dma (mu, result);
  fprintf (stderr, "After MuDmaModel init\n");


#ifdef TEST_EAGER_PROTOCOL     // use pami protocols .. depends on pami.h types
  MuEager eager ((size_t) 10,  // dispatch set id
                 recv_fn,      // dispatch function
                 (void *)NULL, // dispatch cookie
                 mu,           // "packet" device reference
                 self,         // origin endpoint
                 NULL,         // pami_context_t
                 result);
  fprintf (stderr, "After MuEager protocol constructor\n");
#endif

#ifdef TEST_PUT_PROTOCOL
  MuPut (mu, NULL, result);
  fprintf (stderr, "After MuPut protocol constructor\n");
#endif

#ifdef TEST_GET_PROTOCOL
  MuGet (mu, NULL, result);
  fprintf (stderr, "After MuGet protocol constructor\n");
#endif

  mu.advance ();


  return 0;
}
