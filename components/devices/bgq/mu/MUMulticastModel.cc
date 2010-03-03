/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/devices/bgq/mu/MUMulticastModel.cc
/// \brief ???
///
#include "components/devices/bgq/mu/MUMulticastModel.h"

#ifdef TRACE
  #undef TRACE
#endif
#define TRACE(x) //fprintf x
#ifdef DUMP_DESCRIPTOR
  #undef DUMP_DESCRIPTOR
#endif
#define DUMP_DESCRIPTOR(x,d) //dumpDescriptor(x,d)

/// \see MUMulticastModel
XMI::Device::MU::MUMulticastModel::MUMulticastModel (xmi_result_t &status, MUCollDevice & device) :
Interface::AMMulticastModel < MUMulticastModel, sizeof(mu_multicast_statedata_t) > (status),
_device (device),
_wrapper_model (&_desc_model)
{
  COMPILE_TIME_ASSERT(MUCollDevice::message_metadata_size >= sizeof(MUMulticastModel::metadata_t));
  TRACE((stderr, "<%p>:MUMulticastModel::ctor\n", this));
/// \see MUSPI_BaseDescriptorInfoFields_t
  MUSPI_BaseDescriptorInfoFields_t base =
  {

    Pre_Fetch_Only   :
    MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO,

    Payload_Address  :
    0,

    Message_Length   :
    0,

    Torus_FIFO_Map   :
    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CUSER,

    Dest :
    {

      Destination :

      { Destination :
        0}
    }
  };

/// \see MUSPI_CollectiveDescriptorInfoFields_t
  MUSPI_CollectiveDescriptorInfoFields_t coll =
  {

    Op_Code :
    MUHWI_COLLECTIVE_OP_CODE_SIGNED_ADD,/// \todo not true? ignored for MUHWI_COLLECTIVE_TYPE_BROADCAST

    Word_Length:
    4,    /// \todo not true? ignored for MUHWI_COLLECTIVE_TYPE_BROADCAST

    Class_Route:
    0,    /// \todo global class route always 2 (arbitrary see MUCollDevice)?

    Misc:
    MUHWI_PACKET_VIRTUAL_CHANNEL_USER_COMM_WORLD |
    MUHWI_COLLECTIVE_TYPE_BROADCAST,

    Skip       :
    0
  };

/// \see MUSPI_MemoryFIFODescriptorInfoFields_t
  MUSPI_MemoryFIFODescriptorInfoFields_t memfifo =
  {

    Rec_FIFO_Id    :
    0,

    Rec_Put_Offset :
    0,

    Interrupt      :
    MUHWI_DESCRIPTOR_DO_NOT_INTERRUPT_ON_PACKET_ARRIVAL,

    SoftwareBit    :
    0,

    SoftwareBytes  :
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  };

  _desc_model.setBaseFields (&base);

  MUHWI_Destination dst;
  dst.Destination.A_Destination = 1;
  dst.Destination.B_Destination = 0;
  dst.Destination.C_Destination = 0;
  dst.Destination.D_Destination = 0;
  dst.Destination.E_Destination = 0;
  _desc_model.setDestination(dst);

  _desc_model.setCollectiveFields(&coll);
  _desc_model.setMemoryFIFOFields (&memfifo);

  // Use our rank/addr to set our master reception fifo
  xmi_task_t target_rank = __global.mapping.task();
  size_t addr[BGQ_TDIMS + BGQ_LDIMS];
  TRACE((stderr, "<%p>:MUMulticastModel::ctor .. %p->getRecFifoIdForDescriptor() target_rank %zd\n", this, &_device, (size_t)target_rank));
  __global.mapping.task2global ((xmi_task_t)target_rank, addr);
  TRACE((stderr, "<%p>:MUMulticastModel::ctor .. %p->getRecFifoIdForDescriptor(%zd) target_rank %zd\n", this, &_device, addr[5], (size_t)target_rank));
  /// \todo Assuming p is the recv grp id?
  uint32_t recFifoId = _device.getCollRecFifoIdForDescriptor(addr[5]);;

  TRACE((stderr, "<%p>:MUMulticastModel::ctor .. recFifoId %d\n", this, recFifoId));
  _desc_model.setRecFIFOId (recFifoId);
  DUMP_DESCRIPTOR("MUMulticastModel::ctor", &_desc_model);

/// \todo - Calculate the best torus hints.  For now, hard code to A-minus direction.
//_desc_model.setHints ( MUHWI_PACKET_HINT_AM |
//                 MUHWI_PACKET_HINT_B_NONE |
//                 MUHWI_PACKET_HINT_C_NONE |
//                 MUHWI_PACKET_HINT_D_NONE,
//                 MUHWI_PACKET_HINT_E_NONE );
//DUMP_DESCRIPTOR("initializeDescriptor() ..after setHints",&_desc_model);

};

