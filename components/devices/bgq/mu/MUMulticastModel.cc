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

/// \see MUMulticastModel
XMI::Device::MU::MUMulticastModel::MUMulticastModel (MUCollDevice & device, xmi_result_t &status) :
Interface::AMMulticastModel < MUMulticastModel, MUCollDevice, sizeof(mu_multicast_statedata_t) > (device, status),
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


#undef TRACE
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
