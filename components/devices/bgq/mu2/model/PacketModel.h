/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/PacketModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_PacketModel_h__
#define __components_devices_bgq_mu2_PacketModel_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/memory.h>

#include "components/devices/PacketInterface.h"
#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/model/MultiMemoryFifoDescriptor.h"



namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class PacketModel : public Interface::PacketModel < MU::PacketModel, MU::Context, 128 >
      {
          public :

          /// \see PAMI::Device::Interface::PacketModel::PacketModel
          PacketModel (MU::Context & device);

          /// \see PAMI::Device::Interface::PacketModel::~PacketModel
          ~PacketModel ();

          /// \see PAMI::Device::Interface::PacketModel::isPacketDeterministic
          static const bool   deterministic_packet_model         = true;

          /// \see PAMI::Device::Interface::PacketModel::isPacketReliable
          static const bool   reliable_packet_model              = true;

          /// \see PAMI::Device::Interface::PacketModel::getPacketMetadataBytes
          static const size_t packet_model_metadata_bytes        =
            MemoryFifoPacketHeader::packet_singlepacket_metadata_size;

          /// \see PAMI::Device::Interface::PacketModel::getPacketMultiMetadataBytes
          static const size_t packet_model_multi_metadata_bytes  =
            MemoryFifoPacketHeader::packet_multipacket_metadata_size;

          /// \see PAMI::Device::Interface::PacketModel::getPacketPayloadBytes
          static const size_t packet_model_payload_bytes         = MU::Context::payload_size;

          /// \see PAMI::Device::Interface::PacketModel::getPacketTransferStateBytes
          static const size_t packet_model_state_bytes           = 128;

          /// \see PAMI::Device::Interface::PacketModel::init
          pami_result_t init_impl (size_t                      dispatch,
                                   Interface::RecvFunction_t   direct_recv_func,
                                   void                      * direct_recv_func_parm,
                                   Interface::RecvFunction_t   read_recv_func,
                                   void                      * read_recv_func_parm);

          /// \see PAMI::Device::Interface::PacketModel::postPacket
          inline bool postPacket_impl (uint8_t              (&state)[PacketModel::packet_model_state_bytes],
                                       pami_event_function   fn,
                                       void                * cookie,
                                       pami_task_t           target_task,
                                       size_t                target_offset,
                                       void                * metadata,
                                       size_t                metasize,
                                       void                * payload,
                                       size_t                length);


        private:
          MemoryFifoDescriptor            _singlepkt;
          MemoryFifoDescriptor            _multipkt;
          MU::Context                   & _device;
      };

      PacketModel::PacketModel (MU::Context & device) :
          Interface::PacketModel < MU::PacketModel, MU::Context, 128 > (device),
          _device (device)
      {
        MemoryFifoPacketHeader * hdr = NULL;

        // Memory fifo descriptor '0' is for single packet transfers.
        hdr = (MemoryFifoPacketHeader *) & _singlepkt.desc[0].PacketHeader;
        hdr->setSinglePacket (true);

        // Memory fifo descriptor '1' is for multi-packet transfers.
        hdr = (MemoryFifoPacketHeader *) & _multipkt.desc[0].PacketHeader;
        hdr->setSinglePacket (false);
      };

      pami_result_t PacketModel::init_impl (size_t                      dispatch,
                                            Interface::RecvFunction_t   direct_recv_func,
                                            void                      * direct_recv_func_parm,
                                            Interface::RecvFunction_t   read_recv_func,
                                            void                      * read_recv_func_parm)
      {
        // Register the direct dispatch function. The MU device will have access
        // to the packet payload in the memory fifo at the time the dispatch
        // function is invoked and can provide a direct pointer to the packet
        // payload.
        uint16_t id = 0;
        if (_device.registerPacketHandler (dispatch,
                                           direct_recv_func,
                                           direct_recv_func_parm,
                                           id))
          {
            MemoryFifoPacketHeader * hdr = NULL;

            hdr = (MemoryFifoPacketHeader *) & _singlepkt.desc[0].PacketHeader;
            hdr->setDispatchId (id);

            hdr = (MemoryFifoPacketHeader *) & _multipkt.desc[0].PacketHeader;
            hdr->setDispatchId (id);

            return PAMI_SUCCESS;
          }

        return PAMI_ERROR;
      };

      bool PacketModel::postPacket_impl (uint8_t               (&state)[PacketModel::packet_model_state_bytes],
                                         pami_event_function   fn,
                                         void                * cookie,
                                         pami_task_t           target_task,
                                         size_t                target_offset,
                                         void                * metadata,
                                         size_t                metasize,
                                         void                * payload,
                                         size_t                length)
      {
        MUHWI_Destination_t   dest;
        MUSPI_InjFifo_t     * ififo;
        uint16_t              rfifo;
        uint64_t              map;
        uint8_t               hintsABCD;
        uint8_t               hintsE;

        size_t fnum = _device.pinFifo ((size_t) target_task, target_offset, dest,
                                       &ififo, rfifo, map, hintsABCD, hintsE);


        MUHWI_Descriptor_t * desc;
        void               * vaddr;
        uint64_t             paddr;

        size_t ndesc =
          _device.nextInjectionDescriptor (fnum, &desc, &vaddr, &paddr);

        if (likely(ndesc > 0))
          {
            // There is at least one descriptor slot available in the injection
            // fifo before a fifo-wrap event.

            // Clone the single-packet model descriptor into the injection fifo
            MemoryFifoDescriptor * memfifo = (MemoryFifoDescriptor *) desc;
            _singlepkt.clone (memfifo);

            // Initialize the injection fifo descriptor in-place.
            memfifo->initializeDescriptors (dest, map, hintsABCD, hintsE, rfifo);

            // Set the payload information.
            memfifo->desc[0].setPayload (paddr, length);

            // Copy the metadata into the packet header.
            if (likely(metasize > 0))
              {
                uint8_t * hdr = (uint8_t *) & memfifo->desc[0].PacketHeader;
                memcpy((void *) (hdr + (32 - MemoryFifoPacketHeader::packet_singlepacket_metadata_size)), metadata, metasize); // <-- replace with an optimized MUSPI function.
              }

            // Copy the payload into the immediate payload buffer.
            memcpy (vaddr, payload, length);

            // Finally, advance the injection fifo tail pointer. This action
            // completes the injection operation.
            uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (ififo);

            // Invoke the completion callback function
            if (fn != NULL)
              {
                //fn (_context, cookie, PAMI_SUCCESS); // Descriptor is done...notify.
                sequenceNum = 0; // suppress warning for now
              }

          }
        else
          {
            // Create a message and post it to the device.
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

        return true;
      };

    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_PacketModel_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//


