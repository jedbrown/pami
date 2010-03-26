/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUMulticombineModel.cc
 * \brief ???
 */
#include "components/devices/bgq/mu/MUMulticombineModel.h"

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x
const bool   PAMI::Device::MU::MUMulticombineModel::multicombine_model_op_support(pami_dt dt, pami_op op)
{
  const bool support[PAMI_DT_COUNT][PAMI_OP_COUNT] =
  {
    // PAMI_UNDEFINED_OP, PAMI_NOOP, PAMI_MAX, PAMI_MIN, PAMI_SUM, PAMI_PROD, PAMI_LAND, PAMI_LOR, PAMI_LXOR, PAMI_BAND, PAMI_BOR, PAMI_BXOR, PAMI_MAXLOC, PAMI_MINLOC, PAMI_USERDEFINED_OP,
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_UNDEFINED_DT
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_SIGNED_CHAR
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_UNSIGNED_CHAR
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_SIGNED_SHORT
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_UNSIGNED_SHORT
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_SIGNED_INT
    {false,           false,    false,   true,    false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_UNSIGNED_INT
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_SIGNED_LONG_LONG
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_UNSIGNED_LONG_LONG
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_FLOAT
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_DOUBLE
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_LONG_DOUBLE
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_LOGICAL
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_SINGLE_COMPLEX
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_DOUBLE_COMPLEX
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_LOC_2INT
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_LOC_SHORT_INT
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_LOC_FLOAT_INT
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_LOC_DOUBLE_INT
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_LOC_2FLOAT
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false},//PAMI_LOC_2DOUBLE
    {false,           false,    false,   false,   false,   false,    false,    false,   false,    false,    false,   false,    false,      false,      false} //PAMI_USERDEFINED_DT
  };
  TRACE((stderr, "MUMulticombineModel::multicombine_model_op_support(%d, %d) = %d\n", dt, op, support[dt][op]));
  return(support[dt][op]);
}

/// \see MUMulticombineModel
PAMI::Device::MU::MUMulticombineModel::MUMulticombineModel (MUCollDevice & device, pami_result_t &status):
    Interface::MulticombineModel < MUMulticombineModel,MUCollDevice, sizeof(mu_multicombine_statedata_t) > (device, status),
    _device (device),
    _wrapper_model (&_desc_model)
{
  COMPILE_TIME_ASSERT(MUCollDevice::message_metadata_size >= sizeof(MUMulticombineModel::metadata_t));
  TRACE((stderr, "<%p>:MUMulticombineModel::ctor\n", this));
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
    MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_MIN,/// \todo not true? ignored for MUHWI_COLLECTIVE_TYPE_BROADCAST

Word_Length:
    4,    /// \todo not true? ignored for MUHWI_COLLECTIVE_TYPE_BROADCAST

Class_Route:
    0,    /// \todo global class route always 2 (arbitrary see MUCollDevice)?

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
    0,

SoftwareBytes  :
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  };

  _desc_model.setBaseFields (&base);
  _desc_model.setCollectiveFields(&coll);
  _desc_model.setMemoryFIFOFields (&memfifo);

  // Use our rank/addr to set our master reception fifo
  pami_task_t target_rank = __global.mapping.task();
  size_t addr[BGQ_TDIMS + BGQ_LDIMS];
  TRACE((stderr, "<%p>:MUMulticombineModel::ctor .. %p->getRecFifoIdForDescriptor() target_rank %zu\n", this, &_device, (size_t)target_rank));
  __global.mapping.task2global ((pami_task_t)target_rank, addr);
  TRACE((stderr, "<%p>:MUMulticombineModel::ctor .. %p->getRecFifoIdForDescriptor(%zu) target_rank %zu\n", this, &_device, addr[5], (size_t)target_rank));
  /// \todo Assuming p is the recv grp id?
  uint32_t recFifoId = _device.getCollRecFifoIdForDescriptor(addr[5]);;

  TRACE((stderr, "<%p>:MUMulticombineModel::ctor .. recFifoId %d\n", this, recFifoId));
  _desc_model.setRecFIFOId (recFifoId);

  MemoryFifoPacketHeader * hdr = (MemoryFifoPacketHeader *) & _desc_model.PacketHeader;

  // Register the dispatch function.
  bool success =
    _device.registerPacketHandler (0x00AA,
                                   dispatch,
                                   this,
                                   hdr->dev.dispatch_id);
  TRACE((stderr, "<%p>:MUMulticastModel::registerMcastRecvFunction_impl dispatch_id = %#X, success = %d\n", this, hdr->dev.dispatch_id, (unsigned)success));
  DUMP_DESCRIPTOR("MUMultisyncModel::ctor", &_desc_model);

  PAMI_assert(success);

};

PAMI::Device::MU::MUMulticombineModel::~MUMulticombineModel ()
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
