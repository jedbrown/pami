/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUDmaModel.cc
 * \brief ???
 */
#include "components/devices/bgq/mu/MUDmaModel.h"
#include "components/devices/bgq/mu/ResourceManager.h"

/// \see MUSPI_Pt2PtMemoryFIFODescriptor
///
XMI::Device::MU::MUDmaModel::MUDmaModel (MUDevice & device, xmi_context_t context) :
    Interface::DmaModel<MUDmaModel, MUDevice, MemRegion::BgqMemregion, MUInjFifoMessage> (device, context),
    _device (device),
    _wrapper_model (),
    _context (context)
{
  // --------------------------------------------------------------------------
  // Initialize (as much as possible) the rget descriptor model.
  // --------------------------------------------------------------------------
  MUSPI_BaseDescriptorInfoFields_t base = {
    Pre_Fetch_Only   : MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO,
    Payload_Address  : 0,
    Message_Length   : 0,
    Torus_FIFO_Map   : 0,
    Dest : { Destination : { Destination : 0 }}};

  MUSPI_Pt2PtDescriptorInfoFields_t pt2pt = {
    Hints_ABCD : 0,
    Misc1      : MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
                 MUHWI_PACKET_DO_NOT_DEPOSIT |
                 MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE,
    Misc2      : MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC,
    Skip       : 0 };

  MUSPI_RemoteGetDescriptorInfoFields_t rget = {
    Type             : MUHWI_PACKET_TYPE_GET,
    Rget_Inj_FIFO_Id : 0 };

  _rget_desc_model.setBaseFields (&base);
  _rget_desc_model.setPt2PtFields (&pt2pt);
  _rget_desc_model.setRemoteGetFields( &rget );

#if 1   // These are requried in order to work around mambo bugs
#warning using mambo work around - remove later

  // TODO - Calculate the best torusInjectionFifoMap.
  // For now, hard code to A-minus direction.
  _rget_desc_model.setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);

  // TODO - Calculate the best torus hints.
  // For now, hard code to A-minus direction.
  _rget_desc_model.setHints ( MUHWI_PACKET_HINT_AM |
                              MUHWI_PACKET_HINT_B_NONE |
                              MUHWI_PACKET_HINT_C_NONE |
                              MUHWI_PACKET_HINT_D_NONE,
                              MUHWI_PACKET_HINT_E_NONE );
#endif



  // --------------------------------------------------------------------------
  // Initialize (as much as possible) the direct put descriptor model which
  // will be sent as payload of the rget descriptor.
  //
  // This direct put descriptor specifies a deterministically routed transfer
  // so that a deterministically routed memory fifo transfer can follow and
  // notify the receiving rank (a.k.a. the "origin" rank in a remote get
  // operation) that the direct put has completed
  // --------------------------------------------------------------------------
  MUSPI_DirectPutDescriptorInfoFields rput = {
    Rec_Payload_Base_Address_Id : ResourceManager::BAT_DEFAULT_ENTRY_NUMBER,
    Rec_Payload_Offset          : 0,
    Rec_Counter_Base_Address_Id : ResourceManager::BAT_SHAREDCOUNTER_ENTRY_NUMBER,
    Rec_Counter_Offset          : 0,
    Pacing                      : MUHWI_PACKET_DIRECT_PUT_IS_NOT_PACED};

  _rput_desc_model.setBaseFields (&base);
  _rput_desc_model.setPt2PtFields (&pt2pt);
  _rput_desc_model.setDirectPutFields( &rput );

  // The destination of the remote direct put is this (local) rank.
  MUHWI_Destination dst;
  size_t addr[7];
  size_t local_rank = _device.sysdep->mapping.task ();
  _device.sysdep->mapping.task2torus (local_rank, addr);
  dst.Destination.A_Destination = addr[0];
  dst.Destination.B_Destination = addr[1];
  dst.Destination.C_Destination = addr[2];
  dst.Destination.D_Destination = addr[3];
  dst.Destination.E_Destination = addr[4];
  _rput_desc_model.setDestination (dst);

  _rput_desc_model.setPt2PtMisc1 (MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
                                  MUHWI_PACKET_DO_NOT_DEPOSIT |
                                  MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE);

  _rput_desc_model.setPt2PtMisc2 (MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC);

  // Use the shared reception counter.
  _rput_desc_model.setRecCounterBaseAddressInfo (1, 0);

