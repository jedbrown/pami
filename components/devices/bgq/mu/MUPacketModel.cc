/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUPacketModel.cc
 * \brief ???
 */
#include "components/devices/bgq/mu/MUPacketModel.h"

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

/// \see MUSPI_Pt2PtMemoryFIFODescriptor
///
XMI::Device::MU::MUPacketModel::MUPacketModel (MUDevice & device, xmi_context_t context) :
    Interface::MessageModel<MUPacketModel, MUDevice, MUInjFifoMessage> (device, context),
    _device (device),
    _wrapper_model (&_desc_model),
    _context (context)
{
  MUSPI_BaseDescriptorInfoFields_t base =
  {
    Pre_Fetch_Only   : MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO,
    Payload_Address  : 0,
    Message_Length   : 0,
    Torus_FIFO_Map   : 0,
    Dest :
    { Destination :
      { Destination : 0 }}
    };

  MUSPI_Pt2PtDescriptorInfoFields_t pt2pt =
  {
    Hints_ABCD : 0,
    Misc1      :
      MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
      MUHWI_PACKET_DO_NOT_DEPOSIT |
      MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE,
    Misc2      : MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC,
    Skip       : 0
  };

  MUSPI_MemoryFIFODescriptorInfoFields_t memfifo =
  {
    Rec_FIFO_Id    : 0,
    Rec_Put_Offset : 0,
    Interrupt      : MUHWI_DESCRIPTOR_DO_NOT_INTERRUPT_ON_PACKET_ARRIVAL,
    SoftwareBit    : 0,
    SoftwareBytes  :
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
  };


  _desc_model.setBaseFields (&base);
  _desc_model.setPt2PtFields (&pt2pt);
  _desc_model.setMemoryFIFOFields (&memfifo);
};

XMI::Device::MU::MUPacketModel::~MUPacketModel () {};

xmi_result_t XMI::Device::MU::MUPacketModel::init_impl (size_t                      dispatch,
                                                        Interface::RecvFunction_t   direct_recv_func,
                                                        void                      * direct_recv_func_parm,
                                                        Interface::RecvFunction_t   read_recv_func,
                                                        void                      * read_recv_func_parm)
{
  TRACE((stderr, ">> MUPacketModel::init_impl(%p, %p, %p, %p)\n", direct_recv_func, direct_recv_func_parm, read_recv_func, read_recv_func_parm));
  MemoryFifoPacketHeader * hdr =
    (MemoryFifoPacketHeader *) & _desc_model.PacketHeader;

  // Register the direct dispatch function. The MU device will have access
  // to the packet payload in the memory fifo at the time the dispacth
  // function is invoked and can provide a direct pointer to the packet
  // payload.
  bool success =
    _device.registerPacketHandler (direct_recv_func,
                                   direct_recv_func_parm,
                                   hdr->dev.dispatch_id);

  TRACE((stderr, "<< MUPacketModel::init_impl()\n"));
  return XMI_SUCCESS;
};
#undef TRACE
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