XMI::Device::MU::MUMulticastModel::~MUMulticastModel ()
{
};

xmi_result_t XMI::Device::MU::MUMulticastModel::registerMcastRecvFunction_impl(int                        dispatch_id,
                                                                               xmi_dispatch_multicast_fn  func,
                                                                               void                      *arg)
{
  TRACE((stderr, "<%p>:MUMulticastModel::registerMcastRecvFunction_impl(%d, %p, %p)\n", this, dispatch_id, func, arg));
  XMI_assert(multicast_model_active_message);

  MemoryFifoPacketHeader * hdr = (MemoryFifoPacketHeader *) & _desc_model.PacketHeader;

  // Register the direct dispatch function. The MU device will have access
  // to the packet payload in the memory fifo at the time the dispacth
  // function is invoked and can provide a direct pointer to the packet
  // payload.
  _dispatch_function = func;
  _dispatch_arg = arg;
  bool success =
  _device.registerPacketHandler (dispatch_id,
                                 dispatch,
                                 this,
                                 hdr->dev.dispatch_id);

  TRACE((stderr, "<%p>:MUMulticastModel::registerMcastRecvFunction_impl dispatch_id = %d, success = %d\n", this, hdr->dev.dispatch_id, (unsigned)success));
  //XMI_assert(success);
  return success ? XMI_SUCCESS : XMI_ERROR;
};