#if 1   // These are requried in order to work around mambo bugs
#warning using mambo work around - remove later

  // TODO - Calculate the best torusInjectionFifoMap.
  // For now, hard code to A-minus direction.
  _rput_desc_model.setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);

  // TODO - Calculate the best torus hints.
  // For now, hard code to A-minus direction.
  _rput_desc_model.setHints ( MUHWI_PACKET_HINT_AM |
                              MUHWI_PACKET_HINT_B_NONE |
                              MUHWI_PACKET_HINT_C_NONE |
                              MUHWI_PACKET_HINT_D_NONE,
                              MUHWI_PACKET_HINT_E_NONE );
#endif



  // --------------------------------------------------------------------------
  // Initialize (as much as possible) the memory fifo descriptor model which
  // will be sent as payload of the rget descriptor.
  //
  // See the direct put comments above.
  // --------------------------------------------------------------------------
  MUSPI_MemoryFIFODescriptorInfoFields_t memfifo = {
    Rec_FIFO_Id    : 0,
    Rec_Put_Offset : 0,
    Interrupt      : MUHWI_DESCRIPTOR_DO_NOT_INTERRUPT_ON_PACKET_ARRIVAL,
    SoftwareBit    : 1,  // Specify a "single packet" send
    SoftwareBytes  : { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }};

  // Register a memfifo dispatch id and write it into the remote memfifo
  // descriptor model.
  uint16_t dispatch_id;
  _device.registerPacketHandler (255,
                                 (Interface::RecvFunction_t) dispatch_notify,
                                 (void *) this, dispatch_id);
  *((uint16_t *)&memfifo.SoftwareBytes[12]) = dispatch_id;

  _rmem_desc_model.setBaseFields (&base);
  _rmem_desc_model.setPt2PtFields (&pt2pt);
  _rmem_desc_model.setMemoryFIFOFields (&memfifo);

  // The destination of the remote memfifo send is this (local) rank.
  _rmem_desc_model.setDestination (dst);

#if 1   // These are requried in order to work around mambo bugs
#warning using mambo work around - remove later

  // TODO - Calculate the best torusInjectionFifoMap.
  // For now, hard code to A-minus direction.
  _rmem_desc_model.setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);

  // TODO - Calculate the best torus hints.
  // For now, hard code to A-minus direction.
  _rmem_desc_model.setHints ( MUHWI_PACKET_HINT_AM |
                              MUHWI_PACKET_HINT_B_NONE |
                              MUHWI_PACKET_HINT_C_NONE |
                              MUHWI_PACKET_HINT_D_NONE,
                              MUHWI_PACKET_HINT_E_NONE );
#endif



  // --------------------------------------------------------------------------
  // Initialize (as much as possible) the direct put descriptor model.
  //
  // This direct put descriptor is different than the "rget dput" descriptor
  // and is used to directly send data from this local rank to that remote rank.
  // --------------------------------------------------------------------------
  _dput_desc_model.setBaseFields (&base);
  _dput_desc_model.setPt2PtFields (&pt2pt);
  _dput_desc_model.setDirectPutFields( &rput );

  _dput_desc_model.setPt2PtMisc1 (MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
                                  MUHWI_PACKET_DO_NOT_DEPOSIT |
                                  MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE);

  _dput_desc_model.setPt2PtMisc2 (MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC);

  // Use the shared reception counter.
  _dput_desc_model.setRecCounterBaseAddressInfo (1, 0);

#if 1   // These are requried in order to work around mambo bugs
#warning using mambo work around - remove later

  // TODO - Calculate the best torusInjectionFifoMap.
  // For now, hard code to A-minus direction.
  _dput_desc_model.setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);

  // TODO - Calculate the best torus hints.
  // For now, hard code to A-minus direction.
  _dput_desc_model.setHints ( MUHWI_PACKET_HINT_AM |
                              MUHWI_PACKET_HINT_B_NONE |
                              MUHWI_PACKET_HINT_C_NONE |
                              MUHWI_PACKET_HINT_D_NONE,
                              MUHWI_PACKET_HINT_E_NONE );
#endif


};

XMI::Device::MU::MUDmaModel::~MUDmaModel () {};
