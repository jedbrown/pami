/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/DmaModelMemoryFifoCompletion.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_DmaModelMemoryFifoCompletion_h__
#define __components_devices_bgq_mu2_DmaModelMemoryFifoCompletion_h__

#include "components/devices/bgq/mu2/model/DmaModelBase.h"
#include "components/devices/bgq/mu2/model/MultiMemoryFifoDescriptor.h"
#include "components/devices/bgq/mu2/model/MultiMemoryFifoSelfDescriptor.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class DmaModelMemoryFifoCompletion : public MU::DmaModelBase<DmaModelMemoryFifoCompletion>
      {

        public :

          /// \see PAMI::Device::Interface::DmaModel::DmaModel
          inline DmaModelMemoryFifoCompletion (MU::Context & device, pami_result_t & status) :
              MU::DmaModelBase<DmaModelMemoryFifoCompletion> (device, status)
          {
            COMPILE_TIME_ASSERT(sizeof(MU::Context::notify_t) <= MemoryFifoPacketHeader::packet_singlepacket_metadata_size);
// !!!!
// Initialize _rput model: destination = self, deterministic routing, etc
// Initialize _rmem model: destination = self, dispatch = notify, etc
// !!!!

            // Set the destination to the coordinates of this mu context
            _rput.desc[0].setDestination (*(_device.getMuDestinationSelf()));



            MemoryFifoPacketHeader * hdr =
              (MemoryFifoPacketHeader *) & _rack.desc[0].PacketHeader;

            // Set the "notify" system dispatch identifier
            hdr->setSinglePacket (true);
            hdr->setDispatchId (MU::Context::dispatch_system_notify);
          };

          /// \see PAMI::Device::Interface::DmaModel::~DmaModel
          inline ~DmaModelMemoryFifoCompletion () {};

          inline size_t
          initializePayloadDescriptors_impl (void                * vaddr,
                                             uint64_t              local_dst_pa,
                                             uint64_t              remote_src_pa,
                                             size_t                bytes,
                                             size_t                from_task,
                                             size_t                from_offset,
                                             pami_event_function   local_fn,
                                             void                * cookie)
          {
            // Retreive the route information back to mu context "self"
            uint64_t map;
            uint8_t  hintsABCD;
            uint8_t  hintsE;

            _device.pinInformation (from_task,
                                    from_offset,
                                    map,
                                    hintsABCD,
                                    hintsE);

            // Clone the remote direct put model descriptor into the payload
            DirectPutDescriptor * rput = (DirectPutDescriptor *) vaddr;
            _rput.clone (rput);

            // Set the payload of the direct put descriptor to be the physical
            // address of the source buffer on the remote node (from the user's
            // memory region).
            rput->desc[0].setPayload (remote_src_pa, bytes);

            // Set the destination buffer address for the remote direct put.
            rput->desc[0].setRecPayloadBaseAddressInfo (0, local_dst_pa);



            // Clone the remote memory fifo ack model descriptor into the payload
            MemoryFifoSelfDescriptor * rack = (MemoryFifoSelfDescriptor *) (rput + 1);
            _rack.clone (rack);


            rack->initializeDescriptors (map, hintsABCD, hintsE);

            // Copy the completion function+cookie into the packet header.
            MU::Context::notify_t * hdr =
              (MU::Context::notify_t *) & rack->desc[0].PacketHeader;
            hdr->fn = local_fn;
            hdr->cookie = cookie;

            return sizeof(MUHWI_Descriptor_t) + sizeof(MUHWI_Descriptor_t);
          };

        protected:

          DirectPutDescriptor  _rput;
          MemoryFifoDescriptor _rack;

      }; // PAMI::Device::MU::DmaModelMemoryFifoCompletion class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_DmaModelMemoryFifoCompletion_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//




