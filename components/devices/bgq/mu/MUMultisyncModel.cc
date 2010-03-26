/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUMultisyncModel.cc
 * \brief ???
 */
#include "components/devices/bgq/mu/MUMultisyncModel.h"

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

/// \see MUMultisyncModel
PAMI::Device::MU::MUMultisyncModel::MUMultisyncModel (MUCollDevice & device, pami_result_t &status) :
    Interface::MultisyncModel < MUMultisyncModel, MUCollDevice, sizeof(mu_multisync_statedata_t) > (device, status),
    _device (device),
    _wrapper_model (&_desc_model)
{
  COMPILE_TIME_ASSERT(MUCollDevice::message_metadata_size >= sizeof(MUMultisyncModel::metadata_t));
  TRACE((stderr, "<%p>:MUMultisyncModel::ctor\n", this));
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
    MUHWI_COLLECTIVE_OP_CODE_SIGNED_ADD,

Word_Length:
    4,

Class_Route:
    0,

Misc:
    MUHWI_PACKET_VIRTUAL_CHANNEL_USER_COMM_WORLD |
    MUHWI_COLLECTIVE_TYPE_ALLREDUCE,

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
    1,

SoftwareBytes  :
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  };

  _desc_model.setBaseFields (&base);
  _desc_model.setCollectiveFields(&coll);
  _desc_model.setMemoryFIFOFields (&memfifo);

  // Use our rank/addr to set our master reception fifo
  pami_task_t target_rank = __global.mapping.task();
  size_t addr[BGQ_TDIMS + BGQ_LDIMS];
  TRACE((stderr, "<%p>:MUMultisyncModel::ctor .. %p->getRecFifoIdForDescriptor() target_rank %zu\n", this, &_device, (size_t)target_rank));
  __global.mapping.task2global ((pami_task_t)target_rank, addr);
  TRACE((stderr, "<%p>:MUMultisyncModel::ctor .. %p->getRecFifoIdForDescriptor(%zu) target_rank %zu\n", this, &_device, addr[5], (size_t)target_rank));
  /// \todo Assuming p is the recv grp id?
  uint32_t recFifoId = _device.getCollRecFifoIdForDescriptor(addr[5]);;

  TRACE((stderr, "<%p>:MUMultisyncModel::ctor .. recFifoId %d\n", this, recFifoId));
  _desc_model.setRecFIFOId (recFifoId);

  MemoryFifoPacketHeader * hdr = (MemoryFifoPacketHeader *) & _desc_model.PacketHeader;

  // Register the dispatch function.
  bool success =
    _device.registerPacketHandler (0x00BC,
                                   dispatch,
                                   this,
                                   hdr->dev.dispatch_id);
  TRACE((stderr, "<%p>:MUMulticastModel::registerMcastRecvFunction_impl dispatch_id = %#X, success = %d\n", this, hdr->dev.dispatch_id, (unsigned)success));
  DUMP_DESCRIPTOR("MUMultisyncModel::ctor", &_desc_model);

  PAMI_assert(success);

};

PAMI::Device::MU::MUMultisyncModel::~MUMultisyncModel ()
{
};

#undef TRACE
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