void XMI::Device::MU::dumpHexData(const char * pstring, const uint32_t *buffer, size_t n_ints)
{
  fprintf(stderr, "dumphex:%s:%p:%zd:\n\n", pstring, &buffer, n_ints);
  unsigned nChunks = n_ints / 8;
  if (!buffer || !n_ints) return;
  for (unsigned i = 0; i < nChunks; i++)
  {
    fprintf(stderr,
            "<%p>: %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X \n",
            buffer+(i*8),
            *(buffer+(i*8)+0),
            *(buffer+(i*8)+1),
            *(buffer+(i*8)+2),
            *(buffer+(i*8)+3),
            *(buffer+(i*8)+4),
            *(buffer+(i*8)+5),
            *(buffer+(i*8)+6),
            *(buffer+(i*8)+7)
           );
  }
  if (n_ints % 8)
  {
    unsigned lastChunk = nChunks * 8;
    fprintf(stderr,
            "<%p>: %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X \n",
            buffer+lastChunk,
            lastChunk+0<n_ints?*(buffer+lastChunk+0):0xDEADDEAD,
            lastChunk+1<n_ints?*(buffer+lastChunk+1):0xDEADDEAD,
            lastChunk+2<n_ints?*(buffer+lastChunk+2):0xDEADDEAD,
            lastChunk+3<n_ints?*(buffer+lastChunk+3):0xDEADDEAD,
            lastChunk+4<n_ints?*(buffer+lastChunk+4):0xDEADDEAD,
            lastChunk+5<n_ints?*(buffer+lastChunk+5):0xDEADDEAD,
            lastChunk+6<n_ints?*(buffer+lastChunk+6):0xDEADDEAD,
            lastChunk+7<n_ints?*(buffer+lastChunk+7):0xDEADDEAD
           );
    lastChunk = 0; // gets rid of an annoying warning when not tracing the buffer
  }
}
void XMI::Device::MU::dumpDescriptor(const char* string, const MUHWI_Descriptor_t *desc)
{
  fprintf(stderr, "dumpDescriptor:%s:%p:\n\n", string, desc);
  fprintf(stderr, "0x%08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x\n",
          (uint32_t)(*(((uint32_t*)desc) + 0)),
          (uint32_t)(*(((uint32_t*)desc) + 1)),
          (uint32_t)(*(((uint32_t*)desc) + 2)),
          (uint32_t)(*(((uint32_t*)desc) + 3)),
          (uint32_t)(*(((uint32_t*)desc) + 4)),
          (uint32_t)(*(((uint32_t*)desc) + 5)),
          (uint32_t)(*(((uint32_t*)desc) + 6)),
          (uint32_t)(*(((uint32_t*)desc) + 7)),
          (uint32_t)(*(((uint32_t*)desc) + 8)),
          (uint32_t)(*(((uint32_t*)desc) + 9)),
          (uint32_t)(*(((uint32_t*)desc) + 10)),
          (uint32_t)(*(((uint32_t*)desc) + 11)),
          (uint32_t)(*(((uint32_t*)desc) + 12)),
          (uint32_t)(*(((uint32_t*)desc) + 13)),
          (uint32_t)(*(((uint32_t*)desc) + 14)),
          (uint32_t)(*(((uint32_t*)desc) + 15)));

  fprintf(stderr,"desc->Half_Word0.Prefetch_Only             %#X\n", desc->Half_Word0.Prefetch_Only);
  fprintf(stderr,"desc->Half_Word1.Interrupt                 %#X\n", desc->Half_Word1.Interrupt    );
  fprintf(stderr,"desc->Pa_Payload                           %#lX\n", desc->Pa_Payload              );
  fprintf(stderr,"desc->Message_Length                       %#lX\n", desc->Message_Length          );
  fprintf(stderr,"desc->Torus_FIFO_Map                       %#lX\n", desc->Torus_FIFO_Map          );

  MUHWI_CollectiveNetworkHeader_t *chdr = (MUHWI_CollectiveNetworkHeader_t *)& desc->PacketHeader;


  fprintf(stderr,"hdr->Data_Packet_Type                      %#X\n", chdr->Data_Packet_Type                     );
  fprintf(stderr,"hdr->Byte1.OpCode                          %#X\n", chdr->Byte1.OpCode                         );
  fprintf(stderr,"hdr->Byte1.Word_Length                     %#X\n", chdr->Byte1.Word_Length                    );
  fprintf(stderr,"hdr->Byte2.Class_Route                     %#X\n", chdr->Byte2.Class_Route                    );
  fprintf(stderr,"hdr->Byte2.Interrupt                       %#X\n", chdr->Byte2.Interrupt                      );
  fprintf(stderr,"hdr->Byte3.Virtual_channel                 %#X\n", chdr->Byte3.Virtual_channel                );
  fprintf(stderr,"hdr->Byte3.Collective_Type                 %#X\n", chdr->Byte3.Collective_Type                );
  fprintf(stderr,"hdr->Destination.Destination.A_Destination %#X\n", chdr->Destination.Destination.A_Destination);
  fprintf(stderr,"hdr->Destination.Destination.B_Destination %#X\n", chdr->Destination.Destination.B_Destination);
  fprintf(stderr,"hdr->Destination.Destination.C_Destination %#X\n", chdr->Destination.Destination.C_Destination);
  fprintf(stderr,"hdr->Destination.Destination.D_Destination %#X\n", chdr->Destination.Destination.D_Destination);
  fprintf(stderr,"hdr->Destination.Destination.E_Destination %#X\n", chdr->Destination.Destination.E_Destination);
  fprintf(stderr,"hdr->Byte8.Packet_Type                     %#X\n", chdr->Byte8.Packet_Type                    );
  fprintf(stderr,"hdr->Byte8.Size                            %#X\n", chdr->Byte8.Size                           );
  fprintf(stderr,"hdr->Injection_Info.Skip                   %#X\n", chdr->Injection_Info.Skip                  );

  MemoryFifoPacketHeader_t * hdr = (MemoryFifoPacketHeader_t *) & desc->PacketHeader;

  fprintf(stderr,"hdr->dev.issingle                          %#X\n", hdr->dev.issingle   );
//  fprintf(stderr,"hdr->dev.multipkt                          %#X\n", hdr->dev.multipkt   );
  fprintf(stderr,"hdr->dev.singleonly                        %#X\n", hdr->dev.singleonly );
  fprintf(stderr,"hdr->dev.singlepkt                         %#.2X%.2X%.2X%.2X %#.2X%.2X%.2X%.2X %#.2X%.2X%.2X%.2X %#.2X%.2X%.2X%.2X\n", 
          hdr->dev.singlepkt.metadata[0], hdr->dev.singlepkt.metadata[1], hdr->dev.singlepkt.metadata[2], hdr->dev.singlepkt.metadata[3],
          hdr->dev.singlepkt.metadata[4], hdr->dev.singlepkt.metadata[5], hdr->dev.singlepkt.metadata[6], hdr->dev.singlepkt.metadata[7],
          hdr->dev.singlepkt.metadata[8], hdr->dev.singlepkt.metadata[9], hdr->dev.singlepkt.metadata[10], hdr->dev.singlepkt.metadata[11],
          hdr->dev.singlepkt.metadata[12], hdr->dev.singlepkt.metadata[13], hdr->dev.singlepkt.metadata[14], hdr->dev.singlepkt.metadata[15]);
  fprintf(stderr,"hdr->dev.dispatch_id                       %#X\n", hdr->dev.dispatch_id);

}


#undef TRACE
#undef DUMP_DESCRIPTOR
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
